#include <iomanip>
#include <iostream>
#include <map>
#include <set> // To keep track of visited blocks/nodes
#include <string>
#include <vector>

// Include your header files (make sure paths are correct)
#include "src/include/stl/string64.hpp"
#include "src/include/storage/bptNode.hpp" // Contains BPTNode and DataBlock definitions
#include "src/include/storage/fileOperation.hpp" // Contains FileOperation definition

// --- Configuration ---
// !!! IMPORTANT: Set these to match your BPTStorage template parameters !!!
using KeyType = sjtu::string64;
using ValueType = int;
const size_t NODE_SIZE_CONF = 7;
const size_t BLOCK_SIZE_CONF = 57;
// --- End Configuration ---

// Define the specific types based on configuration
using NodeType = BPTNode<KeyType, NODE_SIZE_CONF>;
using BlockType = DataBlock<KeyType, ValueType, BLOCK_SIZE_CONF>;
using NodeFileOp = FileOperation<NodeType>;
using DataFileOp = FileOperation<BlockType>;

// Helper function to print keys/children/data
template <typename T>
void print_array(const std::string &label, const T *arr, int count) {
  std::cout << label << "[";
  for (int i = 0; i < count; ++i) {
    std::cout << arr[i] << (i == count - 1 ? "" : ", ");
  }
  std::cout << "]";
}

// Recursive function to print the B+ Tree structure
void print_tree_recursive(
    int node_id, const std::map<int, NodeType> &nodes,
    const std::map<int, BlockType> &blocks,
    NodeFileOp &node_file, // Pass FileOp to read if needed
    std::set<int> &visited_nodes, std::string indent = "") {

  if (node_id <= 0 || nodes.find(node_id) == nodes.end()) {
    std::cout << indent << "[Invalid Node ID: " << node_id << "]" << std::endl;
    return;
  }
  if (visited_nodes.count(node_id)) {
    std::cout << indent << "[Node " << node_id << " (already visited)]"
              << std::endl;
    return; // Avoid infinite loops in case of corrupted structure
  }
  visited_nodes.insert(node_id);

  const NodeType &node = nodes.at(node_id);

  std::cout << indent << (node.is_leaf ? "[LEAF NODE]" : "[INTERNAL NODE]")
            << " ID: " << node.node_id << ", Parent: " << node.parent_id
            << ", Next: " << node.next_node_id << ", KCount: " << node.key_count
            << (node.is_root ? " (ROOT)" : "") << std::endl;

  std::cout << indent << "  ";
  print_array("Keys: ", node.keys, node.key_count);
  std::cout << std::endl;

  std::cout << indent << "  ";
  if (node.is_leaf) {
    print_array("DataPtrs: ", node.children,
                node.key_count); // Leaf nodes point to data blocks
    std::cout << std::endl;
    // Optional: Print data block content directly here if desired
    // for (int i = 0; i < node.key_count; ++i) {
    //     int block_id = node.children[i];
    //     if (blocks.count(block_id)) {
    //         // Print block summary or key range
    //     }
    // }
  } else {
    print_array("Children: ", node.children,
                node.key_count + 1); // Internal nodes point to child nodes
    std::cout << std::endl;
    // Recursively print children
    std::string child_indent = indent + "    |";
    for (int i = 0; i <= node.key_count; ++i) {
      std::cout << indent << "    +" << std::endl; // Visual connector
      print_tree_recursive(node.children[i], nodes, blocks, node_file,
                           visited_nodes, child_indent);
    }
  }
  std::cout << indent << "--------------------" << std::endl;
}

// Function to find the first data block ID (leftmost leaf's first child)
int find_first_data_block_id(int root_node_id,
                             const std::map<int, NodeType> &nodes) {
  if (nodes.empty() || root_node_id <= 0 ||
      nodes.find(root_node_id) == nodes.end()) {
    return -1;
  }

  int current_node_id = root_node_id;
  while (true) {
    const NodeType &node = nodes.at(current_node_id);
    if (node.is_leaf) {
      if (node.key_count > 0) {
        return node.children[0]; // First child pointer in the leftmost leaf
      } else {
        return -1; // Empty leaf
      }
    } else {
      if (node.key_count < 0)
        return -1; // Should not happen in valid tree
      // Go to the leftmost child
      if (nodes.find(node.children[0]) == nodes.end())
        return -1; // Child missing
      current_node_id = node.children[0];
    }
    if (current_node_id <= 0)
      return -1; // Invalid child pointer
  }
}

