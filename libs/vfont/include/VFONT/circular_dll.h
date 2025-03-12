/**
 * @file circular_dll.h
 * @author Christian Saloň
 */

#pragma once

#include <stdexcept>

namespace vft {

template <typename T>
class CircularDLL {
public:
    class Node {
    public:
        T value;
        Node *previous;
        Node *next;

        Node(T value);
        ~Node() = default;
    };

protected:
    unsigned int _size{0};

    Node *_front{nullptr};
    Node *_back{nullptr};

public:
    CircularDLL() = default;
    ~CircularDLL();

    CircularDLL(const CircularDLL &other);
    CircularDLL &operator=(const CircularDLL &other);

    void clear();

    void insertAt(T value, unsigned int index);
    void insertFirst(T value);
    void insertLast(T value);

    void deleteAt(unsigned int index);
    void deleteFirst();
    void deleteLast();

    Node *getAt(unsigned int index) const;
    Node *getValue(T value) const;
    Node *getFirst() const;
    Node *getLast() const;

    unsigned int size() const;
};

}  // namespace vft
