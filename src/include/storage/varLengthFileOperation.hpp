#ifndef VAR_LENGTH_INT_ARRAY_FILE_OPERATION_HPP
#define VAR_LENGTH_INT_ARRAY_FILE_OPERATION_HPP

#include "stl/vector.hpp"
#include <cassert>
#include <fstream>
#include <string>

using sjtu::vector;

class VarLengthIntArrayFileOperation {
private:
  std::fstream file;
  std::string file_name;
  const int info_len;

public:
  VarLengthIntArrayFileOperation(const std::string &fname, int info_length = 2)
      : file_name(fname), info_len(info_length > 0 ? info_length : 2) {}

  ~VarLengthIntArrayFileOperation() {
    if (file.is_open()) {
      file.close();
    }
  }

  void initialise(std::string FN = "") {
    if (!FN.empty()) {
      if (file.is_open()) {
        file.close();
      }
      file_name = FN;
    }

    if (file.is_open()) {
      file.clear();
      file.seekg(0);
      file.seekp(0);
      return;
    }

    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
      file.clear();

      std::ofstream creator(file_name, std::ios::out | std::ios::binary);
      creator.close();

      file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);

      int tmp = 0;
      file.seekp(0);
      for (int i = 0; i < info_len; ++i) {
        file.write(reinterpret_cast<const char *>(&tmp), sizeof(int));
      }
      file.flush();
    }
    file.clear();
    file.seekg(0);
    file.seekp(0);
  }

  void get_info(int &tmp_val, int n) {
    if (n <= 0 || n > info_len) {
      return;
    }
    if (!file.is_open()) {
      initialise();
    }
    file.seekg(static_cast<std::streamoff>(n - 1) * sizeof(int));
    file.read(reinterpret_cast<char *>(&tmp_val), sizeof(int));
  }

  void write_info(int val, int n) {
    if (n <= 0 || n > info_len) {
      return;
    }
    if (!file.is_open()) {
      initialise();
    }
    file.seekp(static_cast<std::streamoff>(n - 1) * sizeof(int));
    file.write(reinterpret_cast<const char *>(&val), sizeof(int));
  }

  int write(const int *data_ptr, int num_elements) {
    file.seekp(0, std::ios::end);
    int index = file.tellp();

    file.write(reinterpret_cast<const char *>(&num_elements), sizeof(int));

    if (num_elements > 0) {
      file.write(reinterpret_cast<const char *>(data_ptr),
                 static_cast<std::streamsize>(num_elements) * sizeof(int));
    }
    return index;
  }

  int write(int init_value, int num_elements) {
    if (num_elements <= 0) {
      return -1; // Invalid number of elements
    }

    file.seekp(0, std::ios::end);
    int index = file.tellp();
    file.write(reinterpret_cast<const char *>(&num_elements), sizeof(int));
    if (num_elements > 0) {
      int data_buffer[num_elements];
      for (int i = 0; i < num_elements; ++i) {
        data_buffer[i] = init_value;
      }
      file.write(reinterpret_cast<const char *>(data_buffer),
                 static_cast<std::streamsize>(num_elements) * sizeof(int));
    }
    return index;
  }

  vector<int> read(int index) {
    file.seekg(index);
    int num_elements;
    file.read(reinterpret_cast<char *>(&num_elements), sizeof(int));

    if (!file || num_elements <= 0) {
      return vector<int>();
    }

    int data_buffer[num_elements];
    file.read(reinterpret_cast<char *>(data_buffer),
              static_cast<std::streamsize>(num_elements) * sizeof(int));

    vector<int> result;
    for (int i = 0; i < num_elements; ++i) {
      result.push_back(data_buffer[i]);
    }
    return result;
  }

  vector<int> read(int index, int offset, int num_elements) {
    file.seekg(index + sizeof(int) + offset * sizeof(int));
    int data_buffer[num_elements];
    file.read(reinterpret_cast<char *>(data_buffer),
              static_cast<std::streamsize>(num_elements) * sizeof(int));
    vector<int> result;
    for (int i = 0; i < num_elements; ++i) {
      result.push_back(data_buffer[i]);
    }
    return result;
  }

  void update(int index, const int *data_ptr) {
    file.seekg(index);
    int num_elements;
    file.read(reinterpret_cast<char *>(&num_elements), sizeof(int));

    file.seekp(index + sizeof(int));
    if (data_ptr != nullptr) {
      file.write(reinterpret_cast<const char *>(data_ptr),
                 static_cast<std::streamsize>(num_elements) * sizeof(int));
    }
  }

  void update(int index, int offset, int num_elements, const int *data_ptr) {
    file.seekp(index + sizeof(int) + offset * sizeof(int));
    if (data_ptr != nullptr) {
      file.write(reinterpret_cast<const char *>(data_ptr),
                 static_cast<std::streamsize>(num_elements) * sizeof(int));
    }
  }

  void update(int index, const vector<int> &data) {
    if (data.empty()) {
      return;
    }
    file.seekp(index + sizeof(int));
    file.write(reinterpret_cast<const char *>(data.data()),
               static_cast<std::streamsize>(data.size()) * sizeof(int));
  }

  void update(int index, int offset, int num_elements,
              const vector<int> &data) {
    if (data.empty()) {
      return;
    }
    file.seekp(index + sizeof(int) + offset * sizeof(int));
    file.write(reinterpret_cast<const char *>(data.data()),
               static_cast<std::streamsize>(num_elements) * sizeof(int));
  }

  void remove(int index) {
    int marked_num_elements = 0;
    file.seekp(index);
    file.write(reinterpret_cast<const char *>(&marked_num_elements),
               sizeof(int));
  }

  bool isEmpty() {
    file.seekg(0, std::ios::end);
    return file.tellg() == static_cast<std::streamoff>(info_len * sizeof(int));
  }

  void clear() {
    std::ofstream truncator(file_name,
                            std::ios::out | std::ios::binary | std::ios::trunc);
    truncator.close();

    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);

    int tmp = 0;
    file.seekp(0);
    for (int i = 0; i < info_len; ++i) {
      file.write(reinterpret_cast<const char *>(&tmp), sizeof(int));
    }
    file.flush();
    file.clear();
    file.seekg(0);
    file.seekp(0);
  }
};

#endif // VAR_LENGTH_INT_ARRAY_FILE_OPERATION_HPP