#include <iostream>  // std::cout, std::endl

#include "DistanceMatrix.h"
#include "HeldKarpAlberto.h"
#include "read_file.h"
#include "timeout.h"
// #include "HeldKarpBryan.h"

#include <thread>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "1 argument required: filename" << std::endl;
        exit(1);
    }

    // read the graph and create the distance matrix
    const char* filename = argv[1];
    auto point_reader(read_file(filename));
    // auto distance_matrix = point_reader->create_distance_matrix();

    /*
    // matrix used for debug purposes
    DistanceMatrix<int> distance_matrix {
        0, 2, 4, 1, 6,
        2, 0, 5, 4, 7,
        4, 5, 0, 2, 1,
        1, 4, 2, 0, 8,
        6, 7, 1, 8, 0
    };
    */

    std::cout << "A" << std::endl;

    auto f = [&point_reader]() {
        auto distance_matrix = point_reader->create_distance_matrix();
        // the TSP timeout is set to 5 minutes
        auto timeout_min = 5min;

        std::cout << "B" << std::endl;

        // it's either the weight of the optimal Hamiltonian cycle, or an upper-bound of it in case
        // the computation requires more time than the alotted timeout.
        int total_weight = timeout::with_timeout(std::move(timeout_min), &held_karp_tsp_rec,
                                                 std::move(distance_matrix));

        std::cout << std::fixed << total_weight << std::endl;
    };

    std::packaged_task<void()> task(f);
    std::thread thread(std::move(task));

    std::cout << "C" << std::endl;

    thread.join();

    std::cout << "D" << std::endl;

    /*
    // the TSP timeout is set to 5 minutes
    auto timeout_min = 5min;

    // it's either the weight of the optimal Hamiltonian cycle, or an upper-bound of it in case the
    // computation requires more time than the alotted timeout.
    int total_weight = timeout::with_timeout(std::move(timeout_min), &held_karp_tsp_rec,
                                             std::move(distance_matrix));

    std::cout << std::fixed << total_weight << std::endl;
    */
}
