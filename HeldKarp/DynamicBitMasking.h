#pragma once
#include <cmath>     // std::log2
#include <iterator>  // std::forward_iterator_tag
#include <vector>    // std::vector

class DynamicBitMasking {
    using ull = unsigned long long;
    constexpr static ull ONE = static_cast<ull>(1);

    std::vector<ull> bits;

    // quotient of the division by 64
    constexpr static size_t get_index_of_vector(const size_t position) noexcept {
        // 6 is representing 2^6 = 64
        return position >> 6;
    }

    // remainder of the division by 64
    constexpr static size_t get_index_bit_in_entry(const size_t position) noexcept {
        return position & 63;
    }

public:
    // const forward iterator for class DynamicBitMasking
    class const_iterator {
        using vector_cit = std::vector<ull>::const_iterator;
        vector_cit ptr;

    public:
        // iterator traits
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        using value_type = ull;
        using pointer = const ull*;
        using reference = const ull&;

        explicit const_iterator(vector_cit ptr) : ptr(std::move(ptr)) {
        }

        reference operator*() const {
            return *ptr;
        }

        const_iterator& operator++() {
            ++ptr;
            return *this;
        }

        const_iterator& operator--() {
            --ptr;
            return *this;
        }

        bool operator==(const const_iterator& rhs) const {
            return ptr == rhs.ptr;
        }

        bool operator!=(const const_iterator& rhs) const {
            return ptr != rhs.ptr;
        }
    };

    template <typename It>
    DynamicBitMasking(const It& cbegin, const It& cend) {
        for (auto it = cbegin; it != cend; ++it) {
            const size_t position = *it;
            set_bit_in_position(position);
        }
    }

    // equality operator
    bool operator==(const DynamicBitMasking& other) const noexcept {
        return bits == other.bits;
    }

    [[nodiscard]] size_t size() const {
        return bits.size() * 64;
    }

    void reset_bit_in_position(const size_t position) {
        const size_t index = get_index_of_vector(position);
        const size_t bit_pos = get_index_bit_in_entry(position);

        if (index < bits.size()) {
            bits[index] = bits[index] &= ~(ONE << bit_pos);
        }
    }

    void set_bit_in_position(const size_t position) {
        const size_t index = get_index_of_vector(position);
        const size_t bit_pos = get_index_bit_in_entry(position);

        size_t difference = 1 + index - bits.size();

        if (difference > 0) {
            // index is too high for the current capacity so we have to create the entry in the
            // vector
            while (difference != 0) {
                bits.emplace_back(0);
                difference--;
            }
        }

        bits[index] |= ONE << bit_pos;
    }

    [[nodiscard]] bool is_singleton(const size_t position) const {
        const size_t index = get_index_of_vector(position);
        const size_t bit_pos = get_index_bit_in_entry(position);

        bool result = false;

        for (size_t i = 0; i < bits.size(); ++i) {
            if (i == index) {
                result = (bits[i] == (ONE << bit_pos));

                // optimization: if the result in the entry is already false we can exit the
                // function
                if (!result) {
                    return false;
                }
            }
            // if in other entries there is at least 1 bit set to 1 the result is false
            else if (bits[i] != 0) {
                return false;
            }
        }

        return result;
    }

    // return the position of the least significant bit
    [[nodiscard]] size_t get_lsb_position() const {
        for (size_t i = 0; i < bits.size(); ++i) {
            // notice that n is signed, so negation is well-defined
            const auto n = static_cast<long long>(bits[i]);
            const auto tmp = n & (-n);

            if (tmp != 0) {
                return (i * 64) + static_cast<size_t>(std::log2(tmp));
            }
        }

        return 0;
    }

    [[nodiscard]] bool at(const size_t position) const {
        const size_t index = get_index_of_vector(position);
        const size_t bit_pos = get_index_bit_in_entry(position);

        if (index < bits.size()) {
            return bits[index] & (ONE << bit_pos);
        }

        // if the index is out of bound return false
        return false;
    }

    [[nodiscard]] const_iterator begin() const noexcept {
        return const_iterator(std::cbegin(bits));
    }

    [[nodiscard]] const_iterator end() const noexcept {
        return const_iterator(std::end(bits));
    }
};
