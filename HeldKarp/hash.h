#pragma once

#include <functional>     // std::hash
#include <unordered_set>  // std::unordered_set
#include <utility>        // std::pair
#include "DynamicBitMasking.h"

/**
 * Hash functors for custom types
 */
namespace hash {
    template <class T>
    void hash_combine(size_t& seed, const T& v) noexcept {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    // commutative hash functor for std::pair
    struct pair {
        template <typename First, typename Second>
        size_t operator()(const std::pair<First, Second>& pair) const noexcept {
            size_t hash = 0;
            hash_combine(hash, pair.first);
            hash_combine(hash, pair.second);
            return hash;
        }
    };

    // commutative has functor for std::unordered_set.
    // The sets it is applied to are considered immutable.
    // It has been inspired by CPython's frozenset hash function.
    // See: https://stackoverflow.com/a/20833285/6174476
    struct unordered_set {
        template <typename T>
        size_t operator()(const std::unordered_set<T>& set) const noexcept {
            constexpr size_t MAX = std::numeric_limits<size_t>::max();
            constexpr size_t MASK = 2 * MAX + 1;
            const size_t n = set.size();

            size_t hash = 1927868237UL * (n + 1);
            hash &= MASK;

            for (const auto x : set) {
                std::hash<T> hasher;
                const size_t hx = hasher(x);

                // the xor-equal is necessary to make the hash algorithm commutative.
                // 89869747 has an interesting bit pattern that is used to break up sequences of
                // nearby hash values 3644798167 is a randomly chosen large prime number with
                // another interesting bit pattern. (hx << 16) gives two chances to the lower bits
                // to affect the outcome, resulting in better dispersion of nearby hash values.
                hash ^= (hx ^ (hx << 16) ^ 89869747UL) * 3644798167UL;
                hash &= MASK;
            }

            // 69069 is related to linear congruential random number generators
            hash = hash * 69069U + 907133923;
            hash &= MASK;

            if (hash > MAX) {
                hash -= MASK + 1;
            }

            return hash;
        }
    };

    // commutative has functor for DynamicBitMasking.
    // The object it is applied to is considered immutable.
    // It has been inspired by CPython's frozenset hash function.
    // See: https://stackoverflow.com/a/20833285/6174476
    struct DynamicBitMasking {
        size_t operator()(const ::DynamicBitMasking& bitset) const noexcept {
            constexpr size_t MAX = std::numeric_limits<size_t>::max();
            constexpr size_t MASK = 2 * MAX + 1;
            const size_t n = bitset.size();

            size_t hash = 1927868237UL * (n + 1);
            hash &= MASK;

            for (const auto& x : bitset) {
                std::hash<unsigned long long> hasher;
                const size_t hx = hasher(x);

                // the xor-equal is necessary to make the hash algorithm commutative.
                // 89869747 has an interesting bit pattern that is used to break up sequences of
                // nearby hash values 3644798167 is a randomly chosen large prime number with
                // another interesting bit pattern. (hx << 16) gives two chances to the lower bits
                // to affect the outcome, resulting in better dispersion of nearby hash values.
                hash ^= (hx ^ (hx << 16) ^ 89869747UL) * 3644798167UL;
                hash &= MASK;
            }

            // 69069 is related to linear congruential random number generators
            hash = hash * 69069U + 907133923;
            hash &= MASK;

            if (hash > MAX) {
                hash -= MASK + 1;
            }

            return hash;
        }
    };
}  // namespace hash


namespace std {
    // we need to defined std::hash for generic std::unordered_set so that the custom hash:pair
    // hash function works properly when a std::pair<std::unordered_set<T1>, T2> is used as a key
    // in a std::unordered_map.
    template <typename T>
    struct hash<std::unordered_set<T>> {
        size_t operator()(const std::unordered_set<T>& set) const noexcept {
            return ::hash::unordered_set{}(set);
        }
    };

    // we need to defined std::hash for DynamicBitMasking so that the custom hash:pair
    // hash function works properly when a std::pair<DynamicBitMasking, T> is used as a key
    // in a std::unordered_map.
    template <>
    struct hash<DynamicBitMasking> {
        size_t operator()(const DynamicBitMasking& bitset) const noexcept {
            return ::hash::DynamicBitMasking{}(bitset);
        }
    };
}  // namespace std
