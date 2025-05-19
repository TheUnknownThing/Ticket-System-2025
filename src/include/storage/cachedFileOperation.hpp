#ifndef CACHED_FILE_OPERATION_HPP
#define CACHED_FILE_OPERATION_HPP

#include "cache/LRUKCache.hpp"
#include "cache/fileOperation.hpp"
#include <string>

template <class T, int info_len = 2, std::size_t K = 4,
          std::size_t CACHE = 8192>
class CachedFileOperation {
  using Key = int;

  FileOperation<T, info_len> disk;
  LRUKCache<Key, T, K, CACHE> cache;

  static void write_back(const Key &idx, const T &obj, void *context) {
    auto *self = static_cast<CachedFileOperation *>(context);
    T copy = obj;
    self->disk.update(copy, idx);
  }

public:
  CachedFileOperation() : disk(), cache(write_back, this) {}

  explicit CachedFileOperation(const std::string &fn)
      : disk(fn), cache(write_back, this) {}

  ~CachedFileOperation() { cache.flush(); }

  void initialise(std::string fn = "") { disk.initialise(fn); }
  void get_info(int &tmp, int n) { disk.get_info(tmp, n); }
  void write_info(int tmp, int n) { disk.write_info(tmp, n); }
  bool isEmpty() { return disk.isEmpty(); }

  int write(T &t) {
    int idx = disk.write(t);
    cache.put(idx, t, false);
    return idx;
  }

  void read(T &t, const int idx) {
    if (!cache.get(idx, t)) {
      disk.read(t, idx);
      cache.put(idx, t, false);
    }
  }

  void update(T &t, const int idx) { cache.put(idx, t, true); }

  void remove(int idx) {
    cache.flush();
    disk.remove(idx);
  }

  void flush() { cache.flush(); }

  void clear() {
    cache.clear();
    disk.clear();
  }
};

#endif // CACHED_FILE_OPERATION_HPP