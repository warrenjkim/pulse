#pragma once

#include <sys/wait.h>

#include <string>
#include <utility>

namespace pulse::process {

class Output {
 public:
  Output(int raw_status, std::string stdout_data, std::string stderr_data)
      : raw_status_(raw_status),
        stdout_(std::move(stdout_data)),
        stderr_(std::move(stderr_data)) {}

  bool Ok() const { return Exited() && ExitCode() == 0; }

  bool Exited() const { return WIFEXITED(RawStatus()); }

  // Valid only if Exited() is true.
  int ExitCode() const { return WEXITSTATUS(RawStatus()); }

  bool Signaled() const { return WIFSIGNALED(RawStatus()); }

  // Valid only if Signaled() is true.
  int TermSignal() const { return WTERMSIG(RawStatus()); }

  const std::string& StdOut() const& { return stdout_; }
  const std::string& StdErr() const& { return stderr_; }

 private:
  int& RawStatus() const { return const_cast<int&>(raw_status_); }

  int raw_status_;
  std::string stdout_;
  std::string stderr_;
};

}  // namespace pulse::process
