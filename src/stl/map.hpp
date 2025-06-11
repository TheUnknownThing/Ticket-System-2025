#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include "utils/exceptions.hpp"
#include <cstddef>
#include <functional>

namespace sjtu {
template <class Key, class T, class Compare = std::less<Key>> class map {
public:
  /**
   * the internal type of data.
   * it should have a default constructor, a copy constructor.
   * You can use sjtu::map as value_type by typedef.
   */
  typedef std::pair<const Key, T> value_type;
  /**
   * see BidirectionalIterator at CppReference for help.
   *
   * if there is anything wrong throw invalid_iterator.
   *     like it = map.begin(); --it;
   *       or it = map.end(); ++end();
   */

private:
  struct Node {
    value_type data;
    size_t height;
    Node *left;
    Node *right;
    Node *parent;

    Node(const value_type &data, Node *parent = nullptr, Node *left = nullptr,
         Node *right = nullptr, size_t height = 1)
        : data(data), height(height), left(left), right(right), parent(parent) {
    }

    Node(const Node &other) = delete;
    Node &operator=(const Node &other) = delete;

    ~Node() {
      // DO NOTHING!
    }

    Node *balance() {
      update_height();
      int factor = get_balance();
      if (factor > 1) {
        if (left->get_balance() >= 0) {
          return LL();
        } else {
          return LR();
        }
      } else if (factor < -1) {
        if (right->get_balance() <= 0) {
          return RR();
        } else {
          return RL();
        }
      }
      return this;
    }

    void clear() {
      if (left) {
        left->clear();
        delete left;
        left = nullptr;
      }
      if (right) {
        right->clear();
        delete right;
        right = nullptr;
      }
    }

  private:
    int get_balance() {
      return (left ? left->height : 0) - (right ? right->height : 0);
    }

    void update_height() {
      height = 1 + std::max(left ? left->height : 0, right ? right->height : 0);
    }

    Node *RR() {
      Node *new_root = right;
      right = new_root->left;
      if (right)
        right->parent = this;
      new_root->left = this;
      new_root->parent = parent;
      parent = new_root;

      update_height();
      new_root->update_height();

      return new_root;
    }

    Node *LL() {
      Node *new_root = left;
      left = new_root->right;
      if (left)
        left->parent = this;
      new_root->right = this;
      new_root->parent = parent;
      parent = new_root;

      update_height();
      new_root->update_height();

      return new_root;
    }

    Node *LR() {
      left = left->RR();
      return LL();
    }

    Node *RL() {
      right = right->LL();
      return RR();
    }
  };

  Node *root;
  size_t node_count;
  Node *begin_node;
  Node *max_node;

  Node *insert(const value_type &value, Node *&node, Node *parent = nullptr) {
    if (node == nullptr) {
      node = new Node(value, parent);
      ++node_count;
      return node;
    }

    if (Compare()(value.first, node->data.first)) {
      node->left = insert(value, node->left, node);
    } else if (Compare()(node->data.first, value.first)) {
      node->right = insert(value, node->right, node);
    } else {
      return node;
    }

    Node *balanced = node->balance();
    if (balanced != node) {
      balanced->parent = parent;
      if (balanced->left)
        balanced->left->parent = balanced;
      if (balanced->right)
        balanced->right->parent = balanced;
    }
    return balanced;
  }

  Node *erase(const Key &key, Node *&node, Node *parent = nullptr) {
    if (node == nullptr)
      return nullptr;

    if (Compare()(key, node->data.first)) {
      node->left = erase(key, node->left, node);
    } else if (Compare()(node->data.first, key)) {
      node->right = erase(key, node->right, node);
    } else {
      if (!node->left) {
        Node *temp = node->right;
        if (temp)
          temp->parent = parent;
        delete node;
        --node_count;
        return temp;
      } else if (!node->right) {
        Node *temp = node->left;
        if (temp)
          temp->parent = parent;
        delete node;
        --node_count;
        return temp;
      } else {
        // two children
        Node *parent_succ = node;
        Node *succ = node->right;

        while (succ->left) {
          parent_succ = succ;
          succ = succ->left;
        }

        if (parent_succ != node) {
          parent_succ->left = succ->right;
          if (succ->right)
            succ->right->parent = parent_succ;

          succ->right = node->right;
          if (node->right)
            node->right->parent = succ;
        }

        succ->left = node->left;
        node->left->parent = succ;

        succ->parent = parent;

        delete node;
        --node_count;

        return succ->balance();
      }
    }

    Node *balanced = node->balance();
    if (balanced != node) {
      balanced->parent = parent;
      if (balanced->left)
        balanced->left->parent = balanced;
      if (balanced->right)
        balanced->right->parent = balanced;
    }
    return balanced;
  }

  T &find(const Key &key, Node *node) const {
    if (node == nullptr) {
      throw index_out_of_bound();
    }
    if (Compare()(key, node->data.first)) {
      return find(key, node->left);
    } else if (Compare()(node->data.first, key)) {
      return find(key, node->right);
    } else {
      return node->data.second;
    }
  }

  Node *find_node(const Key &key, Node *node) const {
    if (node == nullptr) {
      return nullptr;
    }
    if (Compare()(key, node->data.first)) {
      return find_node(key, node->left);
    } else if (Compare()(node->data.first, key)) {
      return find_node(key, node->right);
    } else {
      return node;
    }
  }

  Node *find_after(Node *node) const {
    if (node == nullptr)
      return nullptr;

    if (node->right) {
      Node *temp = node->right;
      while (temp->left) {
        temp = temp->left;
      }
      return temp;
    }

    Node *parent = node->parent;
    while (parent && node == parent->right) {
      node = parent;
      parent = parent->parent;
    }
    return parent;
  }

  Node *find_before(Node *node) const {
    if (node == nullptr)
      return nullptr;

    if (node->left) {
      Node *temp = node->left;
      while (temp->right) {
        temp = temp->right;
      }
      return temp;
    }

    Node *parent = node->parent;
    while (parent && node == parent->left) {
      node = parent;
      parent = parent->parent;
    }
    return parent;
  }

  void deep_copy(Node *&this_node, Node *other_node, Node *parent = nullptr) {
    if (other_node == nullptr) {
      this_node = nullptr;
      return;
    }
    this_node = new Node(other_node->data, parent);
    this_node->height = other_node->height;
    deep_copy(this_node->left, other_node->left, this_node);
    deep_copy(this_node->right, other_node->right, this_node);
  }

public:
  class const_iterator;
  class iterator {
  private:
    /**
     * TODO add data members
     *   just add whatever you want.
     */
    Node *node;
    map *map_ptr;
    friend class map;

  public:
    iterator() : node(nullptr), map_ptr(nullptr) {}

    iterator(Node *node, map *map_ptr) : node(node), map_ptr(map_ptr) {}

    iterator(const iterator &other)
        : node(other.node), map_ptr(other.map_ptr) {}

    /**
     * TODO iter++
     */
    iterator operator++(int) {
      iterator temp = *this;
      ++(*this);
      return temp;
    }

    /**
     * TODO ++iter
     */
    iterator &operator++() {
      if (node == nullptr) {
        throw invalid_iterator();
      }
      Node *p = node;

      node = map_ptr->find_after(p);
      return *this;
    }

    /**
     * TODO iter--
     */
    iterator operator--(int) {
      iterator temp = *this;
      --(*this);
      return temp;
    }

    /**
     * TODO --iter
     */
    iterator &operator--() {
      if (node == nullptr) {
        if (map_ptr == nullptr || map_ptr->empty()) {
          throw invalid_iterator();
        }
        node = map_ptr->max_node;
        if (node == nullptr) {
          throw invalid_iterator();
        }
      } else {
        Node *p = node;
        if (p == map_ptr->begin().node) {
          throw invalid_iterator();
        }

        node = map_ptr->find_before(p);
      }
      return *this;
    }

    /**
     * a operator to check whether two iterators are same (pointing to the
     * same memory).
     */
    value_type &operator*() const {
      if (node == nullptr) {
        throw invalid_iterator();
      }
      return node->data;
    }

    bool operator==(const iterator &rhs) const {
      return (node == rhs.node) && (map_ptr == rhs.map_ptr);
    }

    bool operator==(const const_iterator &rhs) const {
      return (node == rhs.node) && (map_ptr == rhs.map_ptr);
    }

    /**
     * some other operator for iterator.
     */
    bool operator!=(const iterator &rhs) const {
      return (node != rhs.node) || (map_ptr != rhs.map_ptr);
    }

    bool operator!=(const const_iterator &rhs) const {
      return (node != rhs.node) || (map_ptr != rhs.map_ptr);
    }

    /**
     * for the support of it->first.
     * See
     * <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/>
     * for help.
     */
    value_type *operator->() const noexcept { return &node->data; }
  };

  class const_iterator {
    // it should has similar member method as iterator.
    //  and it should be able to construct from an iterator.
  private:
    // data members.
    Node *node;
    const map *map_ptr;
    friend class map;

  public:
    const_iterator() : node(nullptr), map_ptr(nullptr) {}
    const_iterator(Node *node, const map *map_ptr)
        : node(node), map_ptr(map_ptr) {}

    const_iterator(const const_iterator &other)
        : node(other.node), map_ptr(other.map_ptr) {}

    const_iterator(const iterator &other)
        : node(other.node), map_ptr(other.map_ptr) {}
    const_iterator &operator=(const const_iterator &other) {
      if (this != &other) {
        node = other.node;
        map_ptr = other.map_ptr;
      }
      return *this;
    }
    const_iterator &operator=(const iterator &other) {
      node = other.node;
      map_ptr = other.map_ptr;
      return *this;
    }
    const_iterator operator++(int) {
      const_iterator temp = *this;
      ++(*this);
      return temp;
    }
    const_iterator &operator++() {
      if (node == nullptr) {
        throw invalid_iterator();
      }
      Node *p = node;

      node = map_ptr->find_after(p);
      return *this;
    }
    const_iterator operator--(int) {
      const_iterator temp = *this;
      --(*this);
      return temp;
    }
    const_iterator &operator--() {
      if (node == nullptr) {
        if (map_ptr == nullptr || map_ptr->empty()) {
          throw invalid_iterator();
        }
        node = map_ptr->max_node;
        if (node == nullptr) {
          throw invalid_iterator();
        }
      } else {
        Node *p = node;
        if (p == map_ptr->cbegin().node) {
          throw invalid_iterator();
        }

        node = map_ptr->find_before(p);
      }
      return *this;
    }
    const value_type &operator*() const {
      if (node == nullptr) {
        throw invalid_iterator();
      }
      return node->data;
    }
    const value_type *operator->() const noexcept {
      if (node == nullptr) {
        throw invalid_iterator();
      }
      return &node->data;
    }
    bool operator==(const const_iterator &rhs) const {
      return (node == rhs.node) && (map_ptr == rhs.map_ptr);
    }
    bool operator==(const iterator &rhs) const {
      return (node == rhs.node) && (map_ptr == rhs.map_ptr);
    }
    bool operator!=(const const_iterator &rhs) const {
      return (node != rhs.node) || (map_ptr != rhs.map_ptr);
    }
    bool operator!=(const iterator &rhs) const {
      return (node != rhs.node) || (map_ptr != rhs.map_ptr);
    }
  };

  /**
   * TODO two constructors
   */
  map() {
    root = nullptr;
    node_count = 0;
    begin_node = nullptr;
    max_node = nullptr;
  }

  map(const map &other) {
    root = nullptr;
    node_count = 0;
    deep_copy(root, other.root);
    node_count = other.node_count;
    // find the begin node
    if (root != nullptr) {
      begin_node = root;
      while (begin_node->left != nullptr) {
        begin_node = begin_node->left;
      }

      max_node = root;
      while (max_node->right != nullptr) {
        max_node = max_node->right;
      }
    } else {
      begin_node = nullptr;
      max_node = nullptr;
    }
  }

  /**
   * TODO assignment operator
   */
  map &operator=(const map &other) {
    if (this != &other) {
      clear();
      deep_copy(root, other.root);
      node_count = other.node_count;
      if (root != nullptr) {
        begin_node = root;
        while (begin_node->left != nullptr) {
          begin_node = begin_node->left;
        }
        max_node = root;
        while (max_node->right != nullptr) {
          max_node = max_node->right;
        }
      } else {
        begin_node = nullptr;
        max_node = nullptr;
      }
    }
    return *this;
  }

  /**
   * TODO Destructors
   */
  ~map() {
    if (root != nullptr) {
      root->clear();
    }
    delete root;
  }

  /**
   * TODO
   * access specified element with bounds checking
   * Returns a reference to the mapped value of the element with key
   * equivalent to key. If no such element exists, an exception of type
   * `index_out_of_bound'
   */
  T &at(const Key &key) {
    if (root == nullptr) {
      throw index_out_of_bound();
    }
    return find(key, root);
  }

  const T &at(const Key &key) const {
    if (root == nullptr) {
      throw index_out_of_bound();
    }
    return find(key, root);
  }

  /**
   * TODO
   * access specified element
   * Returns a reference to the value that is mapped to a key equivalent to
   * key, performing an insertion if such key does not already exist.
   */
  T &operator[](const Key &key) {
    Node *node = find_node(key, root);
    if (node != nullptr) {
      return node->data.second;
    } else {
      // Insert new element
      value_type value(key, T());
      std::pair<iterator, bool> result = insert(value);
      return result.first->second;
    }
  }

  /**
   * behave like at() throw index_out_of_bound if such key does not exist.
   */
  const T &operator[](const Key &key) const {
    if (root == nullptr) {
      throw index_out_of_bound();
    }
    return find(key, root);
  }

  /**
   * return a iterator to the beginning
   */
  iterator begin() {
    if (root == nullptr) {
      return iterator(nullptr, this);
    }
    /*Node *node = root;
    while (node->left != nullptr) {
        node = node->left;
    }
    return iterator(node, this);*/
    return iterator(begin_node, this);
  }

  const_iterator cbegin() const {
    if (root == nullptr) {
      return const_iterator(nullptr, this);
    }
    /*Node *node = root;
    while (node->left != nullptr) {
        node = node->left;
    }
    return const_iterator(node, this);*/
    return const_iterator(begin_node, this);
  }

  /**
   * return a iterator to the end
   * in fact, it returns past-the-end.
   */
  iterator end() { return iterator(nullptr, this); }

  const_iterator cend() const { return const_iterator(nullptr, this); }

  /**
   * checks whether the container is empty
   * return true if empty, otherwise false.
   */
  bool empty() const { return node_count == 0; }

  /**
   * returns the number of elements.
   */
  size_t size() const { return node_count; }

  /**
   * clears the contents
   */
  void clear() {
    if (root != nullptr) {
      root->clear();
      delete root;
      root = nullptr;
      begin_node = nullptr;
      max_node = nullptr;
    }
    node_count = 0;
  }

  /**
   * insert an element.
   * return a std::pair, the first of the std::pair is
   *   the iterator to the new element (or the element that prevented the
   * insertion), the second one is true if insert successfully, or false.
   */
  std::pair<iterator, bool> insert(const value_type &value) {
    Node *found_node = find_node(value.first, root);
    if (found_node != nullptr) {
      // Key already exists
      return std::pair<iterator, bool>(iterator(found_node, this), false);
    } else {
      size_t old_size = node_count;
      root = insert(value, root,
                    nullptr); // Pass nullptr as parent for root
      Node *new_node = find_node(value.first, root);
      if (begin_node == nullptr ||
          Compare()(value.first, begin_node->data.first)) {
        begin_node = new_node;
      }
      if (max_node == nullptr || Compare()(max_node->data.first, value.first)) {
        max_node = new_node;
      }
      return std::pair<iterator, bool>(iterator(new_node, this), true);
    }
  }

  /**
   * erase the element at pos.
   *
   * throw if pos pointed to a bad element (pos == this->end() || pos points
   * an element out of this)
   */
  void erase(iterator pos) {
    if (pos.node == nullptr || pos.map_ptr != this) {
      throw container_is_empty();
    }

    bool need_update_max = (pos.node == max_node);
    bool need_update_begin = (pos.node == begin_node);

    root = erase(pos.node->data.first, root, nullptr);

    if (root != nullptr) {
      if (need_update_begin || begin_node == nullptr) {
        begin_node = root;
        while (begin_node->left != nullptr) {
          begin_node = begin_node->left;
        }
      }

      if (need_update_max || max_node == nullptr) {
        max_node = root;
        while (max_node->right != nullptr) {
          max_node = max_node->right;
        }
      }
    } else {
      begin_node = nullptr;
      max_node = nullptr;
    }
  }

  /**
   * Returns the number of elements with key
   *   that compares equivalent to the specified argument,
   *   which is either 1 or 0
   *     since this container does not allow duplicates.
   * The default method of check the equivalence is !(a < b || b > a)
   */
  size_t count(const Key &key) const {
    if (root == nullptr) {
      return 0;
    }
    Node *node = find_node(key, root);
    if (node == nullptr) {
      return 0;
    }
    return 1;
  }

  /**
   * Finds an element with key equivalent to key.
   * key value of the element to search for.
   * Iterator to an element with key equivalent to key.
   *   If no such element is found, past-the-end (see end()) iterator is
   * returned.
   */
  iterator find(const Key &key) {
    if (root == nullptr) {
      return end();
    }
    Node *node = find_node(key, root);
    if (node == nullptr) {
      return end();
    }
    return iterator(node, this);
  }

  const_iterator find(const Key &key) const {
    if (root == nullptr) {
      return cend();
    }
    Node *node = find_node(key, root);
    if (node == nullptr) {
      return cend();
    }
    return const_iterator(node, this);
  }
};

} // namespace sjtu

#endif