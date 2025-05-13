#pragma once

#include <utility>
#include <vector>
#include <concepts>

#include "util.hpp"

template <NonVoidType T, typename P = int, typename C = std::less<P>>
class Heap {
private:
    std::vector<ValueWithPriority<T, P>> heap;
    C compare;

    size_t parent(const size_t i) const {
        return (i - 1) / 2;
    }

    size_t left_child(const size_t i) const {
        return 2 * i + 1;
    }

    size_t right_child(const size_t i) const {
        return 2 * i + 2;
    }

    void sift_up(size_t i) {
        while (i > 0 && compare(heap[parent(i)].priority, heap[i].priority)) {
            std::swap(heap[i], heap[parent(i)]);
            i = parent(i);
        }
    }

    void sift_down(size_t i) {
        auto largest = i;
        auto left = left_child(i);
        auto right = right_child(i);

        if (left < heap.size() && compare(heap[largest].priority, heap[left].priority)) {
            largest = left;

        }
        if (right < heap.size() && compare(heap[largest].priority, heap[right].priority)) {
            largest = right;
        }

        if (largest != i) {
            std::swap(heap[i], heap[largest]);
            sift_down(largest);
        }
    }

    template<Predicate<const T&> Pred>
    std::optional<size_t> find_index(Pred pred) const {
        for (size_t i = 0; i < heap.size(); ++i) {
            if (pred(heap[i].value)) {
                return i;
            }
        }
        return std::nullopt;
    }

public:
    Heap() : compare(C()) {}
    Heap(const C& compare) : compare(compare) {}

    void push(const P& priority, const T& value) {
        heap.emplace_back(ValueWithPriority<T, P>{ value, priority });
        sift_up(heap.size() - 1);
    }

    T pop() {
        if (empty()) {
            throw std::runtime_error("Heap is empty");
        }

        T top = heap.front().value;
        std::swap(heap.front(), heap.back());
        heap.pop_back();

        if (!empty()) {
            sift_down(0);
        }

        return top;
    }

    T peek() const {
        if (empty()) {
            throw std::runtime_error("Heap is empty");
        }

        return heap.front().value;
    }

    bool empty() const {
        return heap.empty();
    }

    size_t size() const {
        return heap.size();
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

        const auto old_priority = heap[*index].priority;
        heap[*index].priority = priority;

        if (compare(old_priority, priority)) {
            sift_up(*index);
        } else {
            sift_down(*index);
        }

        return true;
    }
};