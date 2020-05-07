#pragma once
#include <DistanceMatrix.h>
#include <unordered_set>
#include "HKVectors.h"

template <typename T = int>
class HealdKarp {

    DistanceMatrix<T>& distance_matrix;

    HKVectors<T> hk_vectors;

    T heald_karp() {
        auto vertexes = distance_matrix.get_vertexes();

        // extract the first vertex, it doesn't matter which it is
        size_t init = vertexes.at(0);
        std::unordered_set<size_t> S(vertexes.begin(), vertexes.end());

        int distance = hk_tsp_rec(init, std::move(S));
        return distance;
    }

    T hk_tsp_rec(size_t v, std::unordered_set<size_t>&& S) {

        if (S.size() == 1 && S.count(v) == 1) {
            return distance_matrix.at(v, 0);
        } else if (hk_vectors.is_already_calculated(v, S)) {
            return hk_vectors.get_distance(v, S);
        } else {
            T min_distance = std::numeric_limits<T>::max();
            size_t min_prec = NULL;

            for (size_t u : S) {
                if (u != v) {
                    std::unordered_set<size_t> S_rec = S;
                    S_rec.erase(v);
                    T distance = hk_tsp_rec(u, std::move(S_rec));
                    if (distance + distance_matrix.at(u, v) < min_distance) {
                        min_distance = distance + distance_matrix.at(u, v);
                        min_prec = u;
                    }
                } 
            }

            hk_vectors.set_distance(v, S, min_distance);
            hk_vectors.set_predec(v, S, min_prec);

            return min_distance;
        }
    }



    public:
    explicit HealdKarp(DistanceMatrix<T>&& distance_matrix_ext) : distance_matrix(distance_matrix_ext) {
        
    }

    T execute() {
        return heald_karp();
    }
 
};


