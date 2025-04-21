#ifndef BPT_STORAGE_HPP
#define BPT_STORAGE_HPP

#include "fileOperation.hpp"
#include "bptNode.hpp"

template <typename Key, typename Value>
class BPTStorage {
private:
    FileOperation<BPTNode> file_op;
    std::string file_name;
    int root_index;
    int root_node_index;
    int node_count;
    int leaf_count;
    
    void write_node(int index, BPTNode &node) ;

    void read_node(int index, BPTNode &node) ;

    void insert_into_node(int index, Key key, Value value) ;

    void delete_from_node(int index, Key key, Value value) ;

    void find_in_node(int index, Key key, Value* values) ;

public:
    BPTStorage(const std::string &file_name);

    ~BPTStorage() ;

    bool insert(Key key, Value value);

    bool remove(Key key, Value value);

    Value* find(Key key);
};

#endif // BPT_STORAGE_HPP