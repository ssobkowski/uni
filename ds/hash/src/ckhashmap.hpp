#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <vector>
#include <random>

#include "hasher.hpp"
#include "sip.hpp"

// Concept to detect if a hasher can be constructed with two uint64_t seeds
template<typename H, typename K>
concept SeedableHasher = requires(uint64_t seed1, uint64_t seed2) {
    H(seed1, seed2);
};

template <HashMapKey K, typename V, typename H = Sip13Hasher<K>>
    requires HasherType<H, K>
class CkHashMap {
private:
    struct Entry {
        K key;
        V value;
        bool occupied;
        
        Entry() : occupied(false) {}
        Entry(K k, V v) : key(std::move(k)), value(std::move(v)), occupied(true) {}
    };

    std::vector<Entry> table1;
    std::vector<Entry> table2;
    H hasher1;
    H hasher2;
    size_t size_count;
    size_t capacity;
    mutable std::mt19937_64 rng;
    
    static constexpr double MAX_LOAD_FACTOR = 0.5;
    static constexpr size_t INITIAL_CAPACITY = 16;
    static constexpr size_t MAX_REHASH_ATTEMPTS = 8;

    // Helper to create a hasher with or without seeding
    H create_hasher() {
        if constexpr (SeedableHasher<H, K>) {
            return H(rng(), rng());
        } else {
            return H{};
        }
    }

    void rehash() {
        auto old_table1 = std::move(table1);
        auto old_table2 = std::move(table2);
        
        capacity *= 2;
        table1 = std::vector<Entry>(capacity);
        table2 = std::vector<Entry>(capacity);
        size_count = 0;
        
        // Create new hash functions with different random seeds (if supported)
        hasher1 = create_hasher();
        hasher2 = create_hasher();
        
        // Reinsert all elements
        for (const auto& entry : old_table1) {
            if (entry.occupied) {
                inner_insert(K(entry.key), V(entry.value));
            }
        }
        
        for (const auto& entry : old_table2) {
            if (entry.occupied) {
                inner_insert(K(entry.key), V(entry.value));
            }
        }
    }

    size_t get_hash1(const K& key) const {
        return hasher1.hash(key) % capacity;
    }
    
    size_t get_hash2(const K& key) const {
        return hasher2.hash(key) % capacity;
    }

    bool inner_insert(K key, V value) {
        // Check if key already exists and update
        size_t pos1 = get_hash1(key);
        size_t pos2 = get_hash2(key);
        
        if (table1[pos1].occupied && table1[pos1].key == key) {
            table1[pos1].value = std::move(value);
            return true;
        }
        
        if (table2[pos2].occupied && table2[pos2].key == key) {
            table2[pos2].value = std::move(value);
            return true;
        }
        
        // Try to insert in table1 first
        if (!table1[pos1].occupied) {
            table1[pos1] = Entry(std::move(key), std::move(value));
            ++size_count;
            return true;
        }
        
        // Try to insert in table2
        if (!table2[pos2].occupied) {
            table2[pos2] = Entry(std::move(key), std::move(value));
            ++size_count;
            return true;
        }
        
        // Both positions occupied, start cuckoo eviction
        Entry current_entry(std::move(key), std::move(value));
        bool use_table1 = true;
        
        for (size_t i = 0; i < capacity; ++i) {
            if (use_table1) {
                pos1 = get_hash1(current_entry.key);
                if (!table1[pos1].occupied) {
                    table1[pos1] = std::move(current_entry);
                    ++size_count;
                    return true;
                }
                std::swap(current_entry, table1[pos1]);
                use_table1 = false;
            } else {
                pos2 = get_hash2(current_entry.key);
                if (!table2[pos2].occupied) {
                    table2[pos2] = std::move(current_entry);
                    ++size_count;
                    return true;
                }
                std::swap(current_entry, table2[pos2]);
                use_table1 = true;
            }
        }
        
        // Failed to insert after maximum attempts - need to rehash
        return false;
    }

