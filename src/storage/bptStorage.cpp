#include "storage/bptStorage.hpp"

template <typename Key, typename Value>
BPTStorage<Key, Value>::~BPTStorage() {
    // TODO
}

template <typename Key, typename Value>
BPTStorage<Key, Value>::BPTStorage(const std::string &file_name) : file_name(file_name), root_index(-1), root_node_index(-1), node_count(0), leaf_count(0) {
    // TODO
}

template <typename Key, typename Value>
void BPTStorage<Key, Value>::write_node(int index, BPTNode &node) {
    // TODO
}

template <typename Key, typename Value>
void BPTStorage<Key, Value>::read_node(int index, BPTNode &node) {
    // TODO
}

template <typename Key, typename Value>
void BPTStorage<Key, Value>::insert_into_node(int index, Key key, Value value) {
    // TODO
}

template <typename Key, typename Value>
void BPTStorage<Key, Value>::delete_from_node(int index, Key key, Value value) {
    // TODO
}

template <typename Key, typename Value>
void BPTStorage<Key, Value>::find_in_node(int index, Key key, Value* values) {
    // TODO
}

template <typename Key, typename Value>
bool BPTStorage<Key, Value>::insert(Key key, Value value) {
    // TODO
    return true;
}

template <typename Key, typename Value>
bool BPTStorage<Key, Value>::remove(Key key, Value value) {
    // TODO
    return true;
}

template <typename Key, typename Value>
Value* BPTStorage<Key, Value>::find(Key key) {
    // TODO
    return nullptr;
}


