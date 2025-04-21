#ifndef BPT_FILEOPERATION_HPP
#define BPT_FILEOPERATION_HPP

#include <fstream>

using std::fstream;
using std::ifstream;
using std::ofstream;
using std::string;

template <class T, int info_len = 2> class FileOperation {
private:
  fstream file;
  string file_name;
  int sizeofT = sizeof(T);

  void open_file(std::ios_base::openmode mode) {
    if (!file.is_open() || (file.rdstate() & std::ios::failbit)) {
      if (file.is_open())
        file.close();
      file.open(file_name, mode);
    }
  }

public:
  FileOperation() = default;

  FileOperation(const string &file_name) : file_name(file_name) {}

  ~FileOperation() {
    if (file.is_open())
      file.close();
  }

  void initialise(string FN = "") {
    if (FN != "")
      file_name = FN;
    if (file.is_open())
      file.close();

    file.open(file_name, std::ios::out);
    int tmp = 0;
    for (int i = 0; i < info_len; ++i)
      file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
    file.close();
  }

  //读出第n个int的值赋给tmp，1_base
  void get_info(int &tmp, int n) {
    if (n > info_len)
      return;

    open_file(std::ios::in | std::ios::binary);
    file.seekg((n - 1) * sizeof(int));
    file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
  }

  //将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len)
      return;

    open_file(std::ios::in | std::ios::out | std::ios::binary);
    file.seekp((n - 1) * sizeof(int));
    file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
  }

  //在文件合适位置写入类对象t，并返回写入的位置索引index
  int write(T &t) {
    open_file(std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(0, std::ios::end);
    int index = file.tellp();
    file.write(reinterpret_cast<char *>(&t), sizeof(T));
    return index;
  }

  //用t的值更新位置索引index对应的对象
  void update(T &t, const int index) {
    open_file(std::ios::in | std::ios::out | std::ios::binary);
    file.seekp(index);
    file.write(reinterpret_cast<char *>(&t), sizeof(T));
  }

  //读出位置索引index对应的T对象的值并赋值给t
  void read(T &t, const int index) {
    open_file(std::ios::in | std::ios::binary);
    file.seekg(index);
    file.read(reinterpret_cast<char *>(&t), sizeof(T));
  }

  //删除位置索引index对应的对象
  void Delete(int index) {
    // No implementation needed for space recovery
  }
};

#endif // BPT_FILEOPERATION_HPP