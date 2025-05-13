#pragma once

#include <concepts>
#include <string>
#include <random>
#include <functional>

template<typename T>
concept NonVoidType = !std::is_void_v<T>;

template <NonVoidType T, typename P = int>
struct ValueWithPriority {
    T value;
    P priority;
};

template <typename F, typename Arg>
concept Predicate = requires(F f, Arg arg) {
    { f(arg) } -> std::convertible_to<bool>;
};

namespace util {
    constexpr unsigned int SEED = 280131;

    static std::mt19937 seeded_engine(const std::string& label, const size_t sz) {
        std::seed_seq seq {
            std::hash<std::string>{}(label),
            sz
        };
        return std::mt19937(seq);
    }
    static std::mt19937 random_seeded_engine = seeded_engine("static random", 280131);

    static int random_int(
        const int min,
        const int max,
        std::mt19937& engine = random_seeded_engine
    ) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(engine);
    }
}
