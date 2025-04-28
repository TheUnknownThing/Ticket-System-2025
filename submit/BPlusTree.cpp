#include "storage/bptStorage.hpp"
#include <iostream>

signed main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  int n;
  std::cin >> n;
  std::string op;

  BPTStorage<char[65], int, 8192, 8192> book("data");

  for (int i = 0; i < n; ++i) {
    std::cin >> op;
    // insert, delete, find
    if (op == "insert") {
      char index[65];
      std::cin >> index;
      int value;
      std::cin >> value;
      book.insert(index, value);
    } else if (op == "delete") {
      char index[65];
      std::cin >> index;
      int value;
      std::cin >> value;
      book.remove(index, value);
    } else if (op == "find") {
      char index[65];
      std::cin >> index;
      auto result = book.find(index);
      for (const auto &val : result) {
        std::cout << val << " ";
      }
      std::cout << "\n";
    }
  }
  return 0;
}