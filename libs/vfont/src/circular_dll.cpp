/**
 * @file circular_dll.cpp
 * @author Christian Saloň
 */

#include "circular_dll.h"

namespace vft {

/**
 * @brief Constructor for an element in CircularDLL
 *
 * @tparam T Type of values stored in list
 *
 * @param value Value of element
 */
template <typename T>
CircularDLL<T>::Node::Node(T value) : value{value} {}

/**
 * @brief Destructor for CircularDLL. Deallocates used memory
 *
 * @tparam T
 */
template <typename T>
CircularDLL<T>::~CircularDLL() {
    this->clear();
}

/**
 * @brief Copy constructor for CircularDLL
 *
 * @tparam T Type of values stored in list
 *
 * @param other List used for copying
 */
template <typename T>
CircularDLL<T>::CircularDLL(const CircularDLL &other) {
    Node *current = other.getFirst();

    for (unsigned int i = 0; i < other.size(); i++) {
        this->insertLast(current->value);
        current = current->next;
    }
}

/**
 * @brief Assignment operator for CircularDLL
 *
 * @tparam T Type of values stored in list
 *
 * @param other Other list
 *
 * @return Reference to list after assignment
 */
template <typename T>
CircularDLL<T> &CircularDLL<T>::operator=(const CircularDLL &other) {
    if (this == &other) {
        return *this;
    }

    this->clear();
    Node *current = other.getFirst();

    for (unsigned int i = 0; i < other.size(); i++) {
        this->insertLast(current->value);
        current = current->next;
    }

    return *this;
}

/**
 * @brief Delete all elements of list
 *
 * @tparam T Type of values stored in list
 */
template <typename T>
void CircularDLL<T>::clear() {
    if (this->_size == 0) {
        return;
    }

    // Break circularity at last node
    this->_back->next = nullptr;

    Node *current = this->_front;
    while (current) {
        this->_front = current->next;
        delete current;
        current = this->_front;
    }

    this->_size = 0;
    this->_front = nullptr;
    this->_back = nullptr;
}

/**
 * @brief Inserts new element at given index
 *
 * @tparam T Type of values stored in list
 *
 * @param value Value of new element
 * @param index Index where to insert new element
 */
template <typename T>
void CircularDLL<T>::insertAt(T value, unsigned int index) {
    index = index % (this->_size + 1);

    if (index == 0) {
        this->insertFirst(value);
        return;
    }

    if (index == this->_size) {
        this->insertLast(value);
        return;
    }

    // current will be one index before new node
    Node *current = this->_front;
    for (unsigned int i = 0; i < index - 1; i++) {
        current = current->next;
    }

    Node *newNode = new Node(value);
    newNode->previous = current;
    newNode->next = current->next;

    current->next->previous = newNode;
    current->next = newNode;

    this->_size++;
}

/**
 * @brief Insert new element at start of list
 *
 * @tparam T Type of values stored in list
 *
 * @param value Value of new element
 */
template <typename T>
void CircularDLL<T>::insertFirst(T value) {
    Node *newNode = new Node(value);

    if (this->_size == 0) {
        newNode->next = newNode;
        newNode->previous = newNode;

        this->_front = newNode;
        this->_back = newNode;
    } else {
        newNode->next = this->_front;
        newNode->previous = this->_back;

        this->_front->previous = newNode;
        this->_back->next = newNode;

        this->_front = newNode;
    }

    this->_size++;
}

/**
 * @brief Insert new element at end of list
 *
 * @tparam T Type of values stored in list
 *
 * @param value Value of new element
 */
template <typename T>
void CircularDLL<T>::insertLast(T value) {
    Node *newNode = new Node(value);

    if (this->_size == 0) {
        newNode->next = newNode;
        newNode->previous = newNode;

        this->_front = newNode;
        this->_back = newNode;
    } else {
        newNode->next = this->_front;
        newNode->previous = this->_back;

        this->_front->previous = newNode;
        this->_back->next = newNode;

        this->_back = newNode;
    }

    this->_size++;
}

/**
 * @brief Removes element from list at given index
 *
 * @tparam T Type of values stored in list
 *
 * @param index Index of element which to remove
 */
template <typename T>
void CircularDLL<T>::deleteAt(unsigned int index) {
    index = index % this->_size;

    if (index == 0) {
        this->deleteFirst();
        return;
    }

    if (index == this->_size - 1) {
        this->deleteLast();
        return;
    }

    // current will be one index before new node
    Node *toDelete = this->_front;
    for (unsigned int i = 0; i < index; i++) {
        toDelete = toDelete->next;
    }

    toDelete->previous->next = toDelete->next;
    toDelete->next->previous = toDelete->previous;

    this->_size--;
}

/**
 * @brief Removes the first element of list
 *
 * @tparam T Type of values stored in list
 */
template <typename T>
void CircularDLL<T>::deleteFirst() {
    if (this->_size == 0) {
        throw std::out_of_range("CircularDLL::deleteFirst(): No nodes to delete");
    }

    Node *first = this->_front;

    if (this->_size == 1) {
        this->_front = nullptr;
        this->_back = nullptr;
    } else {
        this->_front = this->_front->next;
        this->_front->previous = this->_back;
        this->_back->next = this->_front;
    }

    delete first;
    this->_size--;
}

/**
 * @brief Removes the last element of list
 *
 * @tparam T Type of values stored in list
 */
template <typename T>
void CircularDLL<T>::deleteLast() {
    if (this->_size == 0) {
        throw std::out_of_range("CircularDLL::deleteLast(): No nodes to delete");
    }

    Node *last = this->_back;

    if (this->_size == 1) {
        this->_front = nullptr;
        this->_back = nullptr;
    } else {
        this->_back = this->_back->previous;
        this->_back->next = this->_front;
        this->_front->previous = this->_back;
    }

    delete last;
    this->_size--;
}

/**
 * @brief Get element at given index
 *
 * @tparam T Type of values stored in list
 *
 * @param index Index of element
 *
 * @return Pointer to element
 */
template <typename T>
CircularDLL<T>::Node *CircularDLL<T>::getAt(unsigned int index) const {
    if (index >= this->_size) {
        throw std::out_of_range("CircularDLL::getAt(): Index must be smaller than size");
    }

    Node *current = this->_front;
    for (unsigned int i = 0; i < index; i++) {
        current = current->next;
    }

    return current;
}

/**
 * @brief Get first element with given value
 *
 * @tparam T Type of values stored in list
 *
 * @param value Value of element
 *
 * @return Pointer to element
 */
template <typename T>
CircularDLL<T>::Node *CircularDLL<T>::getValue(T value) const {
    Node *current = this->_front;
    for (unsigned int i = 0; i < this->_size; i++) {
        if (current->value == value) {
            return current;
        }

        current = current->next;
    }

    return nullptr;
}

/**
 * @brief Get first element in list
 *
 * @tparam T Type of values stored in list
 *
 * @return Pointer to first element
 */
template <typename T>
CircularDLL<T>::Node *CircularDLL<T>::getFirst() const {
    return this->_front;
}

/**
 * @brief Get last element in list
 *
 * @tparam T Type of values stored in list
 *
 * @return Pointer to last element
 */
template <typename T>
CircularDLL<T>::Node *CircularDLL<T>::getLast() const {
    return this->_back;
}

/**
 * @brief Getter for the size of list
 *
 * @tparam T Type of values stored in list
 *
 * @return Size of list
 */
template <typename T>
unsigned int CircularDLL<T>::size() const {
    return this->_size;
}

template class CircularDLL<uint32_t>;
template class CircularDLL<Edge>;

}  // namespace vft
