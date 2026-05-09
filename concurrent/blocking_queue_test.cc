#include "concurrent/blocking_queue.h"

#include <chrono>
#include <future>
#include <optional>
#include <thread>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace pulse::concurrent {

namespace {

using ::testing::Eq;
using ::testing::Optional;

TEST(BlockingQueueTest, ConcurrentPushPop) {
  BlockingQueue<int> queue(100);
  std::vector<int> results;
  std::mutex mu;

  constexpr int kItems = 50;
  std::thread producer([&queue] {
    for (int i = 0; i < kItems; i++) {
      EXPECT_TRUE(queue.push(i));
    }
  });

  std::thread consumer([&mu, &queue, &results] {
    for (int i = 0; i < kItems; i++) {
      if (std::optional<int> front = queue.pop(); front.has_value()) {
        std::scoped_lock l(mu);
        results.push_back(*front);
      }
    }
  });

  producer.join();
  consumer.join();

  EXPECT_THAT(results.size(), Eq(kItems));
}

TEST(BlockingQueueTest, ShutdownEmptyPopReturnsNullopt) {
  BlockingQueue<int> queue(10);

  queue.shutdown();

  EXPECT_THAT(queue.pop(), Eq(std::nullopt));
}

TEST(BlockingQueueTest, ShutdownUnblocks) {
  BlockingQueue<int> queue(10);
  EXPECT_TRUE(queue.push(1));
  EXPECT_TRUE(queue.push(2));

  queue.shutdown();

  EXPECT_THAT(queue.pop(), Optional(1));
  EXPECT_THAT(queue.pop(), Optional(2));
  EXPECT_THAT(queue.pop(), Eq(std::nullopt));
}

TEST(BlockingQueueTest, PushAfterShutdownDropsItem) {
  BlockingQueue<int> queue(10);

  queue.shutdown();

  EXPECT_FALSE(queue.push(1));
  EXPECT_THAT(queue.pop(), Eq(std::nullopt));
}

TEST(BlockingQueueTest, IsShutdown) {
  BlockingQueue<int> queue(10);

  EXPECT_FALSE(queue.is_shutdown());

  queue.shutdown();

  EXPECT_TRUE(queue.is_shutdown());
}

TEST(BlockingQueueTest, PopBlocksUntilPush) {
  BlockingQueue<int> queue(10);
  std::promise<int> promise;
  std::future<int> future = promise.get_future();

  std::thread consumer([&] {
    std::optional<int> front = queue.pop();
    promise.set_value(front.value_or(-1));
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  queue.push(42);

  EXPECT_THAT(future.get(), Eq(42));
  consumer.join();
}

TEST(BlockingQueueTest, PopUnblocksOnShutdown) {
  BlockingQueue<int> queue(10);
  std::promise<std::optional<int>> promise;
  std::future<std::optional<int>> future = promise.get_future();

  std::thread consumer([&] { promise.set_value(queue.pop()); });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  queue.shutdown();

  EXPECT_THAT(future.get(), Eq(std::nullopt));
  consumer.join();
}

TEST(BlockingQueueTest, PushBlocksWhenFull) {
  BlockingQueue<int> queue(1);
  queue.push(1);

  std::atomic<bool> pushed = false;
  std::thread producer([&queue, &pushed] {
    queue.push(2);
    pushed = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_FALSE(pushed);
  queue.pop();

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  EXPECT_TRUE(pushed);
  producer.join();
}

}  // namespace

}  // namespace pulse::concurrent
