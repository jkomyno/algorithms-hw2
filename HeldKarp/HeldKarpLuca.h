#pragma once

#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "DistanceMatrix.h"

// Hasher based on pairs of vertexes and sets.
struct hash_set {
    template <class T>
    size_t operator()(const std::pair<size_t, std::unordered_set<T>>& k) const {

        size_t v = k.first;
        std::unordered_set<T> set = k.second;

        size_t hash1 = 0;

        for (size_t s : set) {
            hash1 = hash1 ^ std::hash<T>{}(s);
        }

        return hash1 ^ v;
    }
};

[[nodiscard]] inline double held_karp_tsp(const DistanceMatrix<double>&& distance_matrix) noexcept {
    // The distances map. The key is pair `(v, S)` where `v` node and `S` is a set of nodes,
    // while the value `d` is the distance between the root of the tsp and `v`
    // passing through the nodes in `S`.
    // Shortly: (v, S) ->  d
    std::unordered_map<std::pair<size_t, std::unordered_set<size_t>>, double, hash_set> d;

    // The same as before, but the value `u` is the prev node in the path.
    // Shortly: (v, S) -> u
    std::unordered_map<std::pair<size_t, std::unordered_set<size_t>>, size_t, hash_set> pi;

    const auto vertexes = distance_matrix.get_vertexes();

    // The root of tsp.
    size_t init = vertexes.at(0);
    // The set of nodes to pass through.
    std::unordered_set<size_t> S(vertexes.begin(), vertexes.end());

    // The Held & Karp algorithm.
    // Compute the min weight for the path from `init` to `v` that passes through all vertexes in S.
    std::function<double(size_t, std::unordered_set<size_t>)> hk_visit =
        [&distance_matrix, &d, &pi, &hk_visit](size_t v, std::unordered_set<size_t>&& S) -> double {
        if (S.count(v) == 1 && S.size() == 1) {  // base case
            return distance_matrix.at(v, 0);
        }

        if (d.find(std::make_pair(v, S)) != d.cend()) {  // distance already computed
            return d.at(std::make_pair(v, S));
        }

        double min_distance = std::numeric_limits<double>::max();
        size_t min_prec = 0;

        std::unordered_set<size_t> S1 = S;  // TODO: try to avoid copies.
        S1.erase(v);

        for (size_t u : S1) {
            // TODO: try to avoid copies.
            double distance = hk_visit(u, std::move(std::unordered_set<size_t>(S1)));

            if (distance + distance_matrix.at(u, v) < min_distance) {  // new minimum found
                min_distance = distance + distance_matrix.at(u, v);
                min_prec = u;
            }
        }

        // Updating distances and parents map.
        auto p = std::make_pair(v, S);

        d[p] = min_distance;
        pi[p] = min_prec;

        return min_distance;
    };

    return hk_visit(init, std::move(S));
}
