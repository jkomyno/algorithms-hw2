#pragma once

#include <algorithm>      // std::generate_n, std::min_element
#include <cmath>          // std::floor
#include <iterator>       // std::inserter
#include <unordered_set>  // std::unordered_set

namespace utils {
    // determine length of string at compile time
    template <size_t N>
    constexpr auto string_length(char const (&)[N]) {
        return N;
    }

    // fun fact, the constant value of π is only available in C++20, so we have to define it
    // manually in C++17
    template <typename T>
    constexpr T pi = T(3.141592);

    // converts x to radians
    [[nodiscard]] inline double to_radians(const double x) noexcept {
        // deg is the integer part of x
        const long deg = static_cast<long>(std::trunc(x));
        const double min = x - deg;
        return pi<double> * (deg + 5.0 * min / 3.0) / 180.0;
    }

    // generate an unordered set with integer values in range [0, n-1]
    [[nodiscard]] inline std::unordered_set<size_t> generate_range_set(size_t n) noexcept {
        std::unordered_set<size_t> set;
        set.reserve(n);

        std::generate_n(std::inserter(set, set.end()), n, [&set] {
            return set.size();
        });

        return set;
    }

    // compute the total weight of the circuit defined by the two iterators cbegin and cend.
    // For example, if the iterators represent a container {0,3,4,1}, and we call w the distance
    // function, it computes the cost as:
    // w(0,3) + w(3,4) + w(4,1) + w(1,0)
    //
    // cbegin and cend are the iterators of the container used to store the circuit.
    // get_distance is the distance function that computes the cost between 2 nodes.
    template <typename It, typename Distance>
    [[nodiscard]] int sum_weights_in_circuit(const It& cbegin, const It& cend,
                                             Distance&& get_distance) {
        int total_weight = 0;
        auto it_prev = cbegin;

        for (auto it_curr = std::next(cbegin, 1); it_curr != cend; ++it_curr) {
            int weight = get_distance(*it_curr, *it_prev);
            total_weight += weight;
            ++it_prev;
        }

        total_weight += get_distance(*cbegin, *it_prev);
        return total_weight;
    }

    /**
     * Return the best solution from a set of multiple solutions.
     * This std::min_element wrapper is necessary to force the compiler to choose the right overload
     * of a higher-order function at compile-time
     *
     * @see https://stackoverflow.com/a/36794145/6174476
     */
    const auto select_best = [](auto&&... args) -> decltype(auto) {
        return std::min_element(std::forward<decltype(args)>(args)...);
    };

    /**
     * Return the best solution from a set of multiple solutions.
     * The result is a std::pair containing the result as first term and its cost as second term
     */
    const auto select_best_result_cost_pair = [](auto&&... args) -> decltype(auto) {
        return select_best(args..., [](const auto& x, const auto& y) {
            return x.second < y.second;
        });
    };
}  // namespace utils
