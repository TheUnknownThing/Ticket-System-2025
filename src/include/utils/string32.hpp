#ifndef STRING32_HPP
#define STRING32_HPP

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

namespace sjtu {
class string32 {
private:
  char s[33];

public:
  string32() { s[0] = '\0'; }

  string32(const char *str) {
    if (str) {
      strncpy(s, str, 32);
      s[32] = '\0';
    } else {
      s[0] = '\0';
    }
  }

  string32(const std::string& str) {
    if (!str.empty()) {
      strncpy(s, str.c_str(), 32);
      s[32] = '\0';
    } else {
      s[0] = '\0';
    }
  }

  string32(const string32 &other) { strcpy(s, other.s); }

  string32 &operator=(const string32 &other) {
    if (this != &other) {
      strcpy(s, other.s);
    }
    return *this;
  }

  string32 &operator=(const std::string &str) {
    if (!str.empty()) {
      strncpy(s, str.c_str(), 32);
      s[32] = '\0';
    } else {
      s[0] = '\0';
    }
    return *this;
  }

  string32 &operator=(const char *str) {
    if (str) {
      strncpy(s, str, 32);
      s[32] = '\0';
    } else {
      s[0] = '\0';
    }
    return *this;
  }

  char &operator[](size_t index) {
    if (index >= 32)
      throw std::out_of_range("Index out of bounds");
    return s[index];
  }

  const char &operator[](size_t index) const {
    if (index >= 32)
      throw std::out_of_range("Index out of bounds");
    return s[index];
  }

  const char *c_str() const { return s; }

  size_t size() const { return strlen(s); }

  size_t length() const { return size(); }

  bool empty() const { return s[0] == '\0'; }

  static constexpr size_t max_size() { return 32; }

  void clear() { s[0] = '\0'; }

  string32 &append(const string32 &other) {
    size_t current_size = size();
    size_t copy_len = std::min(32 - current_size, other.size());
    strncat(s, other.s, copy_len);
    s[32] = '\0';
    return *this;
  }

  string32 &append(const char *str) {
    if (str) {
      size_t current_size = size();
      size_t copy_len = std::min(32 - current_size, strlen(str));
      strncat(s, str, copy_len);
      s[32] = '\0';
    }
    return *this;
  }

  string32 operator+(const string32 &other) const {
    string32 result(*this);
    result.append(other);
    return result;
  }

  string32 &operator+=(const string32 &other) { return append(other); }

  bool operator==(const string32 &other) const {
    return strcmp(s, other.s) == 0;
  }

  bool operator!=(const string32 &other) const {
    return strcmp(s, other.s) != 0;
  }

  bool operator<(const string32 &other) const { return strcmp(s, other.s) < 0; }

  bool operator<=(const string32 &other) const {
    return strcmp(s, other.s) <= 0;
  }

  bool operator>(const string32 &other) const { return strcmp(s, other.s) > 0; }

  bool operator>=(const string32 &other) const {
    return strcmp(s, other.s) >= 0;
  }

  string32 substr(size_t pos, size_t len = 32) const {
    if (pos >= size()) {
      throw std::out_of_range("Position out of range");
    }

    string32 result;
    size_t actual_len = std::min(len, size() - pos);
    strncpy(result.s, s + pos, actual_len);
    result.s[actual_len] = '\0';
    return result;
  }

  static string32 string32_MAX() {
    string32 max_str;
    for (size_t i = 0; i < 32; ++i) {
      max_str.s[i] = 126;
    }
    max_str.s[32] = '\0';
    return max_str;
  }

  friend std::istream &operator>>(std::istream &is, string32 &str) {
    is >> str.s;
    return is;
  }

  friend std::ostream &operator<<(std::ostream &os, const string32 &str) {
    os << str.s;
    return os;
  }
};
} // namespace sjtu
#endif // STRING32_HPP