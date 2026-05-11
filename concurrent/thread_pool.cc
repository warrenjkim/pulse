#include "concurrent/thread_pool.h"

#include <functional>
#include <mutex>

#include "concurrent/blocking_queue.h"

namespace pulse::concurrent {

ThreadPool::ThreadPool(size_t threads) : queue_(threads), in_flight_(0) {
  pool_.reserve(threads);
  for (size_t i = 0; i < threads; i++) {
    pool_.emplace_back([this] {
      while (true) {
        if (std::optional<std::function<void()>> work = queue_.pop();
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
  join();
  queue_.shutdown();
  for (std::thread& thread : pool_) {
    if (thread.joinable()) {
      thread.join();
    }
  }
}

void ThreadPool::submit(std::function<void()> task) {
  in_flight_++;
  queue_.push(std::move(task));
}

void ThreadPool::join() {
  std::unique_lock l(mu_);
  cv_.wait(l, [this] { return in_flight_.load() == 0; });
}

}  // namespace pulse::concurrent
