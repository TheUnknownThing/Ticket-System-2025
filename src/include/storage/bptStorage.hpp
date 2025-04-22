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
    [[maybe_unused]] size_t data_block_count;

    BPTNode<Key, NODE_SIZE> root_node;
    
    int find_leaf_node(Key key);
    
    int create_node(bool is_leaf);
    
    int create_data_block();
    
    /*
    * @brief Insert a key-value pair into a leaf node.
    * @return false if need to split the node, true otherwise.
    * @pa
    */
    bool insert_into_leaf_node(BPTNode<Key, NODE_SIZE>& node , Key key, Value value);

    /*
    * @brief Insert a key and child index into an internal node.
    * @return false if need to split the node, true otherwise.
    */
    bool insert_into_internal_node(BPTNode<Key, NODE_SIZE>& node, Key key, int child_index);
    
    bool delete_from_internal_node(BPTNode<Key, NODE_SIZE>& node, Key key);
    
    bool delete_from_leaf_node(BPTNode<Key, NODE_SIZE>& node, Key key, Value value);

    void merge_leaf_nodes(int left_index, int right_index);

    void merge_internal_nodes(int left_index, int right_index);

    void split_leaf_node(BPTNode<Key, NODE_SIZE>& node, Key key, Value value);

    void split_internal_node(BPTNode<Key, NODE_SIZE>& node, Key key, int child_index);
    
    void split_node(int node_index);
    
    void merge_nodes(int left_index, int right_index);

    void FileInit();

public:
    BPTStorage(const std::string &file_prefix);
    ~BPTStorage();

    bool insert(Key key, Value value);
    
    bool remove(Key key, Value value);
    
    Value* find(Key key);
};

#endif // BPT_STORAGE_HPP