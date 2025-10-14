#pragma once

#include <list>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <unordered_map>

// A thread-safe Least Frequently Used (LFU) cache implementation
// Key must be hashable and both Key and Value must be copy constructible

template <typename Key, typename Value>
class lfu_map final {
  static_assert(std::is_invocable_v<std::hash<Key>, Key> && std::is_copy_constructible_v<Key> &&
                    std::is_copy_constructible_v<Value>,
                "Key must be hashable and both Key and Value must be copy constructible");

 private:
  constexpr static size_t DEFAULT_CAPACITY = 10U;

 private:
  std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> items_map_;
  std::list<std::pair<Key, Value>> items_list_;
  size_t capacity_;

  mutable std::mutex mtx_;

 public:
  explicit lfu_map(size_t capacity = DEFAULT_CAPACITY) : capacity_(capacity) {
    if (capacity_ == 0) {
      throw std::invalid_argument("Capacity must be greater than 0");
    }
  }

  ~lfu_map() = default;

  bool contains(const Key& key) const {
    std::lock_guard<std::mutex> lock(mtx_);
    return items_map_.find(key) != items_map_.end();
  }

  void insert(const Key& key, const Value& value) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = items_map_.find(key);
    if (it != items_map_.end()) {
      // Update existing item and move it to the front
      it->second->second = value;
      items_list_.splice(items_list_.begin(), items_list_, it->second);
    } else {
      // Insert new item
      if (items_list_.size() >= capacity_) {
        // Evict the least frequently used item (back of the list)
        auto last = items_list_.back();
        items_map_.erase(last.first);
        items_list_.pop_back();
      }
      items_list_.emplace_front(key, value);
      items_map_[key] = items_list_.begin();
    }
  }

  Value& get(const Key& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = items_map_.find(key);
    if (it != items_map_.end()) {
      // Move accessed item to the front
      items_list_.splice(items_list_.begin(), items_list_, it->second);
      return it->second->second;
    }
    throw std::out_of_range("Key not found");
  }

  void erase(const Key& key) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = items_map_.find(key);
    if (it != items_map_.end()) {
      items_list_.erase(it->second);
      items_map_.erase(it);
    }
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return items_list_.size();
  }

  bool empty() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return items_list_.empty();
  }

  void clear() {
    std::lock_guard<std::mutex> lock(mtx_);
    items_list_.clear();
    items_map_.clear();
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