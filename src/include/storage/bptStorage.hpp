#ifndef BPT_STORAGE_HPP
#define BPT_STORAGE_HPP

#include "bptNode.hpp"
#include "fileOperation.hpp"
#include "stl/vector.hpp"
#include <functional>
#include <iostream>
#include <string>

template <typename Key, typename Value, size_t NODE_SIZE = 4,
          size_t BLOCK_SIZE = 4>
class BPTStorage {
public:
  using NodeType = BPTNode<Key, NODE_SIZE>;
  using BlockType = DataBlock<Key, Value, BLOCK_SIZE>;

  BPTStorage(const std::string &file_prefix, const Key &MAX_KEY);
  ~BPTStorage();

  /*
   * @brief Insert a key-value pair into the B+ tree.
   */
  void insert(Key key, Value value);

  /*
   * @brief Remove a key-value pair from the B+ tree.
   */
  void remove(Key key, Value value);

  /*
   * @brief Find a key in the B+ tree.
   * @return vector of values associated with the key, or nullptr if not found.
   */
  sjtu::vector<Value> find(Key key);

private:
  FileOperation<NodeType> node_file;
  FileOperation<BlockType> data_file;

  std::string node_file_name;
  std::string data_file_name;
  Key MAX_KEY;

  int root_index;

  NodeType root_node;

  int find_leaf_node(Key key);

  /*
   * @brief Insert a key-value pair into a leaf node.
   */
  void insert_into_leaf_node(int index, Key key, Value value);

  /*
   * @brief Delete a key-value pair from a leaf node.
   */
  void delete_from_leaf_node(int index, Key key, Value value);

  /*
   * @brief Merge two leaf nodes.
   */
  void merge_nodes(int index);

  /*
   * @brief Split a node.
   * @note All the corner cases when inserting is handled here
   * Several cases:
   * 1. node is root. -> new root.
   * 2. node isn't root. -> split, then call insert_into_internal_node for its
   * parents.
   */
  void split_node(int index);

  /*
   * @brief Insert a key and child index into an internal node.
   * @note This function would only be called in split_leaf_node or
   * split_internal_node
   */
  void insert_into_internal_node(int index, Key key, int child_index, int pos);

  /*
   * @brief Delete a key from an internal node.
   * @note This function would only be called in merge_leaf_node or
   * merge_internal_node
   * @note All the corner cases when deleting is handled here.
   * Several cases:
   * 1. node is root. -> Key == 1 -> New root is its child.
   * 2. node is root. -> Key >= 2 -> No Changes.
   * 3. node isn't root. -> Key <= NODE_SIZE / 2 -> Merge_nodes.
   * 4. node isn't root. -> Key > NODE_SIZE / 2 -> No Changes.
   */
  void delete_from_internal_node(int index, int pos);

  void FileInit();

