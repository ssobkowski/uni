#pragma once

#include <concepts>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// implementation for trivially copyable types, such as ints
// that only require raw memory bytes
inline std::span<const uint8_t> hash_bytes(const auto& value)
    requires std::is_trivially_copyable_v<std::remove_cvref_t<decltype(value)>>
{
    return std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
}

// string specialization
template <typename S>
inline std::span<const uint8_t> hash_bytes(const S& value)
    requires requires { std::string_view(value); }
{
    std::string_view sv(value);
    return std::span<const uint8_t>(reinterpret_cast<const uint8_t*>(sv.data()), sv.size());
}

// specialization for vectors
template<typename T>
inline std::span<const uint8_t> hash_bytes(const std::vector<T>& v) 
    requires std::is_trivially_copyable_v<T>
{
    return std::span(reinterpret_cast<const uint8_t*>(v.data()), v.size() * sizeof(T));
}

// specialization for span
template<typename T>
inline std::span<const uint8_t> hash_bytes(const std::span<T>& s) {
    return s;
}


template <typename T>
concept Hashable = requires(const T& value) {
    { hash_bytes(value) } -> std::convertible_to<std::span<const uint8_t>>;
};

template <Hashable T>
class Hasher {
public:
    Hasher() = default;
    virtual uint64_t hash(const T& key) const = 0;
    virtual ~Hasher() = default;
};

template <typename H, typename T>
concept HasherType = std::derived_from<H, Hasher<T>> && requires(const H& hasher, const T& key) {
    { hasher.hash(key) } -> std::convertible_to<uint64_t>;
};

template <typename K>
concept HashMapKey = Hashable<K> && std::equality_comparable<K>;

