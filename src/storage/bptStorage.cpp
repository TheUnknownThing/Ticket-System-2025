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
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert(Key key,
                                                           Value value) {

  int leaf_index = find_leaf_node(key);
  BPTNode<Key, NODE_SIZE> leaf_node;
  node_file.read(leaf_node, leaf_index);
  insert_into_leaf_node(leaf_node, key, value);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
bool BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::remove(Key key,
                                                           Value value) {
  int leaf_index = find_leaf_node(key);
  BPTNode<Key, NODE_SIZE> leaf_node;
  node_file.read(leaf_node, leaf_index);
  delete_from_leaf_node(leaf_node, key, value);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
sjtu::vector<Value>
BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::find(Key key) {
  sjtu::vector<Value> result;
  int leaf_index = find_leaf_node(key);
  BPTNode<Key, NODE_SIZE> leaf_node;
  node_file.read(leaf_node, leaf_index);
  
  int i = 0;
  while (i < leaf_node.key_count && key > leaf_node.keys[i]) {
    i++;
  }
  
  if (i < leaf_node.key_count && leaf_node.children[i] != -1) {
    DataBlock<Key, Value, BLOCK_SIZE> block;
    data_file.read(block, leaf_node.children[i]);
    
    // Check values in this block
    for (int j = 0; j < block.key_count; j++) {
      if (block.data[j].first == key) {
        result.push_back(block.data[j].second);
      } else if (block.data[j].first > key) {
        break;
      }
    }
    
    while (block.next_block_id != -1) {
      data_file.read(block, block.next_block_id);
      if (block.key_count == 0 || block.data[0].first > key) {
        break;
      }
      bool found_key = false;
      
      for (int j = 0; j < block.key_count; j++) {
        if (block.data[j].first == key) {
          result.push_back(block.data[j].second);
          found_key = true;
        } else if (block.data[j].first > key) {
          found_key = false;
          break;
        }
      }
      
      if (!found_key) {
        break;
      }
    }
  }
  
  if (i > 0 && leaf_node.children[i-1] != -1) {
    DataBlock<Key, Value, BLOCK_SIZE> block;
    data_file.read(block, leaf_node.children[i-1]);
    
    for (int j = 0; j < block.key_count; j++) {
      if (block.data[j].first == key) {
        result.push_back(block.data[j].second);
      }
    }
    
    while (block.next_block_id != -1) {
      data_file.read(block, block.next_block_id);
      if (block.key_count == 0 || block.data[0].first > key) {
        break;
      }
      bool found_key = false;
      
      for (int j = 0; j < block.key_count; j++) {
        if (block.data[j].first == key) {
          result.push_back(block.data[j].second);
          found_key = true;
        } else if (block.data[j].first > key) {
          found_key = false;
          break;
        }
      }
      
      if (!found_key) {
        break;
      }
    }
  }
  
  return result;
}

/*
 * Begin of private methods
 */

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
int BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::find_leaf_node(Key key) {
  while (!root_node.is_leaf) {
    int child_index = 0;
    while (child_index < root_node.key_count &&
           key > root_node.keys[child_index]) {
      child_index++;
    }
    int child_id = root_node.children[child_index];
    BPTNode<Key, NODE_SIZE> child_node;
    node_file.read(child_node, child_id);
    if (child_node.is_leaf) {
      return child_id;
    } else {
      root_node = child_node;
    }
  }
  return root_index;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_leaf_node(
    BPTNode<Key, NODE_SIZE> &node, Key key, Value value) {

  // insert
  int i = 0;
  while (i < node.key_count && key > node.keys[i]) {
    i++;
  }

  DataBlock<Key, Value, BLOCK_SIZE> block;
  if (node.children[i] == -1) {
    // create a new block
    block.block_id = data_file.write(block);
    block.key_count = 0;
    node.children[i] = block.block_id;
  } else {
    // read the existing block
    data_file.read(block, node.children[i]);
  }

  auto [need_split, new_block] = block.insert(key, value);

  if (need_split) {
    // need to split the block
    new_block.block_id = data_file.write(new_block);
    data_file.write(block, block.block_id);
    block.next_block_id = new_block.block_id;
    data_file.write(block, block.block_id);

    // update the node

    // TODO: call insert_into_internal_node
  } else {
    // write the block back
    data_file.write(block, block.block_id);
    // nothing to do
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_internal_node(
    BPTNode<Key, NODE_SIZE> &node, Key key, int child_index) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_internal_node(
    BPTNode<Key, NODE_SIZE> &node, Key key) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_leaf_node(
    BPTNode<Key, NODE_SIZE> &node, Key key, Value value) {
  int i = 0;
  while (i < node.key_count && key > node.keys[i]) {
    i++;
  }
  if (i == node.key_count || node.keys[i] != key) {
    // key not found
    return;
  }
  DataBlock<Key, Value, BLOCK_SIZE> block;
  data_file.read(block, node.children[i]);
  auto [deleted, need_merge] = block.delete_key(key);
  if (deleted && need_merge && block.next_block_id != -1) {
    // TODO: merge the block with the next block
  } else {
    // write the block back
    data_file.write(block, node.children[i]);
  }
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
    BPTNode<Key, NODE_SIZE> &node) {

  if (root_node.is_leaf) {
    // split the root node
    BPTNode<Key, NODE_SIZE> new_node;
    new_node.is_leaf = true;
    new_node.key_count = NODE_SIZE / 2;
    new_node.parent_id = root_index;
    new_node.node_id = node_file.write(new_node);
    new_node.children[0] = node.children[NODE_SIZE / 2];
    node.key_count = NODE_SIZE / 2;
    for (int i = 0; i < new_node.key_count; i++) {
      new_node.keys[i] = node.keys[i + NODE_SIZE / 2];
      new_node.children[i + 1] = node.children[i + NODE_SIZE / 2 + 1];
    }
    for (int i = new_node.key_count; i < NODE_SIZE; i++) {
      node.keys[i] = 0;
      node.children[i + 1] = -1;
    }
    node.children[node.key_count] = -1;
    node_file.write(node, node.node_id);
    node_file.write(new_node, new_node.node_id);
    root_node.is_leaf = false;
    root_node.key_count = 1;
    root_node.keys[0] = new_node.keys[0];
    root_node.children[0] = node.node_id;
    root_node.children[1] = new_node.node_id;
    root_node.children[2] = -1;
    root_node.parent_id = -1;
    root_node.node_id = node_file.write(root_node);
    node_file.write_info(root_index, 1);
    node_count++;
    node_file.write_info(node_count, 2);
    return;
  }

  // split a non-root node
  BPTNode<Key, NODE_SIZE> new_node;
  new_node.is_leaf = true;
  new_node.key_count = NODE_SIZE / 2;
  new_node.parent_id = node.parent_id;
  new_node.node_id = node_file.write(new_node);
  new_node.children[0] = node.children[NODE_SIZE / 2];
  node.key_count = NODE_SIZE / 2;
  for (int i = 0; i < new_node.key_count; i++) {
    new_node.keys[i] = node.keys[i + NODE_SIZE / 2];
    new_node.children[i + 1] = node.children[i + NODE_SIZE / 2 + 1];
  }
  for (int i = new_node.key_count; i < NODE_SIZE; i++) {
    node.keys[i] = 0;
    node.children[i + 1] = -1;
  }
  node.children[node.key_count] = -1;
  node_file.write(node, node.node_id);
  node_file.write(new_node, new_node.node_id);
  // update the parent node
  BPTNode<Key, NODE_SIZE> parent_node;
  node_file.read(parent_node, node.parent_id);
  int i = 0;
  while (i < parent_node.key_count && new_node.keys[0] > parent_node.keys[i]) {
    i++;
  }
  for (int j = parent_node.key_count; j > i; j--) {
    parent_node.keys[j] = parent_node.keys[j - 1];
    parent_node.children[j + 1] = parent_node.children[j];
  }
  parent_node.keys[i] = new_node.keys[0];
  parent_node.children[i + 1] = new_node.node_id;
  parent_node.key_count++;
  parent_node.children[parent_node.key_count + 1] = -1;
  node_file.write(parent_node, parent_node.node_id);
  if (parent_node.key_count == NODE_SIZE) {
    // need to split the parent node
    split_internal_node(parent_node);
  }
  node_file.write_info(node_count, 2);
  return;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::split_internal_node(
    BPTNode<Key, NODE_SIZE> &node) {
  // TODO
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::FileInit() {
  if (node_file.isEmpty()) {
    root_node.parent_id = -1;
    root_node.is_leaf = true;
    root_node.is_root = true;
    root_node.next_node_id = -1;
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
