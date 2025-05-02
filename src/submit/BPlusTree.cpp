#include "src/include/storage/bptStorage.hpp"
#include "stl/string64.hpp"
#include <iostream>

constexpr bool DEBUG_MODE = false;

#define DEBUG_PRINT(x) if (DEBUG_MODE) { std::cout << x << std::endl; }

signed main() {
  std::ios::sync_with_stdio(false);
  std::cin.tie(nullptr);
  std::cout.tie(nullptr);
  int n;
  std::cin >> n;
  std::string op;

  sjtu::string64 MAXN;
  MAXN = MAXN.STRING64_MAX();

  BPTStorage<sjtu::string64, int, 56, 54> book("data", MAXN); // for debugging, normal: (57, 55)

  for (int i = 0; i < n; ++i) {
    std::cin >> op;
    // insert, delete, find
    if (op == "insert") {
      sjtu::string64 index;
      std::cin >> index;
      int value;
      std::cin >> value;
      DEBUG_PRINT("Doing insert: " << index << " " << value);
      book.insert(index, value);
    } else if (op == "delete") {
      sjtu::string64 index;
      std::cin >> index;
      int value;
      std::cin >> value;
      DEBUG_PRINT("Doing delete: " << index << " " << value);
      book.remove(index, value);
    } else if (op == "find") {
      sjtu::string64 index;
      std::cin >> index;
      DEBUG_PRINT("Doing find: " << index);
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