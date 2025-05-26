#ifndef SJTU_VECTOR_HPP
#define SJTU_VECTOR_HPP

#include <strings.h>

#include <climits>
#include <cstddef>
#include <cstring>
#include <iostream>
#include "utils/exceptions.hpp"

namespace sjtu {
/**
 * a data container like std::vector
 * store data in a successive memory and support random access.
 */
template <typename T>
class vector {
   public:
    /**
     * TODO
     * a type for actions of the elements of a vector, and you should write
     *   a class named const_iterator with same interfaces.
     */
    /**
     * you can see RandomAccessIterator at CppReference for help.
     */
    class const_iterator;
    class iterator {
        // The following code is written for the C++ type_traits library.
        // Type traits is a C++ feature for describing certain properties of a
        // type. For instance, for an iterator, iterator::value_type is the type
        // that the iterator points to. STL algorithms and containers may use
        // these type_traits (e.g. the following typedef) to work properly. In
        // particular, without the following code,
        // @code{std::sort(iter, iter1);} would not compile.
        // See these websites for more information:
        // https://en.cppreference.com/w/cpp/header/type_traits
        // About value_type:
        // https://blog.csdn.net/u014299153/article/details/72419713 About
        // iterator_category: https://en.cppreference.com/w/cpp/iterator
       public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::output_iterator_tag;

       private:
        /**
         * TODO add data members
         *   just add whatever you want.
         */
        vector<T> *vec;
        size_t index;

       public:
        iterator(vector<T> *v, size_t idx) : vec(v), index(idx) {
        }

        /**
         * return a new iterator which pointer n-next elements
         * as well as operator-
         */
        iterator operator+(const int &n) const {
            // TODO
            return iterator(vec, index + n);
        }
        iterator operator-(const int &n) const {
            // TODO
            return iterator(vec, index - n);
        }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw
        // invaild_iterator.
        int operator-(const iterator &rhs) const {
            // TODO
            if (vec != rhs.vec) throw invalid_iterator();
            return index - rhs.index;
        }
        iterator &operator+=(const int &n) {
            // TODO
            index += n;
            return *this;
        }
        iterator &operator-=(const int &n) {
            // TODO
            index -= n;
            return *this;
        }
        /**
         * TODO iter++
         */
        iterator operator++(int) {
            iterator tmp = *this;
            ++index;
            return tmp;
        }
        /**
         * TODO ++iter
         */
        iterator &operator++() {
            ++index;
            return *this;
        }
        /**
         * TODO iter--
         */
        iterator operator--(int) {
            iterator tmp = *this;
            --index;
            return tmp;
        }
        /**
         * TODO --iter
         */
        iterator &operator--() {
            --index;
            return *this;
        }
        /**
         * TODO *it
         */
        T &operator*() const {
            return vec->at(index);
        }
        /**
         * a operator to check whether two iterators are same (pointing to the
         * same memory address).
         */
        bool operator==(const iterator &rhs) const {
            if (this->vec == rhs.vec && this->index == rhs.index) {
                return true;
            }
            return false;
        }
        bool operator==(const const_iterator &rhs) const {
            if (this->vec == rhs.vec && this->index == rhs.index) {
                return true;
            }
            return false;
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };
    /**
     * TODO
     * has same function as iterator, just for a const object.
     */
    class const_iterator {
       public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T *;
        using reference = T &;
        using iterator_category = std::output_iterator_tag;

       private:
        /*TODO*/
        const vector<T> *vec;
        size_t index;

       public:
        const_iterator(const vector<T> *v, size_t idx) : vec(v), index(idx) {
        }

