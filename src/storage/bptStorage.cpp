#include "storage/bptStorage.hpp"

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::BPTStorage(
    const std::string &file_prefix) {
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

  if (i > 0 && leaf_node.children[i - 1] != -1) {
    DataBlock<Key, Value, BLOCK_SIZE> block;
    data_file.read(block, leaf_node.children[i - 1]);

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

    // insert key after the split
    for (int j = node.key_count; j >= i + 2; j--) {
      node.keys[j] = node.keys[j - 1];
      node.children[j + 1] = node.children[j];
    }

    node.keys[i + 1] = key;
    node.key_count++;
    node.children[node.key_count] = -1;

    if (node.key_count > NODE_SIZE) {
      split_node(node);
    } else {
      node_file.write(node, node.node_id);
      // already written data block, nothing to do
    }

  } else {
    // write the block back
    data_file.write(block, block.block_id);
    // nothing to do
  }
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
    DataBlock<Key, Value, BLOCK_SIZE> next_block;
    data_file.read(next_block, block.next_block_id);
    if (next_block.key_count > BLOCK_SIZE / 2) {
      // borrow elements
      Key new_key = block.borrow(next_block);
      node.children[i + 1] = new_key;
      node_file.write(node, node.node_id);
      data_file.write(block, block.block_id);
      data_file.write(next_block, block.next_block_id);
    } else {
      block.merge_block(next_block);
      for (int j = i; j <= node.key_count - 2; j++) {
        node.children[j] = node.children[j + 1];
      }
      node.key_count--;
      if (node.key_count < NODE_SIZE / 2) {
        // merge
        merge_nodes(node);
      } else {
        node_file.write(node, node.node_id);
      }
    }
  } else {
    // write the block back
    data_file.write(block, node.children[i]);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::merge_nodes(
    BPTNode<Key, NODE_SIZE> &node) {
  if (node.next_node_id == -1) {
    return;
  }
  BPTNode<Key, NODE_SIZE> next_node;
  node_file.read(next_node, node.next_node_id);
  if (next_node.key_count + node.key_count <= NODE_SIZE) {
    // merge
    for (int j = 0; j < node.key_count; j++) {
      node.keys[j + node.key_count] = next_node.keys[j];
      node.children[j + node.key_count] = next_node.children[j];
    }
    node.key_count += next_node.key_count;
    node.next_node_id = next_node.next_block_id;
    node_file.write(node, node.node_id);
    
    BPTNode<Key, NODE_SIZE> parent_node;
    node_file.read(parent_node, node.parent_id);

    delete_from_internal_node(parent_node, next_node.keys[0]);
  } else {
    // borrow elements
    node.keys[node.key_count] = next_node.keys[0];
    node.children[node.key_count + 1] = next_node.children[0];
    for (int j = 0; j < next_node.key_count; j++) {
      next_node.keys[j] = next_node.keys[j + 1];
      next_node.children[j] = next_node.children[j + 1];
    }
    next_node.children[next_node.key_count - 1] = next_node.children[next_node.key_count];

    node.key_count++;
    next_node.key_count--;

    BPTNode<Key, NODE_SIZE> parent_node;
    node_file.read(parent_node, node.parent_id);
    
    for (int j = 0; j < parent_node.key_count; j++) {
      if (parent_node.keys[j] == node.keys[0] && parent_node.keys[j + 1] == node.keys[node_count - 1]) {
        parent_node.keys[j + 1] = next_node.keys[0];
        break;
      }
    }

    node_file.write(parent_node,parent_node.node_id);
    node_file.write(node,node.node_id);
    node_file.write(next_node, next_node.node_id);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::split_node(
    BPTNode<Key, NODE_SIZE> &node) {
  if (node.is_root) {
    BPTNode<Key, NODE_SIZE> child_1, child_2;
    
  } else {
    BPTNode<Key, NODE_SIZE> new_node;
    for (int i = node.key_count / 2; i < node.key_count; i++) {
      new_node.children[i - node.key_count / 2] = node.children[i];
      new_node.keys[i - node.key_count / 2] = node.children[i];
      new_node.key_count++;
    }
    new_node.children[new_node.key_count] = node.children[NODE_SIZE + 1];
    new_node.is_leaf = node.is_leaf;
    new_node.parent_id = node.parent_id;
    node.key_count /= 2;
    new_node.node_id = node_file.write(new_node);
    new_node.next_node_id = node.next_node_id;
    node.next_node_id = new_node.node_id;
    node_file.write(node, node.node_id);
    node_file.write(new_node, new_node.node_id);
    
    BPTNode<Key, NODE_SIZE> parent_node;
    node_file.read(parent_node, node.parent_id);
    insert_into_internal_node(parent_node, new_node.keys[0], new_node.node_id);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_internal_node(
    BPTNode<Key, NODE_SIZE> &node, Key key, int child_index) {
  int i = 0;
  while (i < node.key_count && key > node.keys[i]) {
    i++;
  }
  for (int j = node.key_count; j > i; j--) {
    node.keys[j] = node.keys[j - 1];
    node.children[j + 1] = node.children[j];
  }
  node.keys[i] = key;
  node.key_count++;
  node.children[node.key_count] = child_index;
  if (node.key_count > NODE_SIZE) {
    split_node(node);
  } else {
    node_file.write(node, node.node_id);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_internal_node(
    BPTNode<Key, NODE_SIZE> &node, Key key) {
  if (node.is_root) {
    if (node.key_count > 2) {
      int i = 0;
      while (i < node.key_count && key > node.keys[i]) {
        i++;
      }
      for (int j = i; j < node.key_count - 1; j++) {
        node.keys[j] = node.keys[j + 1];
        node.children[j] = node.children[j + 1];
      }
      node.key_count--;
      node_file.write(node, node.node_id);
    } else {
      // TODO
      // new root node
      // remember to update config
    }
  } else {
    int i = 0;
    while (i < node.key_count && key > node.keys[i]) {
      i++;
    }
    for (int j = i; j < node.key_count - 1; j++) {
      node.keys[j] = node.keys[j + 1];
      node.children[j] = node.children[j + 1];
    }
    node.key_count--;
    if (node.key_count < NODE_SIZE / 2) {
      merge_nodes(node);
    } else {
      node_file.write(node, node.node_id);
    }
  }

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
