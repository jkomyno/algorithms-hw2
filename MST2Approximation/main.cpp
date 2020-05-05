#include <iostream>  // std::cout, std::endl
#include <string>    // std::string

#include "DistanceMatrix.h"
#include "prim_binary_heap_mst.h"
#include "read_file.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(0);
    }

    const char* filename = argv[1];
    auto point_reader(read_file(filename));
    auto&& distance_matrix = point_reader->create_distance_matrix();

    // std::cout << distance_matrix << std::endl;

    const auto mst(mst::prim_binary_heap_mst(std::move(distance_matrix)));

    std::cout << "MST" << std::endl;
    for (const auto& [from, to, weight] : mst) {
        std::cout << from << ", " << to << " (" << weight << ");" << std::endl;
    }

    std::cin.get();
}
