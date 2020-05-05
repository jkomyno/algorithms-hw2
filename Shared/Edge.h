#pragma once

/**
 * Edge represents a weighted edge between two vertexes.
 */
class Edge {
public:
    size_t from;
    size_t to;
    double weight;

    /**
     * Default constructor used to represent "null" values when the MST is initialized
     * in Prim's algorithm.
     */
    Edge() noexcept = default;

    explicit Edge(size_t from, size_t to, double weight) noexcept :
        from(from), to(to), weight(weight) {
    }

    Edge& operator=(const Edge& rhs) noexcept = default;

    /**
     * Default copy and move constructors, needed because in Prim's algorithm we need
     * to override edges
     */
    Edge(const Edge& rhs) noexcept = default;
    Edge(Edge&& rhs) noexcept = default;
};
