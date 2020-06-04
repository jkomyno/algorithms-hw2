#ifndef SUM_WEIGHTS_H
#define SUM_WEIGHTS_H

#include <iterator>     // std::iterator_traits
#include <numeric>      // std::accumulate
#include <type_traits>  // std::enable_if, std::is_same

#include "Edge.h"

/**
 * Compute the sum of the weights of every edge in mst. It expects in input the
 * cbegin() and cend() iterator of the data structure holding the MST.
 */
template <typename It, typename = typename std::enable_if<std::is_same<
                           typename std::iterator_traits<It>::value_type, Edge>::value>::type>
int sum_weights(const It& cbegin, const It& cend) noexcept {
    return std::accumulate(cbegin, cend, 0, [](const int acc, const Edge& elem) {
        return acc + elem.weight;
    });
}

#endif  // SUM_WEIGHTS_H
