#pragma once

#include <algorithm>  // std::transform
#include <limits>     // std::numeric_limits
#include <vector>     // std::vector

#include "DistanceMatrix.h"
#include "Edge.h"
#include "PriorityQueue.h"

namespace mst {
    std::vector<Edge> prim_binary_heap_mst(const DistanceMatrix<int>& distance_matrix) noexcept {
        auto vertexes = distance_matrix.get_vertexes();

        const size_t n_stop = vertexes.size() - 1;
        std::vector<Edge> mst(n_stop);

        // Keys are used to pick the lightest edge in cut.
        // Initially, keys of all vertexes are set to infinity.
        constexpr int Infinity = std::numeric_limits<int>::max();
        std::vector<int> keys(vertexes.size(), Infinity);

        // the source vertex can be randomly chosen. For simplicity, we choose the first vertex
        // available. the first vertex is distant 0 from itself
        keys.at(0) = 0;

        // Priority Queue based on a Min Heap ordered by keys, from smaller to larger.
        // We don't need the O(N) first to reorder the vertexes as a heap, since keys is already
        // a valid heap. The priority queue creation thus takes O(1) time
        constexpr bool IsAlreadyHeap = true;
        auto min_pq(priority_queue::make_min_priority_queue<IsAlreadyHeap>(std::move(keys),
                                                                           std::move(vertexes)));

        // Loop until the the priority queue is empty or mst reached its maximum
        // size (n - 1 edges)
        while (!(min_pq.empty() && n_stop == mst.size())) {
            // u is the vertex with minimum key that belongs to the lightest edge of the cut
            auto u = min_pq.top();
            min_pq.pop();

            // traverse all vertexes which are adjacent to u
            for (const auto [v, weight] : distance_matrix.adjacent_vertexes(u)) {
                // if v is not in MST and w(u, v) is smaller than the current key of v
                if (min_pq.contains(v) && weight < min_pq.key_at(v)) {
                    // update the key associated with node v in O(logN), where N is the number
                    // of elements stored in the priority queue
                    min_pq.update_key(weight, v);

                    // add the edge (u, v) to the Minimum Spanning Tree
                    mst.at(v - 1) = Edge(u, v, weight);
                }
            }
        }

        // mst.erase(mst.begin());
        return mst;
    }
}  // namespace mst
