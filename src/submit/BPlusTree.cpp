#include "src/include/storage/bptStorage.hpp"
#include "utils/string64.hpp"
#include <iostream>
#include <functional>

signed main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  int n;
  std::cin >> n;
  std::string op;

  size_t MAXN = ULONG_MAX;

  BPTStorage<size_t, int, 55, 53> book("data", MAXN);

  std::hash<std::string> string_hasher;

  for (int i = 0; i < n; ++i) {
    std::cin >> op;
    if (op == "insert") {
      std::string index_str;
      std::cin >> index_str;
      int value;
      std::cin >> value;
      size_t index_hash = string_hasher(index_str); 
      book.insert(index_hash, value);
    } else if (op == "delete") {
      std::string index_str;
      std::cin >> index_str;
      int value;
      std::cin >> value;
      size_t index_hash = string_hasher(index_str);
      book.remove(index_hash, value);
    } else if (op == "find") {
      std::string index_str;
      std::cin >> index_str;
      size_t index_hash = string_hasher(index_str);
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