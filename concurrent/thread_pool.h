#pragma once

#include <functional>
#include <thread>
#include <vector>

#include "concurrent/blocking_queue.h"

namespace pulse::concurrent {

// A fixed-size pool of worker threads that executes work concurrently.
class ThreadPool {
 public:
  // Spawns `threads` number of threads that all pull work off of `queue_`.
  explicit ThreadPool(size_t threads);

  // Not copyable or movable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Blocks for all work to complete, flushes `queue_` and joins all of the
  // worker threads. All submitted work is guaranteed to complete before the
  // pool is destroyed.
  ~ThreadPool();

  // TODO(use a move only function)
  // Submits `work` to be executed by a worker thread. It is non-blocking and
  // the work runs asynchronously.
  //
  // NOTE: Must not be called if `join()` has been called.
  void submit(std::function<void()> work);

  // Blocks until all submitted tasks have completed. Safe to call multiple
  // times — subsequent calls return immediately if no work is in flight.
  void join();

 private:
  std::vector<std::thread> pool_;
  BlockingQueue<std::function<void()>> queue_;

  std::mutex mu_;
  std::condition_variable cv_;
  std::atomic<int> in_flight_;
};

}  // namespace pulse::concurrent
