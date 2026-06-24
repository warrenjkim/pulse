#include "pulse/concurrent/thread_pool.h"

#include <cstddef>
#include <functional>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>

#include "pulse/concurrent/blocking_queue.h"

namespace pulse::concurrent {

ThreadPool::ThreadPool(size_t threads) : queue_(threads), in_flight_(0) {
  pool_.reserve(threads);
  for (size_t i = 0; i < threads; i++) {
    pool_.emplace_back([this] {
      while (true) {
        if (std::optional<std::function<void()>> work = queue_.Pop();
            work.has_value()) {
          (*std::move(work))();
          in_flight_--;
          cv_.notify_one();
        } else {
          break;
        }
      }
    });
  }
}

ThreadPool::~ThreadPool() {
  Join();
  queue_.Shutdown();
  for (std::thread& thread : pool_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void ThreadPool::Submit(std::function<void()> task) {
  in_flight_++;
  queue_.Push(std::move(task));
}

void ThreadPool::Join() {
  std::unique_lock l(mu_);
  cv_.wait(l, [this] { return in_flight_.load() == 0; });
}

}  // namespace pulse::concurrent
