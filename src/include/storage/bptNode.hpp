#ifndef BPT_NODE_HPP
#define BPT_NODE_HPP

#include <string>

template <typename Key, size_t NODE_SIZE = 4> struct BPTNode {
public:
  int node_id; // BPT node id = index in file
  int parent_id;

  bool is_leaf;
  bool is_root;

  int key_count;

  // Why +1? Because I would insert first, then split the node.
  Key keys[NODE_SIZE + 1];
  int children[NODE_SIZE + 1]; // internal page: children, leaf page: block id

  BPTNode() : is_leaf(false), is_root(false), key_count(0) {}
};

template <typename Key, typename Value, size_t BLOCK_SIZE = 4>
struct DataBlock {
public:
  // Why +1? Similar to above.
  std::pair<Key, Value> data[BLOCK_SIZE + 1];
  int key_count;
  int block_id;
  int next_block_id;

  DataBlock() : key_count(0), block_id(-1), next_block_id(-1) {}

  /*
   * @brief delete a key from the block.
   * @return true if the key is deleted, and second bool is true if the block
   * needs to be merged.
   */
  std::pair<bool, bool> delete_key(Key key, Value value) {
    for (int i = 0; i < key_count; i++) {
      if (data[i].first == key && data[i].second == value) {
        for (int j = i; j < key_count - 1; j++) {
          data[j] = data[j + 1];
        }
        key_count--;
        return {true, key_count < BLOCK_SIZE / 3};
      }
    }
    return {false, false};
  }

  /*
   * @brief Merge two blocks
   * @return true if the merge is successful, false if the merge fails.
   */
  bool merge_block(DataBlock<Key, Value, BLOCK_SIZE> &block) {
    if (key_count + block.key_count >= BLOCK_SIZE) {
      return false;
    }
    for (int i = 0; i < block.key_count; i++) {
      data[key_count + i] = block.data[i];
    }
    key_count += block.key_count;
    next_block_id = block.next_block_id;
    return true;
  }

  /*
   * @brief Borrow elements from the next block
   */
  Key borrow(DataBlock<Key, Value, BLOCK_SIZE> &block) {
    data[key_count] = block.data[0];
    key_count++;
    block.key_count--;
    for (int j = 0; j < block.key_count - 1; j++) {
      block.data[j] = block.data[j + 1];
    }
    return data[key_count - 1];
  }

  /*
   * @brief insert a key-value pair into the block.
   * @return true if need to split the block, and return the new block.
   * @note Still Need to update the block id and next block id after split.
   */
  std::pair<bool, DataBlock<Key, Value, BLOCK_SIZE>> insert_key(Key key,
                                                                Value value) {
    int i = 0;
    while (i < key_count && data[i].first < key) {
      i++;
    }
    for (int j = key_count; j > i; j--) {
      data[j] = data[j - 1];
    }
    data[i] = std::make_pair(key, value);
    key_count++;

    if (key_count <= BLOCK_SIZE) {
      // no need to split
      return {false, *this};
    }

    // need to split the block
    DataBlock<Key, Value, BLOCK_SIZE> new_block;
    int mid = (BLOCK_SIZE + 1) / 2;
    new_block.key_count = key_count - mid;

    for (int j = 0; j < new_block.key_count; j++) {
      new_block.data[j] = data[mid + j];
    }

    new_block.next_block_id = next_block_id;
    next_block_id = new_block.block_id;
    key_count = mid;

    return {true, new_block};
  }
};

#endif // BPT_NODE_HPP