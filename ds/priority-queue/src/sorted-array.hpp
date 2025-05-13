#pragma once

#include <utility>
#include <vector>
#include <concepts>
#include <algorithm>

#include "util.hpp"

template <NonVoidType T, typename P = int, typename C = std::less<P>>
class SortedArray {
private:
    std::vector<ValueWithPriority<T, P>> array;
    C compare;

    template<Predicate<const T&> Pred>
    std::optional<size_t> find_index(Pred pred) const {
        for (size_t i = 0; i < array.size(); ++i) {
            if (pred(array[i].value)) {
                return i;
            }
        }
        return std::nullopt;
    }

    size_t find_insertion_position(const P& priority) const {
        size_t left = 0;
        size_t right = array.size();
        
        while (left < right) {
            size_t mid = left + (right - left) / 2;
            if (compare(array[mid].priority, priority)) {
                left = mid + 1;
            } else {
                right = mid;
            }
        }
        
        return left;
    }

public:
    SortedArray() : compare(C()) {}
    SortedArray(const C& compare) : compare(compare) {}

    void push(const P& priority, const T& value) {
        size_t pos = find_insertion_position(priority);
        array.emplace(array.begin() + pos, ValueWithPriority<T, P>{ value, priority });
    }

    T pop() {
        if (empty()) {
            throw std::runtime_error("SortedArray is empty");
        }

        T top = array.back().value;
        array.pop_back();
        return top;
    }

    T peek() const {
        if (empty()) {
            throw std::runtime_error("SortedArray is empty");
        }

        return array.back().value;
    }

    bool empty() const {
        return array.empty();
    }

    size_t size() const {
        return array.size();
    }

    bool set_priority(const T& value, const P& priority) {
        return set_priority([&value](const T& v) { return v == value; }, priority);
    }

    template<Predicate<const T&> Pred>
    bool set_priority(Pred pred, const P& priority) {
        const auto index = find_index(pred);
        if (!index) {
            return false;
        }

        T value = array[*index].value;
        array.erase(array.begin() + *index);
        
        size_t pos = find_insertion_position(priority);
        array.emplace(array.begin() + pos, ValueWithPriority<T, P>{ value, priority });

        return true;
    }
}; 
