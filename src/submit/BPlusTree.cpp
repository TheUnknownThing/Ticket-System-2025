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

  sjtu::string64 MAXN;
  MAXN = MAXN.STRING64_MAX();

  BPTStorage<sjtu::string64, int, 6, 6> book("data", MAXN); // for debugging, normal: (57, 57)

  for (int i = 0; i < n; ++i) {
    std::cin >> op;
    // insert, delete, find
    if (op == "insert") {
      sjtu::string64 index;
      std::cin >> index;
      int value;
      std::cin >> value;
      std::cout << "Doing insert: " << index << " " << value << std::endl; // debug
      book.insert(index, value);
    } else if (op == "delete") {
      sjtu::string64 index;
      std::cin >> index;
      int value;
      std::cin >> value;
      std::cout << "Doing delete: " << index << " " << value << std::endl; // debug
      book.remove(index, value);
    } else if (op == "find") {
      sjtu::string64 index;
      std::cin >> index;
      std::cout << "Doing find: " << index << std::endl; // debug
      auto result = book.find(index);
      for (const auto &val : result) {
        std::cout << val << " ";
      }
      if (result.empty()) {
        std::cout << "null";
      }
      std::cout << std::endl;
    }
  }
  return 0;
}