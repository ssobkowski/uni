#pragma once

#include <concepts>
#include <memory>
#include <optional>
#include <vector>

#include "hasher.hpp"
#include "sip.hpp"

template <HashMapKey K, typename V, typename H = Sip13Hasher<K>>
    requires HasherType<H, K>
class ScHashMap {
private:
    struct Entry {
        K key;
        V value;
        
        Entry(K k, V v) : key(std::move(k)), value(std::move(v)) {}
    };

    using Bucket = std::vector<Entry>;
    
    std::vector<Bucket> buckets;
    H hasher;
    size_t size_count;
    size_t capacity;
    
    static constexpr double MAX_LOAD_FACTOR = 0.75;
    static constexpr size_t INITIAL_CAPACITY = 16;

    void resize() {
        auto old_buckets = std::move(buckets);
        capacity *= 2;
        buckets = std::vector<Bucket>(capacity);
        size_count = 0;

        for (auto& bucket : old_buckets) {
            for (auto& entry : bucket) {
                inner_insert(std::move(entry.key), std::move(entry.value));
            }
        }
    }

    size_t get_bucket_index(const K& key) const {
        return hasher.hash(key) % capacity;
    }

    void inner_insert(K key, V value) {
        const auto index = get_bucket_index(key);
        auto& bucket = buckets[index];
        
        for (auto& entry : bucket) {
            if (entry.key == key) {
                entry.value = std::move(value);
                return;
            }
        }
        
        bucket.emplace_back(std::move(key), std::move(value));
        ++size_count;
    }

public:
    explicit ScHashMap(size_t initial_capacity = INITIAL_CAPACITY)
        : hasher(), size_count(0), capacity(initial_capacity)
    {
        buckets.resize(capacity);
    }

    void insert(K key, V value) {
        if (static_cast<double>(size_count + 1) / capacity > MAX_LOAD_FACTOR) {
            resize();
        }
        inner_insert(std::move(key), std::move(value));
    }

    const V& at(const K& key) const {
        const auto index = get_bucket_index(key);
        const auto& bucket = buckets[index];
        
        for (const auto& entry : bucket) {
            if (entry.key == key) {
                return entry.value;
            }
        }
        
        throw std::out_of_range("Key not found in HashMap");
    }

    V& at(const K& key) {
        const auto index = get_bucket_index(key);
        auto& bucket = buckets[index];
        
        for (auto& entry : bucket) {
            if (entry.key == key) {
                return entry.value;
            }
        }
        
        throw std::out_of_range("Key not found in HashMap");
    }

    bool contains_key(const K& key) const {
        const auto index = get_bucket_index(key);
        const auto& bucket = buckets[index];
        
        for (const auto& entry : bucket) {
            if (entry.key == key) {
                return true;
            }
        }
        
        return false;
    }

    std::optional<V> remove(const K& key) {
        const auto index = get_bucket_index(key);
        auto& bucket = buckets[index];
        
        for (auto it = bucket.begin(); it != bucket.end(); ++it) {
            if (it->key == key) {
                V value = std::move(it->value);
                bucket.erase(it);
                --size_count;
                return value;
            }
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
        for (auto& bucket : buckets) {
            bucket.clear();
        }
        size_count = 0;
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
        const auto index = get_bucket_index(key);
        auto& bucket = buckets[index];
        
        for (auto& entry : bucket) {
            if (entry.key == key) {
                return entry.value;
            }
        }

        insert(key, V{});
        
        const auto new_index = get_bucket_index(key);
        auto& new_bucket = buckets[new_index];
        
        for (auto& entry : new_bucket) {
            if (entry.key == key) {
                return entry.value;
            }
        }
        
        throw std::runtime_error("Internal error: failed to insert in operator[]");
    }
};
