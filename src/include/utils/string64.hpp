#ifndef STRING64_HPP
#define STRING64_HPP

#include <algorithm>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace sjtu {
class string64 {
private:
  char s[65];

public:
  string64() { s[0] = '\0'; }

  string64(const char *str) {
    if (str) {
      strncpy(s, str, 64);
      s[64] = '\0';
    } else {
      s[0] = '\0';
    }
  }

  string64(const string64 &other) { strcpy(s, other.s); }

  string64 &operator=(const string64 &other) {
    if (this != &other) {
      strcpy(s, other.s);
    }
    return *this;
  }

  string64 &operator=(const char *str) {
    if (str) {
      strncpy(s, str, 64);
      s[64] = '\0';
    } else {
      s[0] = '\0';
    }
    return *this;
  }

  char &operator[](size_t index) {
    if (index >= 64)
      throw std::out_of_range("Index out of bounds");
    return s[index];
  }

  const char &operator[](size_t index) const {
    if (index >= 64)
      throw std::out_of_range("Index out of bounds");
    return s[index];
  }

  const char *c_str() const { return s; }

  size_t size() const { return strlen(s); }

  size_t length() const { return size(); }

  bool empty() const { return s[0] == '\0'; }

  static constexpr size_t max_size() { return 64; }

  void clear() { s[0] = '\0'; }

  string64 &append(const string64 &other) {
    size_t current_size = size();
    size_t copy_len = std::min(64 - current_size, other.size());
    strncat(s, other.s, copy_len);
    s[64] = '\0';
    return *this;
  }

  string64 &append(const char *str) {
    if (str) {
      size_t current_size = size();
      size_t copy_len = std::min(64 - current_size, strlen(str));
      strncat(s, str, copy_len);
      s[64] = '\0';
    }
    return *this;
  }

  string64 operator+(const string64 &other) const {
    string64 result(*this);
    result.append(other);
    return result;
  }

  string64 &operator+=(const string64 &other) { return append(other); }

  bool operator==(const string64 &other) const {
    return strcmp(s, other.s) == 0;
  }

  bool operator!=(const string64 &other) const {
    return strcmp(s, other.s) != 0;
  }

  bool operator<(const string64 &other) const { return strcmp(s, other.s) < 0; }

  bool operator<=(const string64 &other) const {
    return strcmp(s, other.s) <= 0;
  }

  bool operator>(const string64 &other) const { return strcmp(s, other.s) > 0; }

  bool operator>=(const string64 &other) const {
    return strcmp(s, other.s) >= 0;
  }

  string64 substr(size_t pos, size_t len = 64) const {
    if (pos >= size()) {
      throw std::out_of_range("Position out of range");
    }

    string64 result;
    size_t actual_len = std::min(len, size() - pos);
    strncpy(result.s, s + pos, actual_len);
    result.s[actual_len] = '\0';
    return result;
  }

  string64 STRING64_MAX() const {
    string64 max_str;
    for (size_t i = 0; i < 64; ++i) {
      max_str.s[i] = 126;
    }
    max_str.s[64] = '\0';
    return max_str;
  }

  friend std::istream &operator>>(std::istream &is, string64 &str) {
    is >> str.s;
    return is;
  }

  friend std::ostream &operator<<(std::ostream &os, const string64 &str) {
    os << str.s;
    return os;
  }
};
} // namespace sjtu
#endif // STRING64_HPP