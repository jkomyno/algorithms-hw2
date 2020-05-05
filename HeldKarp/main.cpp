#include <iostream>  // std::cout, std::endl

#include "DistanceMatrix.h"
#include "read_file.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(0);
    }

    const char* filename = argv[1];
    auto point_reader(read_file(filename));
    const auto distance_matrix = point_reader->create_distance_matrix();

    std::cout << distance_matrix << std::endl;

    std::cin.get();
}