  sjtu::vector<Value> sort_result(sjtu::vector<Value> &vec);
};

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::BPTStorage(
    const std::string &file_prefix, const Key &MAX_KEY)
    : MAX_KEY(MAX_KEY) {
  node_file.initialise(file_prefix + "_node");
  data_file.initialise(file_prefix + "_data");
  FileInit();
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::~BPTStorage() {
  node_file.write_info(root_index, 1);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert(Key key,
                                                           Value value) {
  int leaf_index = find_leaf_node(key);
  insert_into_leaf_node(leaf_index, key, value);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::remove(Key key,
                                                           Value value) {
  int leaf_index = find_leaf_node(key);
  delete_from_leaf_node(leaf_index, key, value);
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
sjtu::vector<Value>
BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::find(Key key) {
  sjtu::vector<Value> result;
  int leaf_index = find_leaf_node(key);
  NodeType leaf_node;
  node_file.read(leaf_node, leaf_index);

  int i = 0;
  while (i < leaf_node.key_count && key > leaf_node.keys[i]) {
    i++;
  }

  if (i < leaf_node.key_count) {
    BlockType block;
    data_file.read(block, leaf_node.children[i]);

    // Check current block
    for (int j = 0; j < block.key_count; j++) {
      if (block.data[j].first == key) {
        result.push_back(block.data[j].second);
      } else if (block.data[j].first > key) {
        break;
      }
    }

    // Check linked blocks
    while (block.next_block_id != -1) {
      data_file.read(block, block.next_block_id);
      if (block.key_count == 0 || block.data[0].first > key) {
        break;
      }
      for (int j = 0; j < block.key_count; j++) {
        if (block.data[j].first == key) {
          result.push_back(block.data[j].second);
        } else if (block.data[j].first > key) {
          break;
        }
      }
    }
  }

  return sort_result(result);
}

/*
 * Begin of private methods
 */

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
int BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::find_leaf_node(Key key) {
  node_file.read(root_node, root_index);
  NodeType current_node = root_node;
  int current_id = root_index;

  while (!current_node.is_leaf) {
    int child_index = 0;
    while (child_index < current_node.key_count - 1 &&
           key > current_node.keys[child_index]) {
      child_index++;
    }
    int child_id = current_node.children[child_index];
    current_id = child_id;
    node_file.read(current_node, child_id);
  }

  return current_id;
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_leaf_node(
    int index, Key key, Value value) {

  NodeType node;
  node_file.read(node, index);
  // insert
  int i = 0;
  while (i < node.key_count && key > node.keys[i]) {
    i++;
  }

  BlockType block;
  if (node.children[i] == -1) {
    // create a new block
    block.block_id = data_file.write(block);
    block.parent_id = index;
    block.key_count = 0;
    node.children[i] = block.block_id;
    node.keys[i] = key;
    node.key_count++;
    if (i > 0) {
      BlockType prev_block;
      data_file.read(prev_block, node.children[i - 1]);
      block.next_block_id = prev_block.next_block_id;
      prev_block.next_block_id = block.block_id;
      data_file.update(prev_block, prev_block.block_id);
    } else if (i == 0 && node.key_count > 1) {
      block.next_block_id = node.children[1];
      data_file.update(block, block.block_id);
    }
  } else {
    // read the existing block
    data_file.read(block, node.children[i]);
  }

  auto [need_split, new_block] = block.insert_key(key, value);

  if (need_split) {
    // std::cout << "Split New Block" << std::endl; // debug
    // Write new block
    new_block.block_id = data_file.write(new_block);
    new_block.parent_id = index;
    block.next_block_id = new_block.block_id;
    data_file.update(block, block.block_id);
    data_file.update(new_block, new_block.block_id);

    // Insert separator key
    for (int j = node.key_count; j > i + 1; j--) {
      node.keys[j] = node.keys[j - 1];
      node.children[j] = node.children[j - 1];
    }

    node.keys[i + 1] = node.keys[i];
    node.keys[i] = block.data[block.key_count - 1].first;
    node.children[i + 1] = new_block.block_id;
    node.key_count++;

    if (node.key_count >= NODE_SIZE) {
      node_file.update(node, node.node_id);
      split_node(index);
    } else {
      node_file.update(node, node.node_id);
    }
  } else {
    node_file.update(node, node.node_id);
    data_file.update(block, block.block_id);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_leaf_node(
    int index, Key key, Value value) {
  NodeType node;
  node_file.read(node, index);
  int i = 0;
  while (i < node.key_count && key > node.keys[i]) {
    i++;
  }
  if (i == node.key_count) {
    // key not found
    return;
  }
  BlockType block;
  bool deleted = false, need_merge = false;
  data_file.read(block, node.children[i]);
  std::tie(deleted, need_merge) = block.delete_key(key, value);
  if (!deleted) {
    // not found in the current block, find afterwards
    while (block.next_block_id != -1) {
      data_file.read(block, block.next_block_id);
      if (block.key_count == 0 || block.data[0].first > key) {
        break;
      }
      std::tie(deleted, need_merge) = block.delete_key(key, value);
      if (deleted) {
        index = block.parent_id;
        node_file.read(node, block.parent_id);
        i = 0;
        while (i < node.key_count && node.children[i] != block.block_id) {
          i++;
        }
        break;
      }
    }
  }

  if (deleted && need_merge && i != node.key_count - 1 && !node.is_root) {
    BlockType next_block;
    data_file.read(next_block, block.next_block_id);
    if (next_block.key_count > BLOCK_SIZE / 2) {
      // borrow elements
      Key new_key = block.borrow(next_block);
      node.keys[i] = new_key;
      node_file.update(node, node.node_id);
      data_file.update(block, block.block_id);
      data_file.update(next_block, next_block.block_id);
    } else {
      block.merge_block(next_block);
      data_file.remove(next_block.block_id);
      for (int j = i; j < node.key_count - 1; ++j) {
        node.keys[j] = node.keys[j + 1];
      }
      for (int j = i + 1; j < node.key_count - 1; ++j) {
        node.children[j] = node.children[j + 1];
      }
      node.key_count--;
      node.children[node.key_count] = -1;
      data_file.update(block, block.block_id);
      if (node.key_count <= NODE_SIZE / 3) {
        node_file.update(node, node.node_id);
        merge_nodes(index);
      } else {
        node_file.update(node, node.node_id);
      }
    }
  } else if (deleted) {
    // write the block back
    data_file.update(block, block.block_id);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::merge_nodes(int index) {
  // std::cout << "Merge Nodes" << std::endl; // debug
  NodeType node;
  node_file.read(node, index);
  if (node.parent_id == -1) {
    return;
  }
  NodeType parent_node;
  node_file.read(parent_node, node.parent_id);
  int j = 0;
  while (parent_node.children[j] != index && j < parent_node.key_count) {
    j++;
  }

  NodeType left, right;
  // Check left sibling
  if (j != 0) {
    node_file.read(left, parent_node.children[j - 1]);
    if (left.key_count >= NODE_SIZE / 2) {
      // adopt from left
      for (int k = node.key_count; k > 0; k--) {
        node.keys[k] = node.keys[k - 1];
        node.children[k] = node.children[k - 1];
      }
      node.keys[0] = left.keys[left.key_count - 1];
      node.children[0] = left.children[left.key_count - 1];

      if (!node.is_leaf) {
        NodeType child_node;
        node_file.read(child_node, node.children[0]);
        child_node.parent_id = node.node_id;
        node_file.update(child_node, node.children[0]);
      } else {
        BlockType child_block;
        data_file.read(child_block, node.children[0]);
        child_block.parent_id = node.node_id;
        data_file.update(child_block, node.children[0]);
      }

      node.key_count++;
      left.key_count--;
      parent_node.keys[j - 1] = left.keys[left.key_count - 1];

      node_file.update(node, node.node_id);
      node_file.update(left, left.node_id);
      node_file.update(parent_node, parent_node.node_id);
      return;
    }
  }
  if (j <= parent_node.key_count - 2) {
    node_file.read(right, parent_node.children[j + 1]);
    if (right.key_count >= NODE_SIZE / 2) {
      // adopt from right
      node.keys[node.key_count] = right.keys[0];
      node.children[node.key_count] = right.children[0];
      for (int k = 0; k < right.key_count; k++) {
        right.keys[k] = right.keys[k + 1];
        right.children[k] = right.children[k + 1];
      }

      if (!node.is_leaf) {
        NodeType child_node;
        node_file.read(child_node, node.children[node.key_count]);
        child_node.parent_id = node.node_id;
        node_file.update(child_node, node.children[node.key_count]);
      } else {
        BlockType child_block;
        data_file.read(child_block, node.children[node.key_count]);
        child_block.parent_id = node.node_id;
        data_file.update(child_block, node.children[node.key_count]);
      }

      node.key_count++;
      right.key_count--;
      parent_node.keys[j] = node.keys[node.key_count - 1];

      node_file.update(node, node.node_id);
      node_file.update(right, right.node_id);
      node_file.update(parent_node, parent_node.node_id);
      return;
    }
  }
  if (j != 0) {
    // merge left
    for (int k = 0; k < node.key_count; k++) {
      left.keys[k + left.key_count] = node.keys[k];
      left.children[k + left.key_count] = node.children[k];
      if (!node.is_leaf) {
        NodeType child;
        node_file.read(child, node.children[k]);
        child.parent_id = left.node_id;
        node_file.update(child, child.node_id);
      } else {
        BlockType child_block;
        data_file.read(child_block, node.children[k]);
        child_block.parent_id = left.node_id;
        data_file.update(child_block, child_block.block_id);
      }
    }

    left.key_count += node.key_count;
    parent_node.keys[j - 1] = parent_node.keys[j];
    node_file.update(parent_node, parent_node.node_id);
    node_file.update(left, left.node_id);
    node_file.remove(node.node_id);

    delete_from_internal_node(parent_node.node_id, j);
    return;
  }
  if (j <= parent_node.key_count - 2) {
    // merge right
    for (int k = 0; k < right.key_count; k++) {
      node.keys[k + node.key_count] = right.keys[k];
      node.children[k + node.key_count] = right.children[k];
      if (!right.is_leaf) {
        NodeType child;
        node_file.read(child, right.children[k]);
        child.parent_id = node.node_id;
        node_file.update(child, child.node_id);
      } else {
        BlockType child_block;
        data_file.read(child_block, right.children[k]);
        child_block.parent_id = node.node_id;
        data_file.update(child_block, child_block.block_id);
      }
    }

    node.key_count += right.key_count;
    parent_node.children[j + 1] =
        node.node_id; // doing this line is because I only need to delete
                      // keys[j] and children[j] afterwards
    node_file.update(node, node.node_id);
    node_file.update(parent_node, parent_node.node_id);
    node_file.remove(right.node_id);

    delete_from_internal_node(parent_node.node_id, j);
    return;
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::split_node(int index) {
  // std::cout << "Split Node" << std::endl; // debug
  NodeType node;
  node_file.read(node, index);
  if (node.is_root) {
    NodeType child_1, child_2;
    for (int j = 0; j < node.key_count / 2; j++) {
      child_1.keys[j] = node.keys[j];
      child_1.children[j] = node.children[j];
      child_1.key_count++;
    }

    for (int j = node.key_count / 2; j < node.key_count; j++) {
      child_2.keys[j - node.key_count / 2] = node.keys[j];
      child_2.children[j - node.key_count / 2] = node.children[j];
      child_2.key_count++;
    }

    child_1.node_id = node_file.write(child_1);
    child_2.node_id = node_file.write(child_2);

    node.key_count = 2;
    node.children[0] = child_1.node_id;
    node.children[1] = child_2.node_id;
    node.keys[0] = child_1.keys[child_1.key_count - 1];
    node.keys[1] = child_2.keys[child_2.key_count - 1];

    child_1.is_leaf = child_2.is_leaf = node.is_leaf;
    node.is_leaf = false;

    child_1.parent_id = node.node_id;
    child_2.parent_id = node.node_id;

    if (!child_1.is_leaf) {
      for (int j = 0; j < child_1.key_count; j++) {
        NodeType child_node;
        node_file.read(child_node, child_1.children[j]);
        child_node.parent_id = child_1.node_id;
        node_file.update(child_node, child_1.children[j]);
      }
    } else {
      for (int j = 0; j < child_1.key_count; j++) {
        BlockType child_block;
        data_file.read(child_block, child_1.children[j]);
        child_block.parent_id = child_1.node_id;
        data_file.update(child_block, child_1.children[j]);
      }
    }

    if (!child_2.is_leaf) {
      for (int j = 0; j < child_2.key_count; j++) {
        NodeType child_node;
        node_file.read(child_node, child_2.children[j]);
        child_node.parent_id = child_2.node_id;
        node_file.update(child_node, child_2.children[j]);
      }
    } else {
      for (int j = 0; j < child_2.key_count; j++) {
        BlockType child_block;
        data_file.read(child_block, child_2.children[j]);
        child_block.parent_id = child_2.node_id;
        data_file.update(child_block, child_2.children[j]);
      }
    }

    node_file.update(child_1, child_1.node_id);
    node_file.update(child_2, child_2.node_id);
    node_file.update(node, node.node_id);
  } else {
    NodeType new_node;
    int mid = node.key_count / 2;
    new_node.key_count = node.key_count - mid;
    for (int j = 0; j < new_node.key_count; j++) {
      new_node.children[j] = node.children[j + mid];
      new_node.keys[j] = node.keys[j + mid];
    }
    new_node.is_leaf = node.is_leaf;
    new_node.parent_id = node.parent_id;
    node.key_count = mid;
    new_node.node_id = node_file.write(new_node);
    node_file.update(node, node.node_id);
    node_file.update(new_node, new_node.node_id);

    if (!new_node.is_leaf) {
      for (int i = 0; i < new_node.key_count; i++) {
        NodeType child;
        node_file.read(child, new_node.children[i]);
        child.parent_id = new_node.node_id;
        node_file.update(child, child.node_id);
      }
    } else {
      for (int i = 0; i < new_node.key_count; i++) {
        BlockType child;
        data_file.read(child, new_node.children[i]);
        child.parent_id = new_node.node_id;
        data_file.update(child, child.block_id);
      }
    }

    NodeType parent_node;
    node_file.read(parent_node, node.parent_id);

    int j = 0;
    while (parent_node.children[j] != index && j < parent_node.key_count) {
      j++;
    }
    Key original_key = parent_node.keys[j];
    parent_node.keys[j] = node.keys[node.key_count - 1];
    node_file.update(parent_node, parent_node.node_id);

    insert_into_internal_node(node.parent_id, original_key, new_node.node_id,
                              j + 1);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::insert_into_internal_node(
    int index, Key key, int child_index, int pos) {
  // std::cout << "Insert into Internal Node" << std::endl; // debug
  NodeType node;
  node_file.read(node, index);

  for (int j = node.key_count; j > pos; j--) {
    node.keys[j] = node.keys[j - 1];
    node.children[j] = node.children[j - 1];
  }
  node.keys[pos] = key;
  node.children[pos] = child_index;
  node.key_count++;

  node_file.update(node, node.node_id);

  if (node.key_count >= NODE_SIZE) {
    split_node(index);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::delete_from_internal_node(
    int index, int pos) {
  NodeType node;
  node_file.read(node, index);
  if (node.is_root) {
    if (node.key_count >= 3) {
      for (int j = pos; j < node.key_count - 1; j++) {
        node.keys[j] = node.keys[j + 1];
        node.children[j] = node.children[j + 1];
      }
      node.key_count--;
      if (node.keys[node.key_count - 1] != MAX_KEY)
        node.keys[node.key_count - 1] = MAX_KEY;
      node_file.update(node, node.node_id);
    } else if (node.key_count == 2) {
      int preserved_child = node.children[1 - pos]; // maybe have problems?
      NodeType child_node;
      node_file.read(child_node, preserved_child);
      child_node.is_root = true;
      child_node.parent_id = -1;
      node_file.remove(node.node_id);
      root_node = child_node;
      root_index = child_node.node_id;
      child_node.keys[child_node.key_count - 1] = MAX_KEY;
      node_file.update(child_node, child_node.node_id);

      node_file.write_info(root_index, 1);
    }
  } else {
    for (int j = pos; j < node.key_count - 1; j++) {
      node.keys[j] = node.keys[j + 1];
      node.children[j] = node.children[j + 1];
    }
    node.key_count--;
    if (node.keys[node.key_count] == MAX_KEY)
      node.keys[node.key_count - 1] = MAX_KEY;
    if (node.key_count <= NODE_SIZE / 3) {
      node_file.update(node, node.node_id);
      merge_nodes(index);
    } else {
      node_file.update(node, node.node_id);
    }
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
void BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::FileInit() {
  if (node_file.isEmpty()) {
    root_node.parent_id = -1;
    root_node.is_leaf = true;
    root_node.is_root = true;
    root_node.key_count = 1;
    for (int i = 0; i <= NODE_SIZE; i++) {
      root_node.children[i] = -1;
    }
    BlockType init_block;
    init_block.block_id = data_file.write(init_block);

    root_node.keys[0] = MAX_KEY;
    root_node.children[0] = init_block.block_id;
    data_file.update(init_block, init_block.block_id);

    root_index = node_file.write(root_node);
    root_node.node_id = root_index;
    node_file.update(root_node, root_index);

    node_file.write_info(root_index, 1);
  } else {
    node_file.get_info(root_index, 1);

    node_file.read(root_node, root_index);
  }
}

template <typename Key, typename Value, size_t NODE_SIZE, size_t BLOCK_SIZE>
sjtu::vector<Value> BPTStorage<Key, Value, NODE_SIZE, BLOCK_SIZE>::sort_result(
    sjtu::vector<Value> &vec) {
  if (vec.size() <= 1)
    return vec;

  auto partition = [&vec](int low, int high) {
    Value pivot = vec[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
      if (vec[j] <= pivot) {
        i++;
        Value temp = vec[i];
        vec[i] = vec[j];
        vec[j] = temp;
      }
    }

    Value temp = vec[i + 1];
    vec[i + 1] = vec[high];
    vec[high] = temp;

    return i + 1;
  };

  std::function<void(int, int)> quicksort = [&](int low, int high) {
    if (low < high) {
      int pi = partition(low, high);
      quicksort(low, pi - 1);
      quicksort(pi + 1, high);
    }
  };

  quicksort(0, vec.size() - 1);
  return vec;
}

#endif // BPT_STORAGE_HPP