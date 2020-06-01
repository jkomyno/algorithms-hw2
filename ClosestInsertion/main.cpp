#include <iostream>  // std::cout, std::endl

// #include "parallel_executor.h"
#include "DistanceMatrix.h"
#include "closest_insertion_tsp.h"
#include "random_generator.h"
#include "read_file.h"
#include "sequential_executor.h"
#include "shared_utils.h"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(0);
    }

    const char* filename = argv[1];
    auto point_reader(read_file(filename));
    auto distance_matrix = point_reader->create_distance_matrix();

    // Generate a random source node for the Hamiltonian cycle found with the Closest Insertion
    // heuristic. If you'd rather started from a fixed node (e.g. 0), just substitute the following
    // line with `random_generator::FixedGenerator<size_t> rand_int(0);`.
    random_generator::IntegerRandomGenerator rand_int(0, distance_matrix.size() - 1);

    // calculate the weight of TSP with Closest Insertion Heuristic
    auto solve_tsp = [&]() {
        return closest_insertion_tsp(distance_matrix, rand_int);
    };

    // run Closest Insertion only once
    const auto executor(executor::sequential_executor(1, std::move(solve_tsp)));

    // run Closest Insertion as many times as the number of CPU cores
    // const auto executor(executor::parallel_executor({}, std::move(solve_tsp)));

    // save the best cost found
    const int total_weight = executor.get_best_result(utils::min_element);

    // use std::fixed to avoid displaying numbers in scientific notation
    std::cout << std::fixed << total_weight << std::endl;
}