    static uint64_t get_random_seed() {
        std::random_device rd;
        return rd();
    }

public:
    explicit CkHashMap(size_t initial_capacity = INITIAL_CAPACITY)
        : size_count(0), capacity(initial_capacity), rng(get_random_seed())
    {
        table1.resize(capacity);
        table2.resize(capacity);
        
        // Initialize hash functions with different seeds (if supported)
        hasher1 = create_hasher();
        hasher2 = create_hasher();
    }

    void insert(K key, V value) {
        if (static_cast<double>(size_count + 1) / capacity > MAX_LOAD_FACTOR) {
            rehash();
        }
        
        size_t attempts = 0;
        while (!inner_insert(K(key), V(value)) && attempts < MAX_REHASH_ATTEMPTS) {
            rehash();
            ++attempts;
        }
        
        if (attempts >= MAX_REHASH_ATTEMPTS) {
            throw std::runtime_error("Failed to insert after maximum rehash attempts");
        }
    }

    const V& at(const K& key) const {
        size_t pos1 = get_hash1(key);
        size_t pos2 = get_hash2(key);
        
        if (table1[pos1].occupied && table1[pos1].key == key) {
            return table1[pos1].value;
        }
        
        if (table2[pos2].occupied && table2[pos2].key == key) {
            return table2[pos2].value;
        }
        
        throw std::out_of_range("Key not found in CkHashMap");
    }

    V& at(const K& key) {
        size_t pos1 = get_hash1(key);
        size_t pos2 = get_hash2(key);
        
        if (table1[pos1].occupied && table1[pos1].key == key) {
            return table1[pos1].value;
        }
        
        if (table2[pos2].occupied && table2[pos2].key == key) {
            return table2[pos2].value;
        }
        
        throw std::out_of_range("Key not found in CkHashMap");
    }

    bool contains_key(const K& key) const {
        size_t pos1 = get_hash1(key);
        size_t pos2 = get_hash2(key);
        
        return (table1[pos1].occupied && table1[pos1].key == key) ||
               (table2[pos2].occupied && table2[pos2].key == key);
    }

    std::optional<V> remove(const K& key) {
        size_t pos1 = get_hash1(key);
        size_t pos2 = get_hash2(key);
        
        if (table1[pos1].occupied && table1[pos1].key == key) {
            V value = std::move(table1[pos1].value);
            table1[pos1].occupied = false;
            --size_count;
            return value;
        }
        
        if (table2[pos2].occupied && table2[pos2].key == key) {
            V value = std::move(table2[pos2].value);
            table2[pos2].occupied = false;
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
        for (auto& entry : table1) {
            entry.occupied = false;
        }
        for (auto& entry : table2) {
            entry.occupied = false;
        }
        size_count = 0;
    }

    double load_factor() const {
        return static_cast<double>(size_count) / capacity;
    }

    size_t bucket_count() const {
        return capacity * 2; // Two tables
    }

    V& operator[](const K& key) 
        requires std::is_default_constructible_v<V>
    {
        size_t pos1 = get_hash1(key);
        size_t pos2 = get_hash2(key);
        
        // Check if key exists in table1
        if (table1[pos1].occupied && table1[pos1].key == key) {
            return table1[pos1].value;
        }
        
        // Check if key exists in table2
        if (table2[pos2].occupied && table2[pos2].key == key) {
            return table2[pos2].value;
        }
        
        // Key doesn't exist, insert with default value
        insert(key, V{});
        
        // Now find it again (it should be there after insert)
        if (table1[pos1].occupied && table1[pos1].key == key) {
            return table1[pos1].value;
        }
        
        if (table2[pos2].occupied && table2[pos2].key == key) {
            return table2[pos2].value;
        }
        
        // This should never happen if insert worked correctly
        throw std::runtime_error("Internal error: failed to insert in operator[]");
    }
}; 