        /**
         * return a new iterator which pointer n-next elements
         * as well as operator-
         */
        const_iterator operator+(const int &n) const {
            // TODO
            return const_iterator(vec, index + n);
        }
        const_iterator operator-(const int &n) const {
            // TODO
            return const_iterator(vec, index - n);
        }
        // return the distance between two iterators,
        // if these two iterators point to different vectors, throw
        // invaild_iterator.
        int operator-(const const_iterator &rhs) const {
            // TODO
            if (vec != rhs.vec) throw invalid_iterator();
            return index - rhs.index;
        }
        const_iterator &operator+=(const int &n) {
            // TODO
            index += n;
            return *this;
        }
        const_iterator &operator-=(const int &n) {
            // TODO
            index -= n;
            return *this;
        }
        /**
         * TODO iter++
         */
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++index;
            return tmp;
        }
        /**
         * TODO ++iter
         */
        const_iterator &operator++() {
            ++index;
            return *this;
        }
        /**
         * TODO iter--
         */
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --index;
            return tmp;
        }
        /**
         * TODO --iter
         */
        const_iterator &operator--() {
            --index;
            return *this;
        }
        /**
         * TODO *it
         */
        const T &operator*() const {
            return vec->at(index);
        }
        /**
         * a operator to check whether two iterators are same (pointing to the
         * same memory address).
         */
        bool operator==(const iterator &rhs) const {
            if (this->vec == rhs.vec && this->index == rhs.index) {
                return true;
            }
            return false;
        }
        bool operator==(const const_iterator &rhs) const {
            if (this->vec == rhs.vec && this->index == rhs.index) {
                return true;
            }
            return false;
        }
        /**
         * some other operator for iterator.
         */
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
    };
    /**
     * TODO Constructs
     * At least two: default constructor, copy constructor
     */
    vector() : capacity(0), length(0), container(nullptr) {
    }
    vector(const vector &other) {
        capacity = other.capacity;
        length = other.length;
        container = static_cast<T *>(operator new[](capacity * sizeof(T)));
        for (size_t i = 0; i < length; ++i) {
            new (container + i) T(other.container[i]);
        }
    }
    vector(size_t n) : capacity(n), length(0) {
        if (n == 0) {
            container = nullptr;
        } else {
            container = static_cast<T *>(operator new[](capacity * sizeof(T)));
        }
    }
    vector(size_t n, const T &value) : capacity(n), length(n) {
        if (n == 0) {
            container = nullptr;
        } else {
            container = static_cast<T *>(operator new[](capacity * sizeof(T)));
            for (size_t i = 0; i < n; ++i) {
                new (container + i) T(value);
            }
        }
    }
    /**
     * TODO Destructor
     */
    ~vector() {
        clear();
    }
    /**
     * TODO Assignment operator
     */
    vector &operator=(const vector &other) {
        if (this == &other) {
            return *this;
        }
        clear();
        capacity = other.capacity;
        length = other.length;
        container = static_cast<T *>(operator new[](capacity * sizeof(T)));
        for (size_t i = 0; i < length; ++i) {
            new (container + i) T(other.container[i]);
        }
        return *this;
    }

    void assign(size_t n, const T &value) {
        clear();
        capacity = n;
        length = n;
        if (n == 0) {
            container = nullptr;
        } else {
            container = static_cast<T *>(operator new[](capacity * sizeof(T)));
            for (size_t i = 0; i < n; ++i) {
                new (container + i) T(value);
            }
        }
    }

    T* data() {
        return container;
    }
    const T* data() const {
        return container;
    }

