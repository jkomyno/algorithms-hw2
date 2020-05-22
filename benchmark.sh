#!/bin/bash

# Usage: ./benchmark.sh
#
# Create a csv benchmark file for each program supplying to it the datasets
# in dataset folder.

IFS=' '
read -ra os <<< "$(uname -a)"
os=${os[0]}

if [[ $os ==  MINGW* ]];
then
    exe_folder="./x64/Release";
    ext=".exe";
else
    exe_folder=".";
    ext=".out";
fi

algorithms="MST2Approximation FarthestInsertion SimulatedAnnealing ClosestInsertion HeldKarp"
output_folder="benchmark"

datasets="tsp_dataset"

mkdir -p $output_folder

for algorithm in ${algorithms}; do
    program="${algorithm}${ext}"
    timestamp=$(date "+%Y%m%d_%H%M%S_%N")
    timestamp=${timestamp: 0:18}  # trimming to 2 nanoseconds
    output_filename="${algorithm}.${timestamp}.csv"
    destination="${output_folder}/${output_filename}"

    echo "Benchmark start: ${algorithm}"

    echo "ms;output;d;weight_type;filename" >> ${destination}

    for dataset in ${datasets}/*.tsp; do
        ./time.sh "${exe_folder}/${program}" ${dataset} >> ${destination}
    done

    echo "Benchmark end: ${algorithm}"
    echo ""
done
