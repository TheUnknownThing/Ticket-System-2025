#ifndef LRUK_CACHE_HPP
#define LRUK_CACHE_HPP

#include "stl/map.hpp"
#include <cassert>
#include <cstddef>
#include <limits>

template <class Key, class Value, std::size_t K = 4,
          std::size_t MaxResidency = 8192>
class LRUKCache {
  static_assert(K >= 1, "K must be at least 1");
  static_assert(MaxResidency > 0, "MaxResidency must be greater than 0");

  struct Entry {
    Value val;
    bool dirty = false;
    std::size_t hist[K];
    std::size_t hist_count = 0;
    std::size_t hist_head = 0;

    void push_history(std::size_t time) {
      std::size_t tail = (hist_head + hist_count) % K;
      hist[tail] = time;
      if (hist_count < K) {
        hist_count++;
      } else {
        hist_head = (hist_head + 1) % K;
      }
    }

    std::size_t get_kth_history() const {
      if (hist_count < K) {
        return 0;
      }
      return hist[hist_head];
    }
  };

  sjtu::map<Key, Entry> map;
  std::size_t clk = 0;
  void (*writer)(const Key &, const Value &, void *) = nullptr;
  void *writer_context = nullptr;

  void touch(Entry &e) { e.push_history(++clk); }

  std::size_t kth(const Entry &e) const { return e.get_kth_history(); }

  void evict() {
    assert(!map.empty());
    typename sjtu::map<Key, Entry>::iterator victim_it = map.end();
    std::size_t min_time = std::numeric_limits<std::size_t>::max();

    for (auto it = map.begin(); it != map.end(); ++it) {
      std::size_t current_kth_time = kth(it->second);
      if (current_kth_time < min_time) {
        min_time = current_kth_time;
        victim_it = it;
      }
    }

    assert(victim_it != map.end());
    if (victim_it == map.end())
      return;

    if (victim_it->second.dirty && writer) {
      writer(victim_it->first, victim_it->second.val, writer_context);
    }
    map.erase(victim_it);
  }

public:
  explicit LRUKCache(void (*wb)(const Key &, const Value &, void *),
                     void *context)
      : writer(wb), writer_context(context) {}

  ~LRUKCache() = default;

  LRUKCache(const LRUKCache &) = delete;
  LRUKCache &operator=(const LRUKCache &) = delete;
  LRUKCache(LRUKCache &&) = delete;
  LRUKCache &operator=(LRUKCache &&) = delete;

  bool contains(const Key &k) const { return map.count(k) > 0; }

  bool get(const Key &k, Value &out) {
    auto it = map.find(k);
    if (it == map.end()) {
      return false;
    }
    touch(it->second);
    out = it->second.val;
    return true;
  }

  void put(const Key &k, const Value &v, bool dirty) {
    auto it = map.find(k);
    if (it != map.end()) {
      it->second.val = v;
      it->second.dirty = dirty;
      touch(it->second);
    } else {
      if (map.size() >= MaxResidency) {
        evict();
      }
      Entry &new_entry = map[k];
      new_entry.val = v;
      new_entry.dirty = dirty;
      touch(new_entry);
    }
  }

  void mark_dirty(const Key &k) {
    auto it = map.find(k);
    if (it != map.end()) {
      it->second.dirty = true;
    }
  }

  void flush() {
    for (auto it = map.begin(); it != map.end(); ++it) {
      if (it->second.dirty && writer) {
        writer(it->first, it->second.val, writer_context);
        it->second.dirty = false;
      }
    }
  }
};
#endif // LRUK_CACHE_HPP