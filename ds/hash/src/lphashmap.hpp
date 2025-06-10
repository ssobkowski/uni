#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <vector>

#include "hasher.hpp"
#include "sip.hpp"

template <HashMapKey K, typename V, typename H = Sip13Hasher<K>>
    requires HasherType<H, K>
class LpHashMap {
private:
    enum class EntryState {
        Empty,
        Occupied,
        Deleted  // Tombstone for deleted entries
    };

    struct Entry {
        K key;
        V value;
        EntryState state;
        
        Entry() : state(EntryState::Empty) {}
        Entry(K k, V v) : key(std::move(k)), value(std::move(v)), state(EntryState::Occupied) {}
    };

    std::vector<Entry> table;
    H hasher;
    size_t size_count;
    size_t capacity;
    size_t deleted_count;  // Track tombstones for rehashing decision
    
    static constexpr double MAX_LOAD_FACTOR = 0.7;
    static constexpr size_t INITIAL_CAPACITY = 16;

    void rehash() {
        auto old_table = std::move(table);
        
        capacity *= 2;
        table = std::vector<Entry>(capacity);
        size_count = 0;
        deleted_count = 0;
        
        for (const auto& entry : old_table) {
            if (entry.state == EntryState::Occupied) {
                inner_insert(K(entry.key), V(entry.value));
            }
        }
    }

    size_t get_hash(const K& key) const {
        return hasher.hash(key) % capacity;
    }

    size_t find_slot_for_insert(const K& key) {
        size_t index = get_hash(key);
        size_t original_index = index;
        
        do {
            if (table[index].state != EntryState::Occupied) {
                return index;
            }
            if (table[index].key == key) {
                return index;
            }
            index = (index + 1) % capacity;
        } while (index != original_index);
        
        throw std::runtime_error("Hash table is full");
    }

    size_t find_slot_for_lookup(const K& key) const {
        size_t index = get_hash(key);
        size_t original_index = index;
        
        do {
            if (table[index].state == EntryState::Empty) {
                return capacity;
            }
            if (table[index].state == EntryState::Occupied && table[index].key == key) {
                return index;
            }
            index = (index + 1) % capacity;
        } while (index != original_index);
        
        return capacity;
    }

    void inner_insert(K key, V value) {
        size_t index = find_slot_for_insert(key);
        
        if (table[index].state == EntryState::Occupied && table[index].key == key) {
            table[index].value = std::move(value);
        } else {
            if (table[index].state == EntryState::Deleted) {
                --deleted_count;
            }
            table[index] = Entry(std::move(key), std::move(value));
            ++size_count;
        }
    }

public:
    explicit LpHashMap(size_t initial_capacity = INITIAL_CAPACITY)
        : hasher(), size_count(0), capacity(initial_capacity), deleted_count(0)
    {
        table.resize(capacity);
    }

    void insert(K key, V value) {
        double effective_load = static_cast<double>(size_count + deleted_count) / capacity;
        if (static_cast<double>(size_count + 1) / capacity > MAX_LOAD_FACTOR || 
            effective_load > MAX_LOAD_FACTOR) {
            rehash();
        }
        inner_insert(std::move(key), std::move(value));
    }

    const V& at(const K& key) const {
        size_t index = find_slot_for_lookup(key);
        if (index < capacity) {
            return table[index].value;
        }
        throw std::out_of_range("Key not found in LpHashMap");
    }

    V& at(const K& key) {
        size_t index = find_slot_for_lookup(key);
        if (index < capacity) {
            return table[index].value;
        }
        throw std::out_of_range("Key not found in LpHashMap");
    }

    bool contains_key(const K& key) const {
        return find_slot_for_lookup(key) < capacity;
    }

    std::optional<V> remove(const K& key) {
        size_t index = find_slot_for_lookup(key);
        if (index < capacity) {
            V value = std::move(table[index].value);
            table[index].state = EntryState::Deleted;
            ++deleted_count;
            --size_count;
            return value;
        }
        return std::nullopt;
    }

    size_t size() const {
        return size_count;
    }

    bool empty() const {
        return size_count == 0;
    }

    void clear() {
        for (auto& entry : table) {
            entry.state = EntryState::Empty;
        }
        size_count = 0;
        deleted_count = 0;
    }

    double load_factor() const {
        return static_cast<double>(size_count) / capacity;
    }

    size_t bucket_count() const {
        return capacity;
    }

    V& operator[](const K& key) 
        requires std::is_default_constructible_v<V>
    {
        const auto index = find_slot_for_lookup(key);
        if (index < capacity) {
            return table[index].value;
        }
        
        insert(key, V{});
        
        const auto new_index = find_slot_for_lookup(key);
        if (new_index < capacity) {
            return table[new_index].value;
        }
        
        throw std::runtime_error("Internal error: failed to insert in operator[]");
    }
}; 