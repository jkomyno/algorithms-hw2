#include <iostream>  // std::cout, std::endl

#include "DistanceMatrix.h"
#include "read_file.h"
// #include "HeldKarpBryan.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(0);
    }

    const char* filename = argv[1];
    auto point_reader(read_file(filename));
    auto distance_matrix = point_reader->create_distance_matrix();

    // HeldKarp held_karp(std::move(distance_matrix));
    // int total_weight = held_karp.execute();
    // std::cout << total_weight << std::endl;
}
