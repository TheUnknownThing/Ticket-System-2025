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

  // 1. root is leaf
  if (root_node.is_leaf) {

  } else {
    
  }

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
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_leaf_node(
    BPTNode<Key, NODE_SIZE>& node, Key key, Value value) {
  // TODO
  
  return true;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_internal_node(
    BPTNode<Key, NODE_SIZE>& node, Key key, int child_index) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_internal_node(
    BPTNode<Key, NODE_SIZE>& node, Key key) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_leaf_node(
    BPTNode<Key, NODE_SIZE>& node, Key key, Value value) {
  // TODO
  return true;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::merge_leaf_nodes(
    int left_index, int right_index) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::merge_internal_nodes(
    int left_index, int right_index) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::split_leaf_node(
    BPTNode<Key, NODE_SIZE>& node, Key key, Value value) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::split_internal_node(
    BPTNode<Key, NODE_SIZE>& node, Key key, int child_index) {
  // TODO
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
  if (node_file.isEmpty()) {
    root_node.parent_id = -1;
    root_node.is_leaf = true;
    root_node.key_count = 0;
    for (int i = 0; i < NODE_SIZE; i++) {
      root_node.keys[i] = 0;
      root_node.children[i] = -1;
    }
    root_node.children[NODE_SIZE] = -1;

    root_index = node_file.write(root_node);
    root_node.node_id = root_index;

    node_count = 1;
    data_block_count = 0;
    node_file.write_info(root_index, 1);
    node_file.write_info(node_count, 2);
  } else {
    node_file.get_info(root_index, 1);
    node_file.get_info(node_count, 2);

    node_file.read(root_node, root_index);
  }
}
