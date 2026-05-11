#include "concurrent/thread_pool.h"

#include <atomic>
#include <chrono>
#include <thread>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse::concurrent {

namespace {

using ::testing::Eq;

TEST(ThreadPoolTest, ExecutesTask) {
  ThreadPool pool(2);
  std::atomic<bool> ran = false;
  pool.submit([&ran] { ran = true; });
  pool.join();
  EXPECT_TRUE(ran);
}

TEST(ThreadPoolTest, ExecutesAllTasks) {
  ThreadPool pool(4);
  std::atomic<int> count = 0;
  for (int i = 0; i < 100; i++) {
    pool.submit([&count] { count++; });
  }

  pool.join();
  EXPECT_THAT(count.load(), Eq(100));
}

TEST(ThreadPoolTest, JoinBlocksUntilComplete) {
  ThreadPool pool(2);
  std::atomic<bool> ran = false;
  pool.submit([&ran] {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    ran = true;
  });
  pool.join();
  EXPECT_TRUE(ran);
}

TEST(ThreadPoolTest, DestructorJoins) {
  std::atomic<int> count = 0;
  {
    ThreadPool pool(2);
    for (int i = 0; i < 10; i++) {
      pool.submit([&count] { count++; });
    }
  }

  EXPECT_THAT(count.load(), Eq(10));
}

TEST(ThreadPoolTest, ConcurrentSubmit) {
  ThreadPool pool(4);
  std::atomic<int> count = 0;
  constexpr int kThreads = 4;
  constexpr int kTasksPerThread = 25;
  std::vector<std::thread> producers;
  for (int i = 0; i < kThreads; i++) {
    producers.emplace_back([&pool, &count] {
      for (int j = 0; j < kTasksPerThread; j++) {
        pool.submit([&count] { count++; });
      }
    });
  }

  for (auto& thread : producers) {
    thread.join();
  }

  pool.join();
  EXPECT_THAT(count.load(), Eq(kThreads * kTasksPerThread));
}

}  // namespace

}  // namespace pulse::concurrent
