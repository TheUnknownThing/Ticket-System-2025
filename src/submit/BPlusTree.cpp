#include "src/include/storage/bptStorage.hpp"
#include "stl/string64.hpp"
#include <iostream>

signed main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  int n;
  std::cin >> n;
  std::string op;

  BPTStorage<sjtu::string64, int, 7, 54> book("data");

  for (int i = 0; i < n; ++i) {
    std::cin >> op;
    // insert, delete, find
    if (op == "insert") {
      sjtu::string64 index;
      std::cin >> index;
      int value;
      std::cin >> value;
      book.insert(index, value);
    } else if (op == "delete") {
      sjtu::string64 index;
      std::cin >> index;
      int value;
      std::cin >> value;
      book.remove(index, value);
    } else if (op == "find") {
      sjtu::string64 index;
      std::cin >> index;
      auto result = book.find(index);
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