    /**
     * assigns specified element with bounds checking
     * throw index_out_of_bound if pos is not in [0, size)
     */
    T &at(const size_t &pos) {
        if (pos < 0 || pos > length) {
            throw index_out_of_bound();
        }
        return container[pos];
    }
    const T &at(const size_t &pos) const {
        if (pos < 0 || pos > length) {
            throw index_out_of_bound();
        }
        return container[pos];
    }
    /**
     * assigns specified element with bounds checking
     * throw index_out_of_bound if pos is not in [0, size)
     * !!! Pay attentions
     *   In STL this operator does not check the boundary but I want you to do.
     */
    T &operator[](const size_t &pos) {
        if (pos < 0 || pos > length) {
            throw index_out_of_bound();
        }
        return this->at(pos);
    }
    const T &operator[](const size_t &pos) const {
        if (pos < 0 || pos > length) {
            throw index_out_of_bound();
        }
        return this->at(pos);
    }
    /**
     * access the first element.
     * throw container_is_empty if size == 0
     */
    const T &front() const {
        if (length == 0) {
            throw container_is_empty();
        }
        return this->at(0);
    }
    /**
     * access the last element.
     * throw container_is_empty if size == 0
     */
    const T &back() const {
        if (length == 0) {
            throw container_is_empty();
        }
        return this->at(length - 1);
    }
    /**
     * returns an iterator to the beginning.
     */
    iterator begin() {
        return iterator(this, 0);
    }
    const_iterator begin() const {
        return const_iterator(this, 0);
    }
    const_iterator cbegin() const {
        return const_iterator(this, 0);
    }
    /**
     * returns an iterator to the end.
     */
    iterator end() {
        return iterator(this, length);
    }
    const_iterator end() const {
        return const_iterator(this, length);
    }
    const_iterator cend() const {
        return const_iterator(this, length);
    }
    /**
     * checks whether the container is empty
     */
    bool empty() const {
        if (length == 0) return true;
        return false;
    }
    /**
     * returns the number of elements
     */
    size_t size() const {
        return length;
    }
    /**
     * clears the contents
     */
    void clear() {
        for (size_t i = 0; i < length; ++i) {
            container[i].~T();
        }
        operator delete[](container);
        container = nullptr;
        capacity = 0;
        length = 0;
    }
    /**
     * inserts value before pos
     * returns an iterator pointing to the inserted value.
     */
    iterator insert(iterator pos, const T &value) {
        if (length == capacity) {
            size_t offset = pos - begin();
            double_space();
            pos = begin() + offset;
        }

        for (size_t i = length; i > (pos - begin()); --i) {
            new (container + i) T(std::move(container[i - 1]));
            container[i - 1].~T();
        }

        new (container + (pos - begin())) T(value);
        length++;
        return pos;
    }
    /**
     * inserts value at index ind.
     * after inserting, this->at(ind) == value
     * returns an iterator pointing to the inserted value.
     * throw index_out_of_bound if ind > size (in this situation ind can be size
     * because after inserting the size will increase 1.)
     */
    iterator insert(const size_t &ind, const T &value) {
        if (ind > length) {
            throw index_out_of_bound();
        }
        if (length == capacity) {
            double_space();
        }
        for (size_t i = length; i > ind; --i) {
            new (container + i) T(std::move(container[i - 1]));
            container[i - 1].~T();
        }

        new (container + ind) T(value);
        length++;
        return iterator(this, ind);
    }
    /**
     * removes the element at pos.
     * return an iterator pointing to the following element.
     * If the iterator pos refers the last element, the end() iterator is
     * returned.
     */
    iterator erase(iterator pos) {
        iterator next = pos + 1;
        for (iterator i = pos; i != end() - 1; ++i) {
            *i = std::move(*(i + 1));
            container[i - begin()].~T();
        }
        container[length - 1].~T();

        length--;
        return next;
    }
    /**
     * removes the element with index ind.
     * return an iterator pointing to the following element.
     * throw index_out_of_bound if ind >= size
     */
    iterator erase(const size_t &ind) {
        if (ind >= length) {
            throw index_out_of_bound();
        }
        iterator pos = begin() + ind;

        for (iterator i = pos; i < end() - 1; ++i) {
            *i = std::move(*(i + 1));
        }
        container[length - 1].~T();
        length--;

        return pos + 1;
    }
    /**
     * adds an element to the end.
     */
    void push_back(const T &value) {
        if (length == capacity) {
            double_space();
        }
        new (container + length) T(value);
        length++;
    }
    /**
     * remove the last element from the end.
     * throw container_is_empty if size() == 0
     */
    void pop_back() {
        if (length == 0) throw container_is_empty();
        container[length - 1].~T();
        length--;
    }

   private:
    size_t capacity;
    size_t length;
    T *container;

    void double_space() {
        size_t new_capacity = (capacity == 0) ? 1 : capacity * 2;
        T *new_container =
            static_cast<T *>(operator new[](new_capacity * sizeof(T)));
        for (size_t i = 0; i < capacity; ++i) {
            new (new_container + i) T(std::move(container[i]));
            container[i].~T();
        }

        operator delete[](container);
        container = new_container;
        capacity = new_capacity;
    }
};

}  // namespace sjtu

#endif