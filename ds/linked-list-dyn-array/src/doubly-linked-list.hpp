#pragma once

#include <optional>
#include <stdexcept>

#include "non-void.hpp"

template <NonVoidType T>
class DoublyLinkedList {
private:
    struct Node {
        T data;
        Node* prev = nullptr;
        Node* next = nullptr;

        Node(const T& value) : data(value) {}
        Node(T&& value) : data(std::move(value)) {}
    };

    Node* m_head = nullptr;
    Node* m_tail = nullptr;
    size_t m_size = 0;

public:
    DoublyLinkedList() = default;

    ~DoublyLinkedList() {
        clear();
    }

    void push_front(const T& value) {
        Node* new_node = new Node(value);
        if (!m_head) {
            m_head = m_tail = new_node;
        } else {
            new_node->next = m_head;
            m_head->prev = new_node;
            m_head = new_node;
        }
        ++m_size;
    }

    void push_front(T&& value) {
        Node* new_node = new Node(std::move(value));
        if (!m_head) {
            m_head = m_tail = new_node;
        } else {
            new_node->next = m_head;
            m_head->prev = new_node;
            m_head = new_node;
        }
        ++m_size;
    }

    void push_back(const T& value) {
        Node* new_node = new Node(value);
        if (!m_tail) {
            m_head = m_tail = new_node;
        } else {
            new_node->prev = m_tail;
            m_tail->next = new_node;
            m_tail = new_node;
        }
        ++m_size;
    }

    void push_back(T&& value) {
        Node* new_node = new Node(std::move(value));
        if (!m_tail) {
            m_head = m_tail = new_node;
        } else {
            new_node->prev = m_tail;
            m_tail->next = new_node;
            m_tail = new_node;
        }
        ++m_size;
    }

    T pop_front() {
        if (!m_head) {
            throw std::out_of_range("List is empty");
        }

        Node* temp = m_head;
        T value = std::move(temp->data);

        m_head = m_head->next;
        if (m_head) {
            m_head->prev = nullptr;
        } else {
            m_tail = nullptr;
        }

        delete temp;
        --m_size;
        return value;
    }

    T pop_back() {
        if (!m_tail) {
            throw std::out_of_range("List is empty");
        }

        Node* temp = m_tail;
        T value = std::move(temp->data);

        m_tail = m_tail->prev;
        if (m_tail) {
            m_tail->next = nullptr;
        } else {
            m_head = nullptr;
        }

        delete temp;
        --m_size;
        return value;
    }

    void insert(const T& value, size_t at) {
        if (at > m_size) {
            throw std::out_of_range("Index out of range");
        }

        if (at == 0) {
            push_front(value);
            return;
        }

        if (at == m_size) {
            push_back(value);
            return;
        }

        Node* current = m_head;
        for (size_t i = 0; i < at; ++i) {
            current = current->next;
        }

        Node* new_node = new Node(value);
        new_node->prev = current->prev;
        new_node->next = current;
        current->prev->next = new_node;
        current->prev = new_node;
        ++m_size;
    }

    T remove(size_t at) {
        if (at >= m_size) {
            throw std::out_of_range("Index out of range");
        }

        if (at == 0) {
            return pop_front();
        }

        if (at == m_size - 1) {
            return pop_back();
        }

        Node* current = m_head;
        for (size_t i = 0; i < at; ++i) {
            current = current->next;
        }

        T value = std::move(current->data);
        current->prev->next = current->next;
        current->next->prev = current->prev;
        delete current;
        --m_size;
        return value;
    }

    std::optional<size_t> find(const T& value) const {
        Node* current = m_head;
        size_t index = 0;

        while (current) {
            if (current->data == value) {
                return index;
            }
            current = current->next;
            ++index;
        }
        return {};
    }

    size_t size() const noexcept {
        return m_size;
    }

    bool empty() const noexcept {
        return m_size == 0;
    }

    void clear() noexcept {
        while (m_head) {
            Node* temp = m_head;
            m_head = m_head->next;
            delete temp;
        }
        m_head = m_tail = nullptr;
        m_size = 0;
    }
};
