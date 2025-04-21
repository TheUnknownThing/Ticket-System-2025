#ifndef BPT_STORAGE_HPP
#define BPT_STORAGE_HPP

#include "fileOperation.hpp"
#include "bptNode.hpp"
#include <string>

template <typename Key, typename Value, size_t NODE_SIZE = 4, size_t BLOCK_SIZE = 4>
class BPTStorage {
private:
    FileOperation<BPTNode<Key, NODE_SIZE>> node_file;
    FileOperation<DataBlock<Key, Value, BLOCK_SIZE>> data_file;
    
    std::string node_file_name;
    std::string data_file_name;
    
    int root_index;
    size_t node_count;
    size_t data_block_count;
    
    void write_node(int index, BPTNode<Key, NODE_SIZE> &node);
    void read_node(int index, BPTNode<Key, NODE_SIZE> &node);
    
    void write_data_block(int index, DataBlock<Key, Value, BLOCK_SIZE> &block);
    void read_data_block(int index, DataBlock<Key, Value, BLOCK_SIZE> &block);
    
    int find_leaf_node(Key key);
    
    int create_node(bool is_leaf);
    
    int create_data_block();
    
    void insert_into_internal_node(int index, Key key, int child_index);
    
    bool insert_into_leaf_node(int index, Key key, Value value);
    
    void delete_from_internal_node(int index, Key key);
    
    bool delete_from_leaf_node(int index, Key key, Value value);
    
    Value* find_in_leaf(int index, Key key);
    
    void split_node(int node_index);
    
    void merge_nodes(int left_index, int right_index);

public:
    BPTStorage(const std::string &file_prefix);
    ~BPTStorage();

    bool insert(Key key, Value value);
    
    bool remove(Key key, Value value);
    
    Value* find(Key key);
};

#endif // BPT_STORAGE_HPP