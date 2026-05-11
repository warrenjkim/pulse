#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>

namespace pulse::concurrent {

// A bounded, blocking queue. This class is thread-safe.
template <typename T>
class BlockingQueue {
 public:
  // Constructs a queue with a maximum capacity of `max_size`.
  explicit BlockingQueue(size_t max_size);

  // Not copyable or movable
  BlockingQueue(const BlockingQueue&) = delete;
  BlockingQueue& operator=(const BlockingQueue&) = delete;

  ~BlockingQueue();

  // Pushes `data` onto the queue. Blocks if the queue is full. Returns false if
  // the queue is shut down, true otherwise.
  bool push(T data);

  // Pops the front item off the queue. Blocks if the queue is empty. Returns
  // std::nullopt if the queue is shut down and empty, T otherwise.
  std::optional<T> pop();

  // Unblocks all threads waiting in push() or pop(). All subsequent push()
  // calls return false. `pop()` drains remaining items before returning
  // std::nullopt.
  void shutdown();

  bool is_shutdown() const;

 private:
  mutable std::mutex mu_;
  std::condition_variable cv_;
  std::queue<T> queue_;
  const size_t max_size_;
  bool shutdown_;
};

// Implementation details below;

template <typename T>
BlockingQueue<T>::BlockingQueue(size_t max_size)
    : max_size_(max_size), shutdown_(false) {}

template <typename T>
BlockingQueue<T>::~BlockingQueue() {
  shutdown();
}

template <typename T>
bool BlockingQueue<T>::push(T data) {
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

template <typename T>
std::optional<T> BlockingQueue<T>::pop() {
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

template <typename T>
void BlockingQueue<T>::shutdown() {
  {
    std::unique_lock l(mu_);
    shutdown_ = true;
  }

  cv_.notify_all();
}

template <typename T>
bool BlockingQueue<T>::is_shutdown() const {
  std::scoped_lock l(mu_);
  return shutdown_;
}

}  // namespace pulse::concurrent
