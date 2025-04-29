/*  Simple visualiser for the B+–tree file produced by your code.
 *  By default it assumes    Key = int    NODE_SIZE = 4
 *  If you used different template parameters just change the two
 *  numbers below or compile with e.g.
 *
 *     g++ -std=c++17 -DNODESZ=8 -DKEY=int bpt_visualizer.cpp -o viz
 *
 *  Compile:
 *     g++ -std=c++17 bpt_visualizer.cpp -o bpt_viz
 *  Run:
 *     ./bpt_viz node_file.bin
 */
 #define KEY         int
 #define NODESZ      4        // == NODE_SIZE used in your tree
 
 #include <fstream>
 #include <iomanip>
 #include <iostream>
 #include <string>
 #include <vector>
 
#include "src/include/storage/bptNode.hpp"
#include "src/include/storage/fileOperation.hpp"
 
 /* ──────────────────────────────────────────────── helpers ────── */
 template<typename T>
 std::string to_str(const T &k) {                  // generic
     return std::to_string(k);
 }
 template<>
 std::string to_str<std::string>(const std::string &k) { return k; }
 
 template<typename Key, std::size_t N>
 struct Record {
     BPTNode<Key, N> node;
     int              file_offset;                // its byte position in file
 };
 
 /* ─────────────────────────────────────────────── visualiser ──── */
 template<typename Key = KEY, std::size_t N = NODESZ>
 class Visualiser {
     using Node  = BPTNode<Key, N>;
     using File  = FileOperation<Node>;
 
     File              node_file;
     std::string       file_name;
     int               root_offset {};
     int               node_count {};
     std::vector<Record<Key, N>> nodes;
 
 public:
     explicit Visualiser(std::string fn) : file_name(std::move(fn)) {
         node_file.initialise(file_name);
         node_file.get_info(root_offset, 1);
         node_file.get_info(node_count, 2);
         read_all_nodes();
     }
 
     /* ------- read every node into RAM ---------------------------------- */
     void read_all_nodes() {
         constexpr int header = 2 * sizeof(int);
         nodes.reserve(node_count);
 
         for (int i = 0; i < node_count; ++i) {
             int offset = header + i * sizeof(Node);
 
             Node n;
             node_file.read(n, offset);
             nodes.push_back({n, offset});
         }
     }
 
     /* ------- text dump to stdout --------------------------------------- */
     void dump_stdout() const {
         using std::setw;
 
         std::cout << "root @ "  << root_offset  << "\n"
                   << "nodes: "   << node_count  << "\n\n";
 
         for (auto const &rec : nodes) {
             auto const &n = rec.node;
 
             std::cout << "┌───────────────────────────────────────────────\n";
             std::cout << "│ Node @" << rec.file_offset
                       << (n.is_root ? " (root)" : "")
                       << (n.is_leaf ? " (leaf)" : "") << '\n';
             std::cout << "│ parent = "      << n.parent_id
                       << "   next = "        << n.next_node_id << '\n';
             std::cout << "│ keys   : ";
             for (int i = 0; i < n.key_count; ++i)
                 std::cout << to_str(n.keys[i]) << ' ';
             std::cout << '\n';
 
             if (n.is_leaf) {
                 std::cout << "│ blocks : ";
                 for (int i = 0; i < n.key_count; ++i)
                     std::cout << n.children[i] << ' ';
                 std::cout << '\n';
             } else {
                 std::cout << "│ child  : ";
                 for (int i = 0; i <= n.key_count; ++i)
                     std::cout << n.children[i] << ' ';
                 std::cout << '\n';
             }
             std::cout << "└───────────────────────────────────────────────\n\n";
         }
     }
 
     /* ------- emit a GraphViz .dot file --------------------------------- */
     void dump_dot(const std::string &out = "bpt.dot") const {
         std::ofstream dot(out);
         if (!dot) { std::cerr << "Cannot write " << out << '\n'; return; }
 
         dot << "digraph BPT {\n"
             << "  node [shape=record, height=.1];\n";
 
         /* nodes --------------------------------------------------------- */
         for (auto const &rec : nodes) {
             auto const &n = rec.node;
 
             dot << "  \"" << rec.file_offset << "\" [label=\"{";
             if (n.is_leaf) {
                 dot << "LEAF|";
                 for (int i = 0; i < n.key_count; ++i) {
                     dot << to_str(n.keys[i]);
                     if (i != n.key_count - 1) dot << " | ";
                 }
             } else {                          // internal
                 for (int i = 0; i < n.key_count; ++i) {
                     dot << "<c" << i << "> | "
                         << to_str(n.keys[i]) << " | ";
                 }
                 dot << "<c" << n.key_count << ">";   // last pointer
             }
             dot << "}\"";
             if (n.is_root) dot << ", style=bold";
             dot << "];\n";
         }
 
         /* edges --------------------------------------------------------- */
         for (auto const &rec : nodes) {
             auto const &n = rec.node;
 
             if (!n.is_leaf) {                 // tree edges
                 for (int i = 0; i <= n.key_count; ++i) {
                     if (n.children[i] != -1)
                         dot << "  \"" << rec.file_offset
                             << "\":c" << i << " -> \""
                             << n.children[i] << "\";\n";
                 }
             }
             if (n.is_leaf && n.next_node_id != -1)   // leaf chain
                 dot << "  \"" << rec.file_offset << "\" -> \""
                     << n.next_node_id
                     << "\"  [style=dashed,color=blue];\n";
         }
 
         dot << "}\n";
         std::cout << "dot‑file written: " << out << '\n';
     }
 };
 
 /* ─────────────────────────────────────────────── main ────────── */
 int main(int argc, char *argv[]) {
     if (argc != 2) {
         std::cerr << "usage : " << argv[0] << " <node_file>\n";
         return 0;
     }
 
     try {
         Visualiser<> vis(argv[1]);
         vis.dump_stdout();         // plain text
         vis.dump_dot();            // GraphViz
     }
     catch (const std::exception &e) {
         std::cerr << "error : " << e.what() << '\n';
     }
 }