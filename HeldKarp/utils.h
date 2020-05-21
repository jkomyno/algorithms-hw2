#pragma once

#include <cmath>          // std::log2
#include <iterator>       // std::inserter
#include <unordered_set>  // std::unordered_set

namespace utils {
    using ull = unsigned long long;

    template <typename T>
    std::unordered_set<T> set_singleton_difference(const std::unordered_set<T>& original_set,
                                                   const T& v) {
        std::unordered_set<size_t> difference;
        std::copy_if(original_set.cbegin(), original_set.cend(),
                     std::inserter(difference, difference.end()), [v](const auto& u) {
                         return u != v;
                     });
        return difference;
    }

    inline void set_bit_in_position(ull& number, const size_t position) noexcept {
        number |= static_cast<ull>(1) << position;
    }

    inline ull toggle_bit(ull number, const size_t position) noexcept {
        return number ^ (static_cast<ull>(1) << position);
    }

    inline ull reset_bit(ull number, const size_t position) noexcept {
        return number &= ~(static_cast<ull>(1) << position);
    }

    template <typename It>
    ull set_bits_from_subset(const It& cbegin, const It& cend) noexcept {
        ull bits = 0;

        for (auto it = cbegin; it != cend; ++it) {
            const size_t position = *it;
            set_bit_in_position(bits, position);
        }

        return bits;
    }

    // return the position of the least significant bit (lsb).
    // E.g. the lsb in 12 (1100 in binary) is at position 2
    inline size_t get_least_significant_bit_pos(const ull number) noexcept {
        const auto n = static_cast<long long>(number);
        const auto tmp = n & (-n);
        return static_cast<size_t>(std::log2(tmp));
    }

    // helper used to traverse all the bits set to 1, from the least significant bit to the most
    // significant. visit is a callback that receives the currently visited bit and must return true
    // to continue the execution, or false to break the loop.
    template <typename Visit>
    void for_each(const ull bits, const size_t n, Visit&& visit) noexcept {
        bool keep_going = true;

        // start from the first bit set to 1 in bits
        size_t bit = get_least_significant_bit_pos(bits);
        for (; bit < n && keep_going; ++bit) {

            // avoid running the visit callback if the current bit is set to 0
            if ((bits & (static_cast<ull>(1) << bit)) != 0) {
                keep_going = visit(bit);
            }
        }
    }

    // returns true iff bit is the only bit set to one in number
    inline bool is_singleton(const ull number, const size_t bit) noexcept {
        return number == static_cast<ull>(1) << bit;
    }
}  // namespace utils
