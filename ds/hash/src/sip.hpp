#pragma once

#include "hasher.hpp"

template <Hashable T>
class Sip13Hasher : public Hasher<T> {
    uint64_t k0;
    uint64_t k1;

    static uint64_t rotl(uint64_t x, int b) {
        return (x << b) | (x >> (64 - b));
    }

    static void sipround(uint64_t* v0, uint64_t* v1, uint64_t* v2, uint64_t* v3) {
        *v0 += *v1;
        *v1 = rotl(*v1, 13);
        *v1 ^= *v0;
        *v0 = rotl(*v0, 32);

        *v2 += *v3;
        *v3 = rotl(*v3, 16);
        *v3 ^= *v2;

        *v0 += *v3;
        *v3 = rotl(*v3, 21);
        *v3 ^= *v0;

        *v2 += *v1;
        *v1 = rotl(*v1, 17);
        *v1 ^= *v2;
        *v2 = rotl(*v2, 32);
    }

public:
    Sip13Hasher(uint64_t k0 = 0x0706050403020100, uint64_t k1 = 0x0f0e0d0c0b0a0908)
        : k0(k0), k1(k1)
    {}

    uint64_t hash(const T& key) const override {
        const auto bytes = hash_bytes(key);
        auto len = bytes.size();
        
        uint64_t v0 = 0x736f6d6570736575 ^ k0;
        uint64_t v1 = 0x646f72616e646f6d ^ k1;
        uint64_t v2 = 0x6c7967656e657261 ^ k0;
        uint64_t v3 = 0x7465646279746573 ^ k1;

        const auto data = bytes.data();
        const auto chunks_end = data + len - (len % 8);

        for (const auto* p = data; p < chunks_end; p += 8) {
            uint64_t m = 0;
            for (auto i = 0; i < 8; i++) {
                m |= ((uint64_t)p[i]) << (i * 8);
            }
            v3 ^= m;
            sipround(&v0, &v1, &v2, &v3);
            v0 ^= m;
        }

        uint64_t b = ((uint64_t)len) << 56;
        const auto tail_start = len & (~7);
        switch (len & 7) {
            case 7: b |= ((uint64_t)data[tail_start + 6]) << 48; [[fallthrough]];
            case 6: b |= ((uint64_t)data[tail_start + 5]) << 40; [[fallthrough]];
            case 5: b |= ((uint64_t)data[tail_start + 4]) << 32; [[fallthrough]];
            case 4: b |= ((uint64_t)data[tail_start + 3]) << 24; [[fallthrough]];
            case 3: b |= ((uint64_t)data[tail_start + 2]) << 16; [[fallthrough]];
            case 2: b |= ((uint64_t)data[tail_start + 1]) << 8; [[fallthrough]];
            case 1: b |= ((uint64_t)data[tail_start + 0]); break;
            case 0: break;
        }

        v3 ^= b;
        sipround(&v0, &v1, &v2, &v3);
        v0 ^= b;

        v2 ^= 0xff;
        sipround(&v0, &v1, &v2, &v3);
        sipround(&v0, &v1, &v2, &v3);
        sipround(&v0, &v1, &v2, &v3);

        return v0 ^ v1 ^ v2 ^ v3;
    }
};