// Function to print the linked list of data blocks
void print_data_blocks(int first_block_id,
                       const std::map<int, BlockType> &blocks,
                       DataFileOp &data_file, // Pass FileOp to read if needed
                       std::set<int> &visited_blocks) {

  std::cout << "\n--- Data Blocks Linked List ---" << std::endl;
  if (first_block_id <= 0) {
    std::cout << "[No data blocks found or tree is empty]" << std::endl;
    return;
  }

  int current_block_id = first_block_id;
  while (current_block_id > 0 && blocks.count(current_block_id)) {
    if (visited_blocks.count(current_block_id)) {
      std::cout << "[Block " << current_block_id
                << " (Cycle detected, already visited)]" << std::endl;
      break; // Avoid infinite loops
    }
    visited_blocks.insert(current_block_id);

    const BlockType &block = blocks.at(current_block_id);
    std::cout << "[BLOCK] ID: " << block.block_id
              << ", Next: " << block.next_block_id
              << ", KCount: " << block.key_count << std::endl;
    std::cout << "  Data: {";
    for (int i = 0; i < block.key_count; ++i) {
      // Assuming KeyType and ValueType can be streamed to cout
      std::cout << "(" << block.data[i].first << ": " << block.data[i].second
                << ")" << (i == block.key_count - 1 ? "" : ", ");
    }
    std::cout << "}" << std::endl;
    std::cout << "---------------------------" << std::endl;

    current_block_id = block.next_block_id;
    if (current_block_id > 0 && blocks.find(current_block_id) == blocks.end()) {
      std::cout << "[Error: Next Block ID " << current_block_id
                << " not found in file!]" << std::endl;
      break;
    }
  }
  if (current_block_id > 0 && !blocks.count(current_block_id)) {
    std::cout << "[Dangling Next Pointer: " << current_block_id << "]"
              << std::endl;
  }
  std::cout << "--- End Data Blocks ---" << std::endl;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <node_file.dat> <data_file.dat>"
              << std::endl;
    return 1;
  }

  std::string node_filename = argv[1];
  std::string data_filename = argv[2];

  NodeFileOp node_file(node_filename);
  DataFileOp data_file(data_filename);

  int root_index = -1;
  int node_count = 0; // Assuming info[0] is root_index, info[1] is node_count
  // Adjust indices if your FileOperation stores info differently
  node_file.get_info(root_index, 1);
  node_file.get_info(node_count, 2); // Read node count if stored

  // You might need similar metadata reading for the data file if you store it
  // int data_block_count = 0;
  // data_file.get_info(data_block_count, 1); // Example

  std::cout << "--- B+ Tree Visualizer ---" << std::endl;
  std::cout << "Node File: " << node_filename << std::endl;
  std::cout << "Data File: " << data_filename << std::endl;
  std::cout << "Root Node Index: " << root_index << std::endl;
  std::cout << "Node Count (from metadata): " << node_count << std::endl;
  // std::cout << "Data Block Count (from metadata): " << data_block_count <<
  // std::endl;
  std::cout << "--------------------------" << std::endl;

  if (root_index <= 0) {
    if (node_file.isEmpty()) {
      std::cout << "Tree is empty (files contain only metadata)." << std::endl;
    } else {
      std::cout << "Tree is empty or root index is invalid." << std::endl;
    }
    return 0;
  }

  // Read all nodes and blocks into memory
  std::map<int, NodeType> all_nodes;
  std::map<int, BlockType> all_blocks;

  // Need to figure out how many nodes/blocks to read.
  // Option 1: Use node_count/data_block_count if accurate.
  // Option 2: Read sequentially until EOF (less robust if file has
  // gaps/garbage). Option 3: Traverse from root (only reads reachable
  // nodes/blocks). Let's try reading sequentially based on typical
  // FileOperation usage where new items are appended. We need to know the
  // starting offset.
  int node_info_bytes = 2 * sizeof(int); // Assuming 2 info integers
  int block_info_bytes =
      2 *
      sizeof(int); // Assuming 2 info integers for data file too? Adjust if not.

  std::ifstream node_ifs(node_filename, std::ios::binary | std::ios::ate);
  if (!node_ifs) {
    std::cerr << "Error opening node file: " << node_filename << std::endl;
    return 1;
  }
  long long node_file_size = node_ifs.tellg();
  std::cout << "Node file size: " << node_file_size << " bytes"
            << std::endl;
  node_ifs.close();

  int current_node_offset = node_info_bytes;
  while (current_node_offset < node_file_size) {
    NodeType node;
    // Assuming read() works correctly with the offset as ID
    node_file.read(node, current_node_offset);
    // Simple sanity check - node_id should match offset if write() returns it
    if (node.node_id == current_node_offset) { 
        all_nodes[current_node_offset] = node;
    } else {
      std::cerr << "Warning: Node ID " << node.node_id << " mismatch at offset "
                << current_node_offset << ". Stopping node read." << std::endl;
      // break; // Or continue cautiously
    }
    current_node_offset += sizeof(NodeType);
  }
  std::cout << "Read " << all_nodes.size() << " nodes from file." << std::endl;

  std::ifstream data_ifs(data_filename, std::ios::binary | std::ios::ate);
  if (!data_ifs) {
    std::cerr << "Error opening data file: " << data_filename << std::endl;
    return 1;
  }
  long long data_file_size = data_ifs.tellg();
  data_ifs.close();

  int current_block_offset = block_info_bytes;
  while (current_block_offset < data_file_size) {
    BlockType block;
    data_file.read(block, current_block_offset);
    if (block.block_id == current_block_offset || block.block_id == -1 ||
        block.block_id == 0) { // Allow -1 or 0 if used as sentinel/padding
      if (block.block_id > 0)  // Only store valid blocks
        all_blocks[current_block_offset] = block;
    } else {
      std::cerr << "Warning: Block ID " << block.block_id
                << " mismatch at offset " << current_block_offset
                << ". Stopping block read." << std::endl;
      // break;
    }
    current_block_offset += sizeof(BlockType);
  }
  std::cout << "Read " << all_blocks.size() << " data blocks from file."
            << std::endl;
  std::cout << "--------------------------" << std::endl;

  // Print the tree structure
  std::cout << "\n--- B+ Tree Structure ---" << std::endl;
  std::set<int> visited_nodes;
  print_tree_recursive(root_index, all_nodes, all_blocks, node_file,
                       visited_nodes, "");
  std::cout << "--- End Tree Structure ---" << std::endl;

  // Find the first data block and print the list
  int first_block_id = find_first_data_block_id(root_index, all_nodes);
  std::set<int> visited_blocks;
  print_data_blocks(first_block_id, all_blocks, data_file, visited_blocks);

  return 0;
}