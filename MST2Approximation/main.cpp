#include <iostream>  // std::cout, std::endl

#include "DistanceMatrix.h"
#include "approx_tsp.h"
#include "read_file.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(0);
    }

    const char* filename = argv[1];

    // read the complete weighted undirected graph
    auto point_reader(read_file(filename));

    // create the symmetric distance matrix using either the Euclidean or Geodesic distances
    auto&& distance_matrix = point_reader->create_distance_matrix();

    // calculate the weight of the 2-approximate solution, which uses Prim's MST algorithm
    const auto total_weight = approx_tsp(std::forward<decltype(distance_matrix)>(distance_matrix));

    // use std::fixed to avoid displaying numbers in scientific notation
    std::cout << std::fixed << total_weight << std::endl;
}
