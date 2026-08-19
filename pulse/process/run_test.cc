#include "pulse/process/run.h"

#include <sys/signal.h>

#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "pulse/core/result.h"
#include "pulse/process/output.h"

namespace pulse::process {

namespace {

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;
using ::testing::StrEq;

TEST(RunTest, NoArgs) {
  Result<Output> output = pulse::process::Run("/usr/bin/true");
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->Ok(), IsTrue());
}

TEST(RunTest, CapturesStdOut) {
  Result<Output> output = pulse::process::Run("/bin/echo", {"hello"});
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->Ok(), IsTrue());
  EXPECT_THAT(output->ExitCode(), Eq(0));
  EXPECT_THAT(output->StdOut(), StrEq("hello\n"));
  EXPECT_THAT(output->StdErr(), StrEq(""));
}

TEST(RunTest, CapturesStdErr) {
  Result<Output> output =
      pulse::process::Run("/bin/sh", {"-c", "echo lemon 1>&2"});
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->Ok(), IsTrue());
  EXPECT_THAT(output->StdErr(), StrEq("lemon\n"));
  EXPECT_THAT(output->StdOut(), StrEq(""));
}

TEST(RunTest, CapturesBothStreams) {
  Result<Output> output = pulse::process::Run(
      "/bin/sh", {"-c", "echo out_line; echo err_line 1>&2"});
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->StdOut(), StrEq("out_line\n"));
  EXPECT_THAT(output->StdErr(), StrEq("err_line\n"));
}

TEST(RunTest, NonZeroExitCode) {
  Result<Output> output = pulse::process::Run("/bin/sh", {"-c", "exit 3"});
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->Ok(), IsFalse());
  EXPECT_THAT(output->Exited(), IsTrue());
  EXPECT_THAT(output->ExitCode(), Eq(3));
  EXPECT_THAT(output->Signaled(), IsFalse());
}

TEST(RunTest, SignaledProcessIsNotExited) {
  Result<Output> output = pulse::process::Run("/bin/sh", {"-c", "kill -9 $$"});
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->Ok(), IsFalse());
  EXPECT_THAT(output->Exited(), IsFalse());
  EXPECT_THAT(output->Signaled(), IsTrue());
  EXPECT_THAT(output->TermSignal(), Eq(SIGKILL));
}

TEST(RunTest, NonexistentCommandExits) {
  Result<Output> output = pulse::process::Run("/bad/path");
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->Ok(), IsFalse());
  EXPECT_THAT(output->Exited(), IsTrue());
  EXPECT_THAT(output->ExitCode(), Eq(127));
}

TEST(RunTest, HandlesOutputLargerThanPipeBuffer) {
  Result<Output> output =
      pulse::process::Run("/bin/sh", {"-c", "yes | head -c 200000"});
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->Ok(), IsTrue());
  EXPECT_THAT(output->StdOut().size(), Eq(200000u));
}

TEST(RunTest, InterleavedLargeStdOutAndStdErr) {
  Result<Output> output = pulse::process::Run(
      "/bin/sh",
      {"-c", "head -c 100000 /dev/zero & head -c 100000 /dev/zero 1>&2; wait"});
  ASSERT_THAT(output.ok(), IsTrue());
  EXPECT_THAT(output->Ok(), IsTrue());
  EXPECT_THAT(output->StdOut().size(), Eq(100000u));
  EXPECT_THAT(output->StdErr().size(), Eq(100000u));
}

}  // namespace

}  // namespace pulse::process
