#pragma once

#include <iomanip>   // std::setw
#include <iostream>  // std::ostream
#include <numeric>   // std::iota
#include <vector>    // std::vector

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
template <typename T = double>
class DistanceMatrix {
    size_t n_vertexes;
    std::vector<T> data;

    // maps a matrix index pair to a vector index
    [[nodiscard]] size_t get_index(size_t row, size_t column) const {
        return row * n_vertexes + column;
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

    explicit DistanceMatrix(std::vector<Edge>&& mst) noexcept :
        n_vertexes(mst.size() + 1), data(n_vertexes * n_vertexes, 0) {
        init_from_mst(std::move(mst));
    }

    explicit DistanceMatrix(const std::vector<Edge>& mst) noexcept :
        n_vertexes(mst.size() + 1), data(n_vertexes * n_vertexes, 0) {
        init_from_mst(mst);
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
    [[nodiscard]] std::vector<std::pair<size_t, double>> adjacent_vertexes(const size_t i) const {
        std::vector<std::pair<size_t, double>> adj_v;
        adj_v.reserve(n_vertexes - 1);

        for (size_t j = 0; j < n_vertexes; ++j) {
            const double weight = at(i, j);

            // a node j is adjacent to i if the weight of the arc (i, j) is different than 0
            if (weight != 0) {
                adj_v.emplace_back(j, weight);
            }
        }

        return adj_v;
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
