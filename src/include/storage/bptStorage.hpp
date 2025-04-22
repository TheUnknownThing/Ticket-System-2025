#ifndef BPT_STORAGE_HPP
#define BPT_STORAGE_HPP

#include "bptNode.hpp"
#include "fileOperation.hpp"
#include "stl/vector.hpp"
#include <string>

template <typename Key, typename Value, size_t NODE_SIZE = 4,
          size_t BLOCK_SIZE = 4>
class BPTStorage {
public:
  BPTStorage(const std::string &file_prefix);
  ~BPTStorage();

  /*
   * @brief Insert a key-value pair into the B+ tree.
   */
  void insert(Key key, Value value);

  /*
   * @brief Remove a key-value pair from the B+ tree.
   * @return true if the removal is successful, false otherwise.
   */
  bool remove(Key key, Value value);

  /*
   * @brief Find a key in the B+ tree.
   * @return vector of values associated with the key, or nullptr if not found.
   */
  sjtu::vector<Value> find(Key key);

private:
  FileOperation<BPTNode<Key, NODE_SIZE>> node_file;
  FileOperation<DataBlock<Key, Value, BLOCK_SIZE>> data_file;

  std::string node_file_name;
  std::string data_file_name;

  int root_index;
  size_t node_count;
  [[maybe_unused]] size_t data_block_count;

  BPTNode<Key, NODE_SIZE> root_node;

  int find_leaf_node(Key key);

  /*
   * @brief Insert a key-value pair into a leaf node.
   * @return false if need to split the node, true otherwise.
   */
  bool insert_into_leaf_node(BPTNode<Key, NODE_SIZE> &node, Key key,
                             Value value);

  /*
   * @brief Insert a key and child index into an internal node.
   * @return false if need to split the node, true otherwise.
   */
  bool insert_into_internal_node(BPTNode<Key, NODE_SIZE> &node, Key key,
                                 int child_index);

  /*
   * @brief Delete a key from an internal node.
   * @return false if need to merge the node, true otherwise.
   */
  bool delete_from_internal_node(BPTNode<Key, NODE_SIZE> &node, Key key);

  /*
   * @brief Delete a key-value pair from a leaf node.
   * @return false if need to merge the node, true otherwise.
   */
  bool delete_from_leaf_node(BPTNode<Key, NODE_SIZE> &node, Key key,
                             Value value);

  /*
   * @brief Merge two leaf nodes.
   */
  void merge_leaf_nodes(int left_index, int right_index);

  /*
   * @brief Merge two internal nodes.
   */
  void merge_internal_nodes(int left_index, int right_index);

  /*
   * @brief Split a leaf node.
   */
  void split_leaf_node(BPTNode<Key, NODE_SIZE> &node);

  /*
   * @brief Split an internal node.
   */
  void split_internal_node(BPTNode<Key, NODE_SIZE> &node);

  void FileInit();
};

#endif // BPT_STORAGE_HPP