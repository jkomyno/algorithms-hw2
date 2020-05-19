#pragma once

#include <unordered_map>  // std::unordered_map
#include <unordered_set>  // std::unordered_set
#include <vector>         // std::vector

#include "DistanceMatrix.h"
#include "hash.h"
#include "timeout.h"
#include "utils.h"

// type of the Dynamic Programming structure that holds the intermediate distances.
// This is what makes Held & Karp algorithm a better algorithm than the brute force approach
// (which would be O(n!))
using held_karp_dp_bits_t = std::unordered_map<std::pair<utils::ull, size_t>, int, hash::pair>;

using held_karp_dp_t =
    std::unordered_map<std::pair<std::unordered_set<size_t>, size_t>, int, hash::pair>;

/**
 * @param signal is the timeout signal. When signal.is_expired() is true, the recursion stops and
 * the local minimum is returned.
 * @param distance_matrix represents the graph as a Distance Matrix.
 * @param C is the dynamic programming map that keeps tracks of the possible subpaths.
 * @param bits is the subpath represented as a 64-bit number.
 * @param v is the node in the subpath currently considered. It's initially set to 0.
 */
int held_karp_tsp_rec_bits_helper(timeout::timeout_signal& signal,
                                  DistanceMatrix<int>& distance_matrix, held_karp_dp_bits_t& C,
                                  utils::ull bits, size_t v = 0) {
    // there's only one node in the subpath. Break the recursion and return w(v, 0).
    if (utils::is_singleton(bits, v)) {
        return distance_matrix.at(v, 0);
    }

    // the weight of the subpath starting from node 0 to node v has already been computed. Return
    // it.
    if (C.count({bits, v})) {
        return C[{bits, v}];
    }

    int min_dist = std::numeric_limits<int>::max();
    const utils::ull difference = utils::toggle_bit(bits, v);
    const size_t n = distance_matrix.size();

    // iterate over any bit set to 1 in differences, from the least significant to the most
    // significant bit
    utils::for_each(difference, n, [&](const size_t bit) {
        int dist = held_karp_tsp_rec_bits_helper(signal, distance_matrix, C, difference, bit);
        int tmp_dist = dist + distance_matrix.at(v, bit);

        if (tmp_dist < min_dist) {
            min_dist = tmp_dist;
        }

        // return true to continue, return false to break the loop
        return !signal.is_expired();
    });

    C[{bits, v}] = min_dist;
    return min_dist;
}

/**
 * @param signal is the timeout signal. When signal.is_expired() is true, the recursion stops and
 * the local minimum is returned.
 * @param distance_matrix represents the graph as a Distance Matrix.
 * @param C is the dynamic programming map that keeps tracks of the possible subpaths.
 * @param subset is the std::unordered_set of nodes in the path currently considered.
 * @param v is the node in the subpath currently considered. It's initially set to 0.
 */
int held_karp_tsp_rec_helper(timeout::timeout_signal& signal, DistanceMatrix<int>& distance_matrix,
                             held_karp_dp_t& C, const std::unordered_set<size_t>& subset,
                             size_t v = 0) {
    // there's only one node in the subpath. Break the recursion and return w(v, 0).
    if (std::unordered_set({v}) == subset) {
        return distance_matrix.at(v, 0);
    }

    // the weight of the subpath starting from node 0 to node v has already been computed. Return
    // it.
    if (C.count({subset, v})) {
        return C[{subset, v}];
    }

    int min_dist = std::numeric_limits<int>::max();

    // difference = subset \ {v}.
    std::unordered_set<size_t> difference = utils::set_singleton_difference(subset, v);

    for (const size_t u : difference) {
        int dist = held_karp_tsp_rec_helper(signal, distance_matrix, C, difference, u);
        int tmp_dist = dist + distance_matrix.at(v, u);

        if (tmp_dist < min_dist) {
            min_dist = tmp_dist;
        }

        // if the time is expired, we break the loop, unroll the recursion and return the best solution
        // found up to now.
        if (signal.is_expired()) {
            break;
        }
    }

    C[{subset, v}] = min_dist;
    return min_dist;
}

/**
 * Recursive Held Karp implementation. Subpaths are represented as a 64-bit number where bits set to
 * 1 represent nodes in the path.
 * As soon as the timeout expires, the recursive steps are halted and the subpath that crosses every
 * node starting from 0 with the minimum distance up to that moment is returned.
 * This function only returns the weight of the optimal Hamiltonian Cycle. The actual path isn't
 * computed.
 * Time: O(2^n * n^2)
 * Space: O(2^n * n)
 */
