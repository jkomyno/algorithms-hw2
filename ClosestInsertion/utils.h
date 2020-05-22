#pragma once
#include <algorithm>      // std::max_element
#include <iterator>       // std::next, std::advance
#include <limits>         // std::numeric_limits
#include <list>           // std::list
#include <unordered_map>  // std::unordered_map
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

namespace utils {
    // return the vertex k that doesn't belong to the partial Hamiltonian circuit that
    // maximizes the distance δ(k, circuit).
    // get_distance is the distance function that computes the cost between 2 nodes.
    template <typename Distance>
    [[nodiscard]] size_t select_new_k(std::unordered_set<size_t>& not_visited,
                                      std::vector<size_t>& circuit,
                                      Distance&& get_distance) noexcept {
        // map that stores the maximum distance for each candidate vertex k
        std::unordered_map<size_t, double> node_max_weight_map;
        node_max_weight_map.reserve(not_visited.size());

        for (const auto k : not_visited) {
            double max_hk_weight = std::numeric_limits<double>::min();
            for (const auto h : circuit) {
                const double weight = get_distance(h, k);

                if (weight > max_hk_weight) {
                    max_hk_weight = weight;
                }
            }

            // update the map with the maximum weight between h and k found up to now
            node_max_weight_map[k] = max_hk_weight;
        }

        const auto max_map_comparator = [](const auto& x, const auto& y) {
            return x.second < y.second;
        };
        const auto it_new_k = std::max_element(node_max_weight_map.cbegin(),
                                               node_max_weight_map.cend(), max_map_comparator);

        // obtain the maximum of the maximum distances δ(k, circuit)
        const size_t new_k = it_new_k->first;
        return new_k;
    }

    // find the arc (i, j) that minimizes the value of w(i, k) - w(k, j) - w(i, j)
    // and add k between i and j in circuit.
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
        const int first_weight = utils::sum_weights_in_circuit(
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
            int weight = utils::sum_weights_in_circuit(circuit_insertion_list.cbegin(),
                                                       circuit_insertion_list.cend(), get_distance);

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
