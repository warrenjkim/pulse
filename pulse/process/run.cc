#include "pulse/process/run.h"

#include <sys/fcntl.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "pulse/core/error.h"
#include "pulse/core/result.h"
#include "pulse/process/output.h"

extern char** environ;

namespace pulse::process {

namespace {

constexpr size_t kReadBufferSize = 4096;

Result<void> SetCloExec(int fd) {
  if (fcntl(fd, F_SETFD, FD_CLOEXEC) == -1) {
    return Error{.code = Error::Code::kFailedPrecondition,
                 .message = strerror(errno)};
  }

  return Result<void>{};
}

Result<void> OpenCloExecPipe(int fds[2]) {
  if (pipe(fds) == -1) {
    return Error{.code = Error::Code::kFailedPrecondition,
                 .message = strerror(errno)};
  }

  if (Result<void> r = SetCloExec(fds[0]); !r.ok()) {
    close(fds[0]);
    close(fds[1]);
    return std::move(r).error();
  }

  if (Result<void> r = SetCloExec(fds[1]); !r.ok()) {
    close(fds[0]);
    close(fds[1]);
    return std::move(r).error();
  }

  return Result<void>{};
}

Result<void> DrainPipes(int out_fd, int err_fd, std::string* stdout_data,
                        std::string* stderr_data) {
  int open = 2;
  char buffer[kReadBufferSize];
  pollfd fds[2] = {
      {.fd = out_fd, .events = POLLIN, .revents = 0},
      {.fd = err_fd, .events = POLLIN, .revents = 0},
  };
  while (open > 0) {
    if (poll(fds, 2, /*timeout=*/-1) == -1) {
      if (errno == EINTR) {
        continue;
      }

      return Error{.code = Error::Code::kFailedPrecondition,
                   .message = strerror(errno)};
    }

    for (pollfd& pfd : fds) {
      if (pfd.fd == -1 || !(pfd.revents & (POLLIN | POLLHUP | POLLERR))) {
        continue;
      }

      if (ssize_t bytes = read(pfd.fd, buffer, sizeof(buffer)); bytes > 0) {
        std::string* dest = (pfd.fd == out_fd) ? stdout_data : stderr_data;
        dest->append(buffer, static_cast<size_t>(bytes));
      } else if (bytes == 0) {
        pfd.fd = -1;
        open--;
      } else if (errno != EINTR) {
        return Error{.code = Error::Code::kFailedPrecondition,
                     .message = strerror(errno)};
      }
    }
  }

  return Result<void>{};
}

}  // namespace

Result<Output> Run(std::string_view cmd, std::vector<std::string> args) {
  std::string command(cmd);

  std::vector<char*> argv;
  argv.reserve(args.size() + 2);
  argv.push_back(command.data());
  for (std::string& arg : args) {
    argv.push_back(arg.data());
  }
  argv.push_back(nullptr);

  int out[2];
  if (Result<void> r = OpenCloExecPipe(out); !r.ok()) {
    return std::move(r).error();
  }

  int err[2];
  if (Result<void> r = OpenCloExecPipe(err); !r.ok()) {
    close(out[0]);
    close(out[1]);
    return std::move(r).error();
  }

  pid_t pid = fork();
  if (pid == -1) {
    close(out[0]);
    close(out[1]);
    close(err[0]);
    close(err[1]);
    return Error{.code = Error::Code::kFailedPrecondition,
                 .message = strerror(errno)};
  }

  if (pid == 0) {
    close(out[0]);
    close(err[0]);
    dup2(out[1], STDOUT_FILENO);
    dup2(err[1], STDERR_FILENO);
    close(out[1]);
    close(err[1]);
    execve(command.c_str(), argv.data(), environ);
    _exit(127);
  }

  close(out[1]);
  close(err[1]);

  std::string stdout_data;
  std::string stderr_data;
  Result<void> drain_result =
      DrainPipes(out[0], err[0], &stdout_data, &stderr_data);

  close(out[0]);
  close(err[0]);

  int status;
  if (!drain_result.ok()) {
    waitpid(pid, &status, 0);
    return std::move(drain_result).error();
  }

  if (waitpid(pid, &status, 0) == -1) {
    return Error{.code = Error::Code::kFailedPrecondition,
                 .message = strerror(errno)};
  }

  return Output(status, std::move(stdout_data), std::move(stderr_data));
}

}  // namespace pulse::process
