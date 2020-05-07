#include <iostream>  // std::cout, std::endl

#include "DistanceMatrix.h"
#include "held_karp.h"
#include "read_file.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(0);
    }

    // read the graph and create the distance matrix
    const char* filename = argv[1];
    auto point_reader(read_file(filename));
    const auto distance_matrix = point_reader->create_distance_matrix();

    // calculate the weight held & kart
    const auto total_weight =
        held_karp_tsp(std::forward<decltype(distance_matrix)>(distance_matrix));

    // use std::fixed to avoid displaying numbers in scientific notation
    std::cout << std::fixed << total_weight << std::endl;
}
