#include <iostream>  // std::cout, std::endl

#include "DistanceMatrix.h"
#include "closest_insertion_tsp.h"
#include "read_file.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(0);
    }

    const char* filename = argv[1];
    auto point_reader(read_file(filename));
    auto distance_matrix = point_reader->create_distance_matrix();

    // calculate the weight of TSP with Farthest Insertion Heuristic
    const auto total_weight =
        closest_insertion_tsp(std::forward<decltype(distance_matrix)>(distance_matrix));

    // use std::fixed to avoid displaying numbers in scientific notation
    std::cout << std::fixed << total_weight << std::endl;
}
