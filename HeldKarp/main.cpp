#include <iostream>  // std::cout, std::endl

#include "DistanceMatrix.h"
#include "read_file.h"
#include "HealdKarp.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(0);
    }

    const char* filename = argv[1];
    auto point_reader(read_file(filename));
    auto distance_matrix = point_reader->create_distance_matrix();

    HealdKarp heald_karp(std::move(distance_matrix));
    double distance = heald_karp.execute();

    std::cout << distance << std::endl;

    std::cin.get();
}
