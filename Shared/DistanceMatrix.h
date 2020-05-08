#pragma once

#include <algorithm>  // std::max_element
#include <cmath>      // std::sqrt
#include <iomanip>    // std::setw
#include <iostream>   // std::ostream
#include <numeric>    // std::iota
#include <vector>     // std::vector

#include "Edge.h"

/**
 * DistanceMatrix represents a distance matrix for a complete, weighted, undirected graph.
 * It's a symmetric matrix which main diagonal is filled with 0s.
 * It stores its values in a 1D vector.
 *
 * Note: although its main purpose is storing complete graphs, it may also be used to store
 * non complete graphs, such as MTSs. In that case, the absence of arcs is represented as
 * 0 distances.
 */
template <typename T = int>
class DistanceMatrix {
    size_t n_vertexes;
    std::vector<T> data;

    using ForwardIt = decltype(data.cbegin());

    // maps a matrix index pair to a vector index
    [[nodiscard]] size_t get_index(size_t row, size_t column) const {
        return row * n_vertexes + column;
    }

    // returns the begin and end iterator to the required row
    [[nodiscard]] std::pair<ForwardIt, ForwardIt> get_row_at(size_t row) const {
        const size_t begin = get_index(row, 0);
        const size_t end = get_index(row, this->size() - 1);
        const auto it_begin = data.cbegin() + begin;
        const auto it_end = data.cbegin() + end + 1;
        return {it_begin, it_end};
    }

    // given a matrix index, return the corresponding row and column indexes
    [[nodiscard]] std::pair<size_t, size_t> get_row_column_by_index(size_t mat_index) const {
        const size_t row = mat_index / n_vertexes;
        const size_t column = mat_index % n_vertexes;
        return {row, column};
    }

    // initialize the distance matrix according to the distance(i, j) function
    template <typename Distance>
    void init(const Distance& distance) noexcept {
        const size_t dimension = size();

        // the vector data is already filles with 0, so we don't have to
        // place 0s in the diagonal.

        // populate the triangle above the main diagonal
        for (size_t i = 0; i < dimension; ++i) {
            for (size_t j = i + 1; j < dimension; ++j) {
                at(i, j) = distance(i, j);
            }
        }

        // symmetrically copies the triangle above the main diagonal to the triangle
        // below the main diagonal
        for (size_t i = dimension - 1; i > 0; --i) {
            for (size_t j = 0; j < i; ++j) {
                at(i, j) = at(j, i);
            }
        }
    }

    void init_from_mst(const std::vector<Edge>& mst) noexcept {
        for (const auto& [i, j, weight] : mst) {
            at(i, j) = weight;
            at(j, i) = weight;
        }
    }

public:
    // create a new square matrix with n_vertexes rows initialized to all 0s.
    // distance(i, j) returns the distance between the i-th and j-th point.
    template <typename Distance>
    explicit DistanceMatrix(size_t n_vertexes, Distance&& distance) noexcept :
        n_vertexes(n_vertexes), data(n_vertexes * n_vertexes, 0) {
        init(std::forward<Distance>(distance));
    }

    // TODO: this should probably belong to an AdjacentList implementation
    explicit DistanceMatrix(std::vector<Edge>&& mst) noexcept :
        n_vertexes(mst.size() + 1), data(n_vertexes * n_vertexes, 0) {
        init_from_mst(std::forward<decltype(mst)>(mst));
    }

    // this constructor reads an arbitrary square matrix in input.
    // This can be useful for debugging purposes.
    // TODO: remove this constructor before submitting the project.
    DistanceMatrix(std::initializer_list<T>&& debug_matrix) noexcept :
        n_vertexes(static_cast<size_t>(std::sqrt(static_cast<double>(debug_matrix.size())))),
        data(std::move(debug_matrix)) {
    }

    // return number of rows/columns of the matrix
    [[nodiscard]] size_t size() const noexcept {
        return n_vertexes;
    }

    // retrieve the value saved at position (i, j)
    [[nodiscard]] const T& at(size_t i, size_t j) const {
        return data.at(get_index(i, j));
    }

    // set the value for position (i, j)
    [[nodiscard]] T& at(size_t i, size_t j) {
        return data.at(get_index(i, j));
    }

    // return the vertexes in the distance matrix
    [[nodiscard]] std::vector<size_t> get_vertexes() const {
        std::vector<size_t> v(n_vertexes);
        std::iota(v.begin(), v.end(), 0);
        return v;
    }

    // given a vertex i, return the vertexes adjacent to i.
    [[nodiscard]] std::vector<std::pair<size_t, int>> adjacent_vertexes(const size_t i) const {
        std::vector<std::pair<size_t, int>> adj_v;
        adj_v.reserve(n_vertexes - 1);

        for (size_t j = 0; j < n_vertexes; ++j) {
            const int weight = at(i, j);

            // a node j is adjacent to i if the weight of the arc (i, j) is different than 0
            if (weight != 0) {
                adj_v.emplace_back(j, weight);
            }
        }

        return adj_v;
    }

    // retrieves the node which is closest to the given node i
    [[nodiscard]] size_t get_closest_node(const size_t i) const {
        const auto& [it_begin, it_end] = get_row_at(i);

        T min = std::numeric_limits<T>::max();
        size_t min_index = 0;
        size_t index = 0;
        for (auto it = it_begin; it != it_end; ++it) {
            T curr_distance = *it;
            if (curr_distance != 0 && curr_distance < min) {
                min = curr_distance;
                min_index = index;
            }
            ++index;
        }

        return min_index;
    }

    // retrieves the node which is farthest to the given node i
    [[nodiscard]] size_t get_farthest_node(const size_t i) const {
        const auto& [it_begin, it_end] = get_row_at(i);
        const auto it_max = std::max_element(it_begin, it_end);
        const size_t max_index = std::distance(it_begin, it_max);
        return max_index;
    }

    // retrieves the 2 farthest nodes in the graph
    [[nodiscard]] std::pair<size_t, size_t> get_2_farthest_nodes() const {
        // TODO: we can optimize this operation by creating an upper-triangle iterator
        const auto it_max = std::max_element(data.cbegin(), data.cend());
        const size_t max_matrix_index = std::distance(data.cbegin(), it_max);
        return get_row_column_by_index(max_matrix_index);
    }

    // pretty-print distance matrix, useful for debugging/visualization purposes
    friend std::ostream& operator<<(std::ostream& os, const DistanceMatrix<T>& distance_matrix) {
        const auto width = 8;
        const size_t size = distance_matrix.size();

        for (size_t row = 0; row < size - 1; ++row) {
            for (size_t column = 0; column < size - 1; ++column) {
                os << std::setw(width) << distance_matrix.at(row, column) << ' ';
            }
            os << std::setw(width) << distance_matrix.at(row, size - 1) << std::endl;
        }
        for (size_t column = 0; column < size; ++column) {
            os << std::setw(width) << distance_matrix.at(size - 1, column) << ' ';
        }
        os << std::endl;
        return os;
    }
};
