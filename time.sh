# Usage: ./time.ps1 [program-name] [input-file] > output.csv
# Create a UTF-8 encoded CSV file that can be easily processed by Python
#
# The CSV file has the following columns
#  ms: number of milliseconds needed to execute the program on the given tsp dataset 
#  output: the output of the program
#  d: the dimension of the given tsp dataset
#  weight_type: the type of weight of given tsp dataset
#  filename: name of the input file with the tsp dataset
#
# Note: `program-name` should be fully qualified name, either with relative or absolute path.
# If you want to omit the path to the program you should export the variable PATH adding to it
# the path to the program.

if [ $# -lt 2 ]; then
    echo "USAGE: /time.sh program-name input-file"
    echo "EXAMPLE: ./time.sh \"./MyProgram.out\" \"path/to/dataset.tsp\""
    exit 1
fi

program=${1}
input_file=${2}

# algorithm benchmark
ts=$(date +%s%N)                      # start
output=$(${program} ${input_file})    # execute
te=$(date +%s%N)                      # end
ms=$((($te - $ts)/1000000))           # time taken

# parse useful dataset information
filename=$(basename ${input_file})
d=$(sed -n -e 's/^.*DIMENSION[ ]*:[ ]* //p' < ${input_file})
weight_type=$(sed -n -e 's/^.*EDGE_WEIGHT_TYPE[ ]*:[ ]* //p' < ${input_file})

# output
echo "${ms};${output};${d};${weight_type};${filename}"
