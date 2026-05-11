#pragma once

#include <functional>
#include <thread>
#include <vector>

#include "concurrent/blocking_queue.h"

namespace pulse::concurrent {

class ThreadPool {
 public:
  explicit ThreadPool(size_t threads);

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  ~ThreadPool();

  // TODO(use a move only function)
  void submit(std::function<void()> work);

  void join();

 private:
  std::vector<std::thread> pool_;
  BlockingQueue<std::function<void()>> queue_;

  std::mutex mu_;
  std::condition_variable cv_;
  std::atomic<int> in_flight_;
};

}  // namespace pulse::concurrent
