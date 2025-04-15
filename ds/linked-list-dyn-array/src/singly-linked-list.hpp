#pragma once

#include <optional>
#include <stdexcept>

#include "non-void.hpp"

template <NonVoidType T>
class SinglyLinkedList {
private:
    struct Node {
        T data;
        Node* next = nullptr;

        Node(const T& value) : data(value) {}
        Node(T&& value) : data(std::move(value)) {}
    };

    Node* m_head = nullptr;
    size_t m_size = 0;

public:
    SinglyLinkedList() = default;

    ~SinglyLinkedList() {
        clear();
    }

    void push_front(const T& value) {
        Node* new_node = new Node(value);
        new_node->next = m_head;
        m_head = new_node;
        ++m_size;
    }

    void push_front(T&& value) {
        Node* new_node = new Node(std::move(value));
        new_node->next = m_head;
        m_head = new_node;
        ++m_size;
    }

    void push_back(const T& value) {
        if (!m_head) {
            push_front(value);
            return;
        }

        Node* current = m_head;
        while (current->next) {
            current = current->next;
        }
        current->next = new Node(value);
        ++m_size;
    }

    void push_back(T&& value) {
        if (!m_head) {
            push_front(std::move(value));
            return;
        }

        Node* current = m_head;
        while (current->next) {
            current = current->next;
        }
        current->next = new Node(std::move(value));
        ++m_size;
    }

    T pop_front() {
        if (!m_head) {
            throw std::out_of_range("List is empty");
        }

        Node* temp = m_head;
        T value = std::move(temp->data);
        m_head = m_head->next;
        delete temp;
        --m_size;
        return value;
    }

    T pop_back() {
        if (!m_head) {
            throw std::out_of_range("List is empty");
        }

        if (!m_head->next) {
            return pop_front();
        }

        Node* current = m_head;
        while (current->next->next) {
            current = current->next;
        }

        T value = std::move(current->next->data);
        delete current->next;
        current->next = nullptr;
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

        Node* current = m_head;
        for (size_t i = 0; i < at - 1; ++i) {
            current = current->next;
        }

        Node* new_node = new Node(value);
        new_node->next = current->next;
        current->next = new_node;
        ++m_size;
    }

    T remove(size_t at) {
        if (at >= m_size) {
            throw std::out_of_range("Index out of range");
        }

        if (at == 0) {
            return pop_front();
        }

        Node* current = m_head;
        for (size_t i = 0; i < at - 1; ++i) {
            current = current->next;
        }

        Node* to_remove = current->next;
        T value = std::move(to_remove->data);
        current->next = to_remove->next;
        delete to_remove;
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
        m_size = 0;
    }
};

