#include "storage/bptStorage.hpp"

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::BPTStorage(
    const std::string &file_prefix) {
  // TODO
  node_file.initialise(file_prefix + "_node");
  data_file.initialise(file_prefix + "_data");
  FileInit();
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::~BPTStorage() {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert(Key key,
                                                           Value value) {
  // TODO
  return true;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::remove(Key key,
                                                           Value value) {
  // TODO
  return true;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
Value *BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::find(Key key) {
  // TODO
  return nullptr;
}

/*
 * Begin of private methods
 */

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::write_node(
    int index, BPTNode<Key, NODE_SIZE> &node) {
  node_file.write_info(index, node);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::read_node(
    int index, BPTNode<Key, NODE_SIZE> &node) {
  node_file.get_info(node, index);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::write_data_block(
    int index, DataBlock<Key, Value, BLOCK_SIZE> &block) {
  data_file.write_info(index, block);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::read_data_block(
    int index, DataBlock<Key, Value, BLOCK_SIZE> &block) {
  data_file.get_info(block, index);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
int BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::find_leaf_node(Key key) {
  // TODO
  return -1;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
int BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::create_node(bool is_leaf) {
  // TODO
  return -1;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
int BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::create_data_block() {
  // TODO
  return -1;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_internal_node(
    int index, Key key, int child_index) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_leaf_node(
    int index, Key key, Value value) {
  // TODO
  return true;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_internal_node(
    int index, Key key) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_leaf_node(
    int index, Key key, Value value) {
  // TODO
  return true;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
Value *BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::find_in_leaf(int index,
                                                                   Key key) {
  // TODO
  return nullptr;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::split_node(int node_index) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::merge_nodes(
    int left_index, int right_index) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::FileInit() {
  if (node_file.is_empty()) {
    // TODO, I will use a pseudo-root node
  } 
  if (data_file.is_empty()) {
    // TODO
  }
}
