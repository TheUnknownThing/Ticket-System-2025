#ifndef CACHED_FILE_OPERATION_HPP
#define CACHED_FILE_OPERATION_HPP

#include "cache/LRUKCache.hpp"
#include "cache/fileOperation.hpp"

template <class T, int info_len = 2,
          std::size_t K = 3,        // LRU‑K parameter
          std::size_t CACHE = 1024> // max pages in RAM
class CachedFileOperation {
  using Key = int; // file offset == index

  FileOperation<T, info_len> disk;
  LRUKCache<Key, T, K, CACHE> cache;

  /*---------------------------------------------------------
   * write‑back functor for the cache
   *-------------------------------------------------------*/
  void write_back(const Key &idx, const T &obj) {
    T copy = obj;
    disk.update(copy, idx);
  }

public:
  /*---------------------------------------------------------
   * ctors / dtor
   *-------------------------------------------------------*/
  CachedFileOperation()
      : disk(), cache([this](const Key &k, const T &v) { write_back(k, v); }) {}

  explicit CachedFileOperation(const std::string &fn)
      : disk(fn),
        cache([this](const Key &k, const T &v) { write_back(k, v); }) {}

  ~CachedFileOperation() { cache.flush(); }

  /*---------------------------------------------------------
   * identical public interface – thin forwarding wrappers
   *-------------------------------------------------------*/
  void initialise(std::string fn = "") { disk.initialise(fn); }

  void get_info(int &tmp, int n) { disk.get_info(tmp, n); }
  void write_info(int tmp, int n) { disk.write_info(tmp, n); }
  bool isEmpty() { return disk.isEmpty(); }

  /*---------------------------------------------------------
   * Data read / write with cache support
   *-------------------------------------------------------*/
  int write(T &t) {
    int idx = disk.write(t);  // page already on disk
    cache.put(idx, t, false); // clean page in cache
    return idx;
  }

  void read(T &t, const int idx) {
    if (!cache.get(idx, t)) { // miss → load from disk
      disk.read(t, idx);
      cache.put(idx, t, false); // now cached
    }
  }

  void update(T &t, const int idx) {
    cache.put(idx, t, true); // defer write‑back
    /* no direct disk access here – handled by cache */
  }

  void remove(int idx) {
    cache.flush();    // safest: flush first
    disk.remove(idx); // (does nothing today)
  }

  /* explicit flush for callers that want deterministic I/O */
  void flush() { cache.flush(); }
};

#endif // CACHED_FILE_OPERATION_HPP