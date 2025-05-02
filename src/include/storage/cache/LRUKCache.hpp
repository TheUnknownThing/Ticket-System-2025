#ifndef LRUK_CACHE_HPP
#define LRUK_CACHE_HPP

#include <cassert>
#include <deque>
#include <functional>
#include <limits>
#include <unordered_map>

template <class Key, class Value,
          std::size_t K = 4,
          std::size_t MaxResidency = 8192>
class LRUKCache {
  static_assert(K >= 1, "K must be at least 1");

  struct Entry {
    Value val;
    bool dirty = false;
    std::deque<std::size_t> hist;
  };

  std::unordered_map<Key, Entry> map;                     // key -> entry
  std::size_t clk = 0;                                    // logical time
  std::function<void(const Key &, const Value &)> writer; // write‑back

  void touch(Entry &e) {
    e.hist.push_back(++clk);
    if (e.hist.size() > K)
      e.hist.pop_front();
  }

  std::size_t kth(const Entry &e) const {
    return (e.hist.size() < K) ? 0 : e.hist.front();
  }

  void evict() {
    assert(map.size() > 0);
    auto victim_it = map.begin();
    std::size_t min_time = std::numeric_limits<std::size_t>::max();

    for (auto it = map.begin(); it != map.end(); ++it) {
      auto t = kth(it->second);
      if (t < min_time) {
        min_time = t;
        victim_it = it;
      }
    }

    // write back if required
    if (victim_it->second.dirty && writer)
      writer(victim_it->first, victim_it->second.val);

    map.erase(victim_it);
  }

public:
  explicit LRUKCache(std::function<void(const Key &, const Value &)> wb)
      : writer(std::move(wb)) {}

  /*------------------------------------------------------------------*/
  /* public interface                                                 */
  /*------------------------------------------------------------------*/
  bool contains(const Key &k) const { return map.count(k); }

  /* get()     – read‑only access; returns false if key not present  */
  bool get(const Key &k, Value &out) {
    auto it = map.find(k);
    if (it == map.end())
      return false;
    touch(it->second);
    out = it->second.val;
    return true;
  }

  /* insert/update value; 'dirty' tells whether a later write‑back   */
  /* is necessary                                                    */
  void put(const Key &k, const Value &v, bool dirty) {
    Entry &e = map[k];
    e.val = v;
    e.dirty = dirty;
    touch(e);

    if (map.size() > MaxResidency)
      evict();
  }

  /* mark page as dirty without changing its payload */
  void mark_dirty(const Key &k) {
    auto it = map.find(k);
    if (it != map.end())
      it->second.dirty = true;
  }

  /* write every dirty page to backing store                          */
  void flush() {
    for (auto &pr : map)
      if (pr.second.dirty && writer)
        writer(pr.first, pr.second.val);
    // mark clean after flush
    for (auto &pr : map)
      pr.second.dirty = false;
  }
};
#endif // LRUK_CACHE_HPP