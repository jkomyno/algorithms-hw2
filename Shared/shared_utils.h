#pragma once

#include <algorithm>      // std::generate_n, std::min_element, std::max_element
#include <cmath>          // std::floor
#include <iterator>       // std::inserter
#include <iterator>       // std::next, std::advance
#include <limits>         // std::numeric_limits
#include <list>           // std::list
#include <unordered_map>  // std::unordered_map
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

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
    [[nodiscard]] int sum_weights_as_circuit(const It& cbegin, const It& cend,
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
     * Return the minimum element in a list of elements.
     * This std::min_element wrapper is necessary to force the compiler to choose the right overload
     * of a higher-order function at compile-time
     *
     * @see https://stackoverflow.com/a/36794145/6174476
     */
    const auto min_element = [](auto&&... args) -> decltype(auto) {
        return std::min_element(std::forward<decltype(args)>(args)...);
    };

    /**
     * Return the maximum element in a list of elements.
     * This std::max_element wrapper is necessary to force the compiler to choose the right overload
     * of a higher-order function at compile-time
     *
     * @see https://stackoverflow.com/a/36794145/6174476
     */
    const auto max_element = [](auto&&... args) -> decltype(auto) {
        return std::max_element(std::forward<decltype(args)>(args)...);
    };

    /**
     * Return the best solution from a set of multiple solutions.
     * The result is a std::pair containing the result as first term and its cost as second term
     */
    const auto select_best_result_cost_pair = [](auto&&... args) -> decltype(auto) {
        return min_element(args..., [](const auto& x, const auto& y) {
            return x.second < y.second;
        });
    };

    // return the vertex k that doesn't belong to the partial Hamiltonian circuit that
    // maximizes or minimizes the distance δ(k, circuit) w.r.t. get_best_k.
    // get_distance is the distance function that computes the cost between 2 nodes.
    template <class GetBestK, typename Distance>
    [[nodiscard]] size_t select_new_k(std::unordered_set<size_t>& not_visited,
                                      std::vector<size_t>& circuit, Distance&& get_distance,
                                      GetBestK&& get_best_k) noexcept {
        // map that stores the minimum distance for each candidate vertex k
        std::unordered_map<size_t, double> node_min_weight_map;
        node_min_weight_map.reserve(not_visited.size());

        for (const auto k : not_visited) {
            double min_hk_weight = std::numeric_limits<double>::max();
            for (const auto h : circuit) {
                const double weight = get_distance(h, k);

                if (weight < min_hk_weight) {
                    min_hk_weight = weight;
                }
            }

            // update the map with the minimum weight between h and k found up to now
            node_min_weight_map[k] = min_hk_weight;
        }

        // both std::min_element and std::max_element need a comparator that returns true when x < y
        auto map_comparator = [](const auto& x, const auto& y) {
            return x.second < y.second;
        };

        // maximize or minimize distances based on comparator
        const auto it_new_k = get_best_k(node_min_weight_map.cbegin(), node_min_weight_map.cend(),
                                         std::move(map_comparator));

        // obtain the maximum of the maximum or minimum distances δ(k, circuit)
        const size_t new_k = it_new_k->first;
        return new_k;
    }

    // return the vertex k that doesn't belong to the partial Hamiltonian circuit that
    // maximizes the distance δ(k, circuit).
    // get_distance is the distance function that computes the cost between 2 nodes.
    template <typename Distance>
    [[nodiscard]] size_t select_new_k_maximize(std::unordered_set<size_t>& not_visited,
                                               std::vector<size_t>& circuit,
                                               Distance&& get_distance) noexcept {
        return select_new_k(not_visited, circuit, get_distance, utils::max_element);
    }

    // return the vertex k that doesn't belong to the partial Hamiltonian circuit that
    // minimizes the distance δ(k, circuit).
    // get_distance is the distance function that computes the cost between 2 nodes.
    template <typename Distance>
    [[nodiscard]] size_t select_new_k_minimize(std::unordered_set<size_t>& not_visited,
                                               std::vector<size_t>& circuit,
                                               Distance&& get_distance) noexcept {
        return select_new_k(not_visited, circuit, get_distance, utils::min_element);
    }

    // find the arc (i, j) that minimizes the value of w(i, k) - w(k, j) - w(i, j)
    // and add k in between i and j in circuit.
    template <typename Distance>
    void perform_best_circuit_insertion(const size_t k, std::vector<size_t>& circuit,
                                        Distance&& get_distance) {
        const size_t size = circuit.size();

        // circuit_combinations will be sequentially modified to represent all possible insertions
        // of the vertex k between the vertexes in circuit. We prefer this mutable approach rather
        // than first enumerating all possible insertions and then selecting the best to keep the
        // space complexity at O(n) instead of O(n^2).
        // Initially, it is an ordered copy of the given circuit.
        // We use std::list because insertions and deletions at any point are O(1).
        std::list<size_t> circuit_insertion_list(circuit.cbegin(), circuit.cend());

        // compute the first insertion possibility's weight.
        // For example, suppose that circuit is {3,4,1} and that k = 0.
        // The possible insertions are {0,3,4,1}, {3,0,4,1}, and {3,4,0,1}.
        // The following block sets circuit_insertion_list to the first possible insertion
        // ({0,3,4,1}, according to the example) and saves its circuit weight, i.e.
        // w(0,3) + w(3,4) + w(4,1) + w(1,0).
        auto it_list = circuit_insertion_list.begin();
        it_list = circuit_insertion_list.insert(it_list, k);
        const int first_weight = utils::sum_weights_as_circuit(
            circuit_insertion_list.cbegin(), circuit_insertion_list.cend(), get_distance);

        // keep track of the minimum weight
        int min_weight = first_weight;

        // keep track of the index of the list where the circuit weight is minimized
        size_t index_min_weight = 0;

        // compute the other possible insertions removing k from circuit_insertion_list and adding
        // it again in the next suitable position. Removals and insertions are O(1) because we're
        // using lists. Since the total possible insertions in a list with n elements is n and we
        // already computed the first possible insertion, we need only n-2 other iterations.
        for (size_t i = 1; i < size; ++i) {
            it_list = circuit_insertion_list.erase(it_list);
            it_list = circuit_insertion_list.insert(std::next(it_list), k);
            const int weight = utils::sum_weights_as_circuit(
                circuit_insertion_list.cbegin(), circuit_insertion_list.cend(), get_distance);

            if (weight < min_weight) {
                min_weight = weight;
                index_min_weight = i;
            }
        }

        // add k to the place where the circuit weight is minimized.
        // This operation takes O(n), because circuit is a vector.
        const auto it_insert = std::next(circuit.begin(), index_min_weight);
        circuit.insert(it_insert, k);
    }
}  // namespace utils
