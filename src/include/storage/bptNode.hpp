#ifndef BPT_NODE_HPP
#define BPT_NODE_HPP

#include <string>

template <typename Key, size_t NODE_SIZE = 4>
struct BPTNode{
public:
    int node_id; // BPT node id = index in file
    int parent_id;
    bool is_leaf;
    int key_count = 0;
    Key keys[NODE_SIZE];
    int children[NODE_SIZE + 1]; // internal page: children, leaf page: block id
};

template <typename Key, typename Value, size_t BLOCK_SIZE = 4>
struct DataBlock {
public:
    std::pair<Key, Value> data[BLOCK_SIZE];
    int key_count;
    int block_id;
    int next_block_id;
    
    DataBlock() {
        key_count = 0;
        block_id = -1;
        next_block_id = -1;
    }
};


#endif // BPT_NODE_HPP