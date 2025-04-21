#ifndef BPT_NODE_HPP
#define BPT_NODE_HPP

#include <string>

struct BPTNode {
    static constexpr int ORDER = 4;
    bool is_leaf;
    int size;
    std::string keys[ORDER - 1];
    int values[ORDER - 1];
    int child_indices[ORDER];
    int next_leaf;
    BPTNode() : is_leaf(true), size(0), next_leaf(-1) {
        for (int i = 0; i < ORDER; i++) {
            child_indices[i] = -1;
        }
    }
};

#endif // BPT_NODE_HPP
