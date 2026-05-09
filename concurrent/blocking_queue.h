#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace pulse::concurrent {

template <typename T>
class BlockingQueue {
 public:
  explicit BlockingQueue(size_t max_size)
      : max_size_(max_size), shutdown_(false) {}

  ~BlockingQueue() { shutdown(); }

  bool push(T data) {
    {
      std::unique_lock l(mu_);
      cv_.wait(l, [this] { return shutdown_ || queue_.size() < max_size_; });
      if (shutdown_) {
        return false;
      }

      queue_.push(std::move(data));
    }

    cv_.notify_one();
    return true;
  }

  std::optional<T> pop() {
    std::optional<T> front = std::nullopt;
    {
      std::unique_lock l(mu_);
      cv_.wait(l, [this] { return shutdown_ || !queue_.empty(); });
      if (shutdown_ && queue_.empty()) {
        return std::nullopt;
      }

      front = std::move(queue_.front());
      queue_.pop();
    }

    cv_.notify_one();
    return front;
  }

  void shutdown() {
    {
      std::unique_lock l(mu_);
      shutdown_ = true;
    }

    cv_.notify_all();
  }

  bool is_shutdown() const {
    std::scoped_lock l(mu_);
    return shutdown_;
  }

 private:
  mutable std::mutex mu_;
  std::condition_variable cv_;
  std::queue<T> queue_;
  const size_t max_size_;
  bool shutdown_;
};

}  // namespace pulse::concurrent
