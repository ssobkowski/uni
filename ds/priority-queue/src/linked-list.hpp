#pragma once

#include <utility>
#include <memory>

#include "util.hpp"

template <NonVoidType T, typename P = int, typename C = std::less<P>>
class LinkedList {
private:
    struct Node {
        ValueWithPriority<T, P> data;
        std::unique_ptr<Node> next;
        
        Node(const ValueWithPriority<T, P>& data) : data(data), next(nullptr) {}
    };
    
    std::unique_ptr<Node> head;
    C compare;
    size_t node_count = 0;
    
public:
    LinkedList() : compare(C()) {}
    LinkedList(const C& compare) : compare(compare) {}
    
    void push(const P& priority, const T& value) {
        auto new_node = std::make_unique<Node>(ValueWithPriority<T, P>{value, priority});
        
        // Empty list or new node has higher priority than head
        if (!head || compare(head->data.priority, priority)) {
            new_node->next = std::move(head);
            head = std::move(new_node);
        } else {
            Node* current = head.get();
            while (current->next && !compare(current->next->data.priority, priority)) {
                current = current->next.get();
            }
            new_node->next = std::move(current->next);
            current->next = std::move(new_node);
        }
        
        node_count++;
    }
    
    T pop() {
        if (empty()) {
            throw std::runtime_error("Priority queue is empty");
        }
        
        T top_value = head->data.value;
        head = std::move(head->next);
        node_count--;
        
        return top_value;
    }
    
    T peek() const {
        if (empty()) {
            throw std::runtime_error("Priority queue is empty");
        }
        
        return head->data.value;
    }
    
    bool empty() const {
        return head == nullptr;
    }
    
    size_t size() const {
        return node_count;
    }

    bool set_priority(const T& value, const P& priority) {
        return set_priority([&value](const T& v) { return v == value; }, priority);
    }
    
    template<Predicate<const T&> Pred>
    bool set_priority(Pred pred, const P& priority) {
        if (empty()) {
            return false;
        }
        
        // Special case: head contains the value
        if (pred(head->data.value)) {
            auto temp = std::move(head->next);
            head->next = nullptr;
            head->data.priority = priority;
            
            auto new_node = std::move(head);
            head = std::move(temp);
            node_count--;
            
            push(priority, head->data.value);
            return true;
        }
        
        Node* prev = head.get();
        while (prev->next && !pred(prev->next->data.value)) {
            prev = prev->next.get();
        }
        
        if (!prev->next) {
            return false; // Value not found
        }
        
        auto node_to_update = std::move(prev->next);
        prev->next = std::move(node_to_update->next);
        node_to_update->data.priority = priority;
        node_count--;
        
        push(priority, node_to_update->data.value);
        return true;
    }
};