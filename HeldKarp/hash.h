#pragma once

#include <utility>  // std:: pair

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
    struct bitset_node_pair_hash {
        template <typename First, typename Second>
        size_t operator()(const std::pair<First, Second>& pair) const noexcept {
            size_t hash = 0;
            hash_combine(hash, pair.first);
            hash_combine(hash, pair.second);
            return hash;
        }
    };
}  // namespace hash