inline int held_karp_tsp_rec(timeout::timeout_signal&& signal,
                             DistanceMatrix<int>&& distance_matrix) {
    constexpr unsigned char BITSET_TRESHOLD = 63;
    const size_t size = distance_matrix.size();
    const auto vertexes = distance_matrix.get_vertexes();

    if (size < BITSET_TRESHOLD) {
        // C is the map that keeps track of subpaths, the weight of each subpath and the head node
        // of
        // said subpath, assuming that the path starts from node 0.
        // The key of the map is in the form (subpath, head node).
        // The value of the map is an integer representing the weight of the subpath.
        // Subpaths are represented as a 64-bit binary number, where the bits set to 1
        // represent the nodes that are in the path. The i-th bit from the least significant bit to
        // the most significant represents node (i - 1).
        //
        // Example: suppose that a considered subpath crosses nodes 0, 1, and 3. The binary
        // representation of such path is thus b1011 (11 in decimal).
        // Let the subpath have weight 15 and let the head node of the path be node 3.
        // This map entry would then be represented as ((b1011, 3) -> 15).
        held_karp_dp_bits_t C;
        utils::ull bits = utils::set_bits_from_subset(vertexes.cbegin(), vertexes.cend());
        return held_karp_tsp_rec_bits_helper(signal, distance_matrix, C, bits);
    }

    // general, slower case with more than 63 nodes
    held_karp_dp_t C;

    // subset initially contains every node in the graph
    std::unordered_set<size_t> subset(vertexes.cbegin(), vertexes.cend());
    return held_karp_tsp_rec_helper(signal, distance_matrix, C, subset);
}








/**
 * The iterative version down here is just for reference and shouldn't be used, because it doesn't
 * allow the use of timeout signals.
 */

inline void clear_old_dp_entries(held_karp_dp_bits_t& C, const size_t threshold) noexcept {
    auto it = C.begin();
    while (it != C.end()) {
        const auto entry = *it;
        const utils::ull bits = entry.first.first;
        if (utils::pop_count(bits) < threshold) {
            it = C.erase(it);
        } else {
            ++it;
        }
    }
}

// Iterative Held Karp implementation with manual bit sets.
// Time: O(2^n * n^2)
// Space: O(2^n * sqrt(n))
inline int held_karp_tsp(DistanceMatrix<int>&& distance_matrix) {
    using ull = unsigned long long;
    const size_t n = distance_matrix.size();

    // map each subset of the nodes to the cost needed to reach that subset,
    // as well as the node crossed immediately before reaching that subset.
    // Node subsets are represented as set bits
    held_karp_dp_bits_t C;

    // base case: set the cost from the initial state 0
    for (size_t k = 1; k < n; ++k) {
        const std::pair<ull, size_t> key{utils::shift_one_by(k), k};
        C[key] = distance_matrix.at(0, k);
    }

    // iterate over subsets of increasing length, storing intermediate results
    for (size_t subset_size = 2; subset_size < n; ++subset_size) {
        for (const auto& subset : utils::generate_combinations(1, n, subset_size)) {

            // set bits for every node in this subset
            ull bits = utils::set_bits_from_subset(subset.cbegin(), subset.cend());

            // find the lowest cost to get to the current subset
            for (const size_t k : subset) {
                ull prev = utils::toggle_bit(bits, k);

                std::vector<int> res;
                res.reserve(subset_size - 1);

                for (const size_t m : subset) {
                    // avoid comparing a node with itself or with the initial node
                    if (!(m == 0 || m == k)) {
                        const int dist_mk = distance_matrix.at(m, k);
                        res.emplace_back(C[{prev, m}] + dist_mk);
                    }
                }

                const auto it_min_value = std::min_element(res.cbegin(), res.cend());
                C[{bits, k}] = *it_min_value;
            }
        }

        // removes all the entries in C where the number of bits sets to 1 (i.e., the number of
        // nodes used in the partial path is less than subset_size-1)
        clear_old_dp_entries(C, subset_size - 1);
    }

    // we're interested in every bit, but the least significant,
    // which is the start state
    ull bits = (utils::shift_one_by(n) - 1) - 1;

    // calculate the optimal cost
    std::vector<int> res;
    for (size_t k = 1; k < n; ++k) {
        const int dist_k0 = distance_matrix.at(k, 0);
        res.emplace_back(C[{bits, k}] + dist_k0);
    }

    const auto it_min_value = std::min_element(res.cbegin(), res.cend());
    return *it_min_value;
}
