#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>
#include <optional>

#include "non-void.hpp"

template <NonVoidType T>
class DynamicArray {
private:
    T* m_data = nullptr;
    size_t m_capacity = 0;
    size_t m_size = 0;

    static constexpr size_t DEFAULT_CAPACITY = 512;

    void resize(size_t cap) {
        size_t new_cap = cap > 0 ? cap : 1;

        T* new_data = new T[new_cap];
        for (size_t i = 0; i < m_size; ++i) {
            new_data[i] = std::move(m_data[i]);
        }
        delete[] m_data;

        m_data = new_data;
        m_capacity = new_cap;
    }

public: 
    explicit DynamicArray(size_t n = DEFAULT_CAPACITY)
        : m_capacity(n), m_size(0)
    {
        m_data = new T[m_capacity];
    }

    ~DynamicArray()
    {
        delete[] m_data;
    }

    void push_back(const T& value) {
        if (m_size >= m_capacity) {
            resize(m_capacity * 2);
        }
        m_data[m_size++] = value;
    }

    void push_back(T&& value) {
        if (m_size >= m_capacity) {
            resize(m_capacity * 2);
        }
        m_data[m_size++] = std::move(value);
    }

    void push_front(const T& value) {
        insert(value, 0);
    }

    void push_front(T&& value) {
        insert(std::move(value), 0);
    }

    T pop_back() {
        if (m_size == 0) {
            throw std::out_of_range("Array is empty");
        }
        return std::move(m_data[--m_size]);
    }

    T pop_front() {
        if (m_size == 0) {
            throw std::out_of_range("Array is empty");
        }
        return remove(0);
    }

    void insert(const T& value, size_t at) {
        if (at > m_size) {
            throw std::out_of_range("Index out of range");
        }
        if (m_size >= m_capacity) {
            resize(m_capacity * 2);
        }
        for (size_t i = m_size; i > at; --i) {
            m_data[i] = std::move(m_data[i - 1]);
        }
        m_data[at] = value;
        ++m_size;
    }

    void insert(T&& value, size_t at) {
        if (at > m_size) {
            throw std::out_of_range("Index out of range");
        }
        if (m_size >= m_capacity) {
            resize(m_capacity * 2);
        }
        for (size_t i = m_size; i > at; --i) {
            m_data[i] = std::move(m_data[i - 1]);
        }
        m_data[at] = std::move(value);
        ++m_size;
    }

    T remove(size_t at) {
        if (at >= m_size) {
            throw std::out_of_range("Index out of range");
        }
        T value = std::move(m_data[at]);
        for (size_t i = at; i < m_size - 1; ++i) {
            m_data[i] = std::move(m_data[i + 1]);
        }
        --m_size;
        return value;
    }

    T& operator[](size_t idx) noexcept {
        return m_data[idx];
    }

    const T& operator[](size_t idx) const noexcept {
        return m_data[idx];
    }

    std::optional<size_t> find(const T& value) const {
        for (size_t i = 0; i < m_size; i++) {
            if (m_data[i] == value) {
                return i;
            }
        }
        return {};
    }

    size_t size() const noexcept {
        return m_size;
    }

    size_t capacity() const noexcept {
        return m_capacity;
    }

    size_t empty() const noexcept {
        return m_size == 0;
    }

    void reserve(size_t cap) {
        if (cap > m_capacity) {
            resize(cap);
        }
    }

    void clear() noexcept {
        m_size = 0;
    }
};
