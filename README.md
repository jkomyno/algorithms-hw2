# Algorithms-HW2

[![Build Status](https://travis-ci.com/jkomyno/algorithms-hw2.svg?token=jkozECYTHDjAbqyyzwJB&branch=master)](https://travis-ci.com/jkomyno/algorithms-hw2)

> Travelling Salesman Problem with Held & Karp, approximated and heuristic 
> algorithms.

An hands-on experience about algorithms over graph and graph theory with the
focus on efficiency and, obviously, correctness. (*Part 2*)

## Usage

The repository comes with some utility commands that allows you to compile,
test and bench owr algorithms. We use *make* to automatize this process.

**Available commands**

- `make all`, to compile all the algorithm sources in this project.
- `make ALG`, where *ALG* is one of *HeldKarp*, *MST2Approximation*,
    *MST2Approximation*, *FarthestInsertionAlternative*, *SimulatedAnnealing*,
    *ClosestInsertion*, to compile given algorithm sources.
- `make benchmark`, to run benchmarks on our algorithms.
- `make clear`, to clean up the working directory.

Within the Makefile we provieded some variables to modify our pipeline.
In particular you can use your own compiler rewriting the `CXX` flag. Other
variables can be modified, for example `CXXFLAGS`, `OUT_DIR` and `EXT`.

Example
```
make CXX="g++" CXXFLAGS="-O3 -std=c++17 -I Shared" OUT_DIR="build" EXT="exe" all
```

**Scripts**

We created some bash scripts in order to automatize recurrent tasks like
benchmarking. You can take a look at the script `benchmark.sh` for further
informations.

## Project Structure

The project is structured as a unique **Visual Studio solution** containing
multiple subprojects, one for every implemented algorithm. The code for each
project is stored in a folder with the same name of the related algorithm.

These projects are:

* [HeldKarp](./HeldKarp): Held Karp TSP algorithm with dynamic bitmasking;
* [MST2Approximation](./MST2Approximation): TSP 2-approximated algorithm based on MST;
* [ClosestInsertion](./ClosestInsertion): Metric-TSP heuristic;
* [FarthestInsertion](./FarthestInsertion): another Metric-TSP heuristic;
* [FarthestInsertionAlternative](./FarthestInsertionAlternative): Metric-TSP heuristic with an alternative implementation w.r.t. the one given by our professor;
* [SimulatedAnnealing](./SimulatedAnnealing): TSP algorithm with temperature simulation.

The shared data structures and utils are stored in the *Shared* folder.

The project comes with some extra folders:
* **benchmark**: it contains CSV benchmarks of the algorithm as well as the
script used to analyze them. For further details please refer to the script
[analysis.py](./benchmark/analysis.py) that is self documented and it contains
all the stuff used in the analysis phase;
* **tsp_datasets**: it contains the input data for the algorithms given by our
professor, i.e. 13 graphs with either generated and real word data. The 
dimension of this graphs varies from 14 to 1000 nodes. The file format is
based on [TSP LIB](http://comopt.ifi.uni-heidelberg.de/software/TSPLIB95/).

## Related Projects

* [**algorithms-hw1**](https://github.com/jkomyno/algorithms-hw1) the previous
homework on MST algorithms.

## Authors

**Bryan Lucchetta**
- GitHub: [@1-coder](https://github.com/1-coder)

**Luca Parolari**
- GitHub: [@lparolari](https://github.com/lparolari)

**Alberto Schiabel**
- GitHub: [@jkomyno](https://github.com/jkomyno)

## License

This project is MIT licensed. See [LICENSE](LICENSE) file.
