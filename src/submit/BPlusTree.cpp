#include "src/include/storage/bptStorage.hpp"
#include "utils/string32.hpp"
#include <iostream>
#include <cstddef>
#include <cstring>

struct CustomStringHasher {
    size_t operator()(const char* str) const {
        size_t hash = 5381;
        const size_t salt = 33;
        int c;

        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c + salt;
        }

        return hash;
    }
};

signed main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  int n;
  std::cin >> n;
  std::string op;

  BPTStorage<size_t, int, 20, 20> book("data", ULONG_MAX);

  CustomStringHasher custom_hasher;

  for (int i = 0; i < n; ++i) {
    std::cin >> op;
    if (op == "insert") {
      sjtu::string32 index_str;
      std::cin >> index_str;
      int value;
      std::cin >> value;
      size_t index_hash = custom_hasher(index_str.c_str());
      book.insert(index_hash, value);
    } else if (op == "delete") {
      sjtu::string32 index_str;
      std::cin >> index_str;
      int value;
      std::cin >> value;
      size_t index_hash = custom_hasher(index_str.c_str());
      book.remove(index_hash, value);
    } else if (op == "find") {
      sjtu::string32 index_str;
      std::cin >> index_str;
      size_t index_hash = custom_hasher(index_str.c_str());
      auto result = book.find(index_hash);
      for (const auto &val : result) {
        std::cout << val << " ";
      }
      if (result.empty()) {
        std::cout << "null";
      }
      std::cout << "\n";
    }
  }
  return 0;
}