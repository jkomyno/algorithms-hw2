# Usage: ./benchmark.sh
#
# Create a csv benchmark file for each program supplying to it the datasets
# in dataset folder.

exe_folder="."
algorithms="MST2Approximation FarthestInsertion HeldKarp" # SimulatedAnnealing
output_folder="benchmark"
ext=".out"

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
