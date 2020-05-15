#pragma once

#include <algorithm>  // std::prev_permutation, std::fill
#include <cmath>      // std::log2
#include <vector>     // std::vector

namespace utils {
    using ull = unsigned long long;

    // return r-long subsequences of elements from the items in range [start, end - 1].
    // Combinations are emitted in lexicographic sort order.
    // There will be no repeat values in each combination.
    // It's the equivalent of Python's itertools.combinations
    inline std::vector<std::vector<size_t>> generate_combinations(size_t start, size_t end,
                                                                  size_t r) noexcept {
        // number of combinations
        const size_t n = end - start;

        // vector of combinations, each one with size r
        std::vector<std::vector<size_t>> combinations;

        // boolean vector used to obtain combinations from std::prev_permutation
        std::vector<bool> v(n);

        std::fill(v.begin(), v.begin() + r, true);

        do {
            std::vector<size_t> combination;
            combination.reserve(r);

            for (size_t i = 0; i < n; ++i) {
                if (v[i]) {
                    combination.push_back(i + start);
                }
            }

            combinations.emplace_back(std::move(combination));

        } while (std::prev_permutation(v.begin(), v.end()));

        return combinations;
    }

    inline void set_bit_in_position(ull& number, const size_t position) noexcept {
        number |= static_cast<ull>(1) << position;
    }

    inline ull toggle_bit(ull number, const size_t position) noexcept {
        return number ^ (static_cast<ull>(1) << position);
    }

    inline ull shift_one_by(const size_t n) noexcept {
        return static_cast<ull>(1) << n;
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

    // count the number of bits sets to 1 in number v (64 bits).
    // See: http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
    inline size_t pop_count(ull v) noexcept {
        static const int S[] = {1, 2, 4, 8, 16};
        static const int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF, 0x0000FFFF};

        size_t c = v - ((v >> 1) & B[0]);
        c = ((c >> S[1]) & B[1]) + (c & B[1]);
        c = ((c >> S[2]) + c) & B[2];
        c = ((c >> S[3]) + c) & B[3];
        c = ((c >> S[4]) + c) & B[4];

        return c;
    }
}  // namespace utils
