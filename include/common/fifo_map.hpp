#pragma once

#include <list>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

// A thread-safe Least Frequently Used (LFU) cache implementation
// Key must be hashable and both Key and Value must be copy or move constructible

template <typename Key, typename Value, size_t Capacity = 4UL>
class fifo_map final {
  static_assert(std::is_invocable_v<std::hash<Key>, Key> &&
                    (std::is_copy_constructible_v<Key> || std::is_move_constructible_v<Key>) &&
                    (std::is_copy_constructible_v<Value> || std::is_move_constructible_v<Value>),
                "Key must be hashable and both Key and Value must be copy or move constructible");

  static_assert(Capacity > 0, "Capacity must be greater than 0");
  static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be a power of 2");
  static_assert(std::is_default_constructible_v<Value>, "Value must be default constructible");

 private:
  std::unordered_map<Key, typename std::array<std::pair<Key, Value>, Capacity>::iterator> items_map_;
  std::array<std::pair<Key, Value>, Capacity> items_arr_;
  size_t capacity_;
  size_t size_;
  size_t next_index_;

  mutable std::mutex mtx_;

 public:
  explicit fifo_map() : size_{0UL}, capacity_{Capacity}, next_index_{0UL}, items_arr_{} {}

  ~fifo_map() = default;

  bool contains(const Key& key) const {
    std::lock_guard<std::mutex> lock(mtx_);
    return items_map_.find(key) != items_map_.end();
  }

  // Insert with copy semantics
  void insert(const Key& key, const Value& value) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = items_map_.find(key);
    if (it != items_map_.end()) {
      // Update existing item and move it to the front
      it->second->second = value;
    } else {
      // Insert new item
      if (size_ < capacity_) {
        items_arr_[next_index_] = std::make_pair(key, value);
        items_map_[key] = &items_arr_[next_index_];
        ++size_;
      } else {
        // Evict the oldest item
        auto& evict_item = items_arr_[next_index_];
        items_map_.erase(evict_item.first);
        evict_item = std::make_pair(key, value);
        items_map_[key] = &evict_item;
      }
      next_index_ = (next_index_ + 1) & (capacity_ - 1);
    }
  }

  // Insert with move semantics
  void insert(const Key& key, Value&& value) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = items_map_.find(key);
    if (it != items_map_.end()) {
      // Update existing item
      it->second->second = std::move(value);
    } else {
      // Insert new item
      if (size_ < capacity_) {
        items_arr_[next_index_] = std::make_pair(key, std::move(value));
        items_map_[key] = &items_arr_[next_index_];
        ++size_;
      } else {
        // Evict the oldest item
        auto& evict_item = items_arr_[next_index_];
        items_map_.erase(evict_item.first);
        evict_item = std::make_pair(key, std::move(value));
        items_map_[key] = &evict_item;
      }
      next_index_ = (next_index_ + 1) & (capacity_ - 1);
    }
  }

  // Emplace with perfect forwarding
  template <typename... Args>
  void emplace(const Key& key, Args&&... args) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = items_map_.find(key);
    if (it != items_map_.end()) {
      // Update existing item
      it->second->second = Value(std::forward<Args>(args)...);
    } else {
      // Insert new item
      if (size_ < capacity_) {
        items_arr_[next_index_] = std::make_pair(key, Value(std::forward<Args>(args)...));
        items_map_[key] = &items_arr_[next_index_];
        ++size_;
      } else {
        // Evict the oldest item
        auto& evict_item = items_arr_[next_index_];
        items_map_.erase(evict_item.first);
        evict_item = std::make_pair(key, Value(std::forward<Args>(args)...));
        items_map_[key] = &evict_item;
      }
      next_index_ = (next_index_ + 1) & (capacity_ - 1);
    }
  }

  Value& get(const Key& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = items_map_.find(key);
    if (it != items_map_.end()) {
      return it->second->second;
    }

    throw std::out_of_range("Key not found");
  }

  void erase(const Key& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = items_map_.find(key);
    if (it != items_map_.end()) {
      items_map_.erase(it);
      --size_;
    }
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return size_;
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return size_ == 0;
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mtx_);
    items_arr_.clear();
    items_map_.clear();
    size_ = 0;
  }

  size_t capacity() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return capacity_;
  }

  Value& operator[](const Key& key) {
    if (!contains(key)) {
      insert(key, Value{});
    }

    return get(key);
  }
};