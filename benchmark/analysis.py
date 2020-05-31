"""
Usage:
> python3 analysis.py [--help] [--latex] [--plot] [--tables] [--save-plot]

The following script analyzes benchmarks for KruskalNaive, KruskalUnionFind (and variants) and
PrimBinaryHeap algorithms.

The aim of the script is to produce in output som graphical representation of
the benchmarks, in particular
- for each algorithm show how it scales with the size of the input
- for some (maybe fixed) input sizes show the compared performance of the
  implemented algorithms.

Options:
- [--help]: show this message
- [--tables]: show comparison tables
- [--latex]: export comparison tables in latex format (use with [--tables])
- [--plot]: plot comparisons
- [--save-plot]: save charts to a specific folder instead of visualizing them
"""

from functools import reduce
import sys
import os
from io import StringIO

import pandas as pd
import numpy as np
import glob
from itertools import chain
from tabulate import tabulate
from typing import List, Dict
import matplotlib.pyplot as plt
from types import SimpleNamespace
from tably import Tably
import seaborn as sns


# number of floating point decimals displayed in output
N_DECIMALS = 3

# number of floating point decimals displayed in output for values that represent percentages
N_DECIMALS_PERCENTAGE = 2

IS_HELP = '--help' in sys.argv
# if the script is given the argument '--latex', turn on LaTeX table generation
FOR_LATEX = '--latex' in sys.argv
# if the script is given the argument '--tables', turn on tables comparisons
IS_TABLE_ENABLED = '--tables' in sys.argv
# if the script is given the argument '--plot', turn on comparison plots (with options)
IS_PLOT_ENABLED = '--plot' in sys.argv
# if the script is given the argument '--save-plot', save plots to folder.
IS_SAVE_PLOT_ENABLED = '--save-plot' in sys.argv

PLOT_IMG_DIR = 'images'

HELD_KARP = 'HeldKarp'
HELD_KARP_DYNAMICBITMASKING = 'HeldKarp_DynamicBitMasking'
HELD_KAR_BITMASKING = 'HeldKarp_BitMasking'
HELD_KARP_UNORDERED_SET = 'HeldKarp_UnorderedSet'
MST_2_APPROX = 'MST2Approximation'
FARTHEST_INSERTION = 'FarthestInsertion'
SIMULATED_ANNEALING = 'SimulatedAnnealing'
CLOSEST_INSERTION = 'ClosestInsertion'
CLOSEST_INSERTION_PARALLEL = 'ClosestInsertionParallel'

programs = [
    HELD_KARP,
    MST_2_APPROX,
    FARTHEST_INSERTION,
    SIMULATED_ANNEALING,
    HELD_KARP_DYNAMICBITMASKING,
    CLOSEST_INSERTION,
    CLOSEST_INSERTION_PARALLEL,
    HELD_KAR_BITMASKING,
    HELD_KARP_UNORDERED_SET
]

ms_programs = [
    'ms_held_karp',
    'ms_mst2approx',
    'ms_farthest_insertion',
    'ms_simulated_annealing',
    'ms_HeldKarp_DynamicBitMasking',
    'ms_closest_insertion',
    'ms_closest_insertion_parallel',
    'ms_HeldKarp_BitMasking',
    'ms_HeldKarp_UnorderedSet'
]

GROUND_TRUTH_DF = pd.DataFrame({
    'd': [ 52, 14, 150, 493, 1000, 51, 202, 229, 100, 100, 442, 16, 22 ],
    'instance': [
        'berlin52.tsp', 'burma14.tsp', 'ch150.tsp', 'd493.tsp', 'dsj1000.tsp', 'eil51.tsp', 'gr202.tsp',
        'gr229.tsp', 'kroA100.tsp', 'kroD100.tsp', 'pcb442.tsp', 'ulysses16.tsp', 'ulysses22.tsp',],
    'exact': [ 7542, 3323, 6528, 35002, 18659688, 426, 40160, 134602, 21282, 21294, 50778, 6859, 7013, ]
})


def read_csvs_of_program(program: str) -> List[pd.DataFrame]:
    """
    Given the name of an algorithm program, collects the list of CVS benchmarks recorded
    for that particular program.
    :param program: name of the program which benchmarks should be retrieved
    :return: list of benchmark CSV files
    """

    csv_files = glob.glob(f'./{program}.*.csv')
    dataframes_per_program = []

    for csv_file in csv_files:
        dataframe = pd.read_csv(csv_file, sep=';', decimal='.', encoding='utf-8')
        dataframes_per_program.append(dataframe)

    return dataframes_per_program


def check_same_n_rows(dfs_flat: List[pd.DataFrame]):
    """
    Utility function
    :param dfs_flat:
    """

    dfs_size = map(lambda x: x.shape[0], dfs_flat)
    is_valid = len(set(dfs_size)) == 1
    if not is_valid:
        raise AssertionError('The CSV files have different number of rows')


def check_same_outputs(dfs_flat: List[pd.DataFrame]):
    """
    :param dfs_flat:
    :return:
    """
    # Nothing to do, solution can be approximated.
    pass


def check_not_empty(dfs:  Dict[str, List[pd.DataFrame]]):
    for k, v in dfs.items():
        if len(v) == 0:
            raise AssertionError(f'There are no data for {k}')


def check_validity(dfs: Dict[str, List[pd.DataFrame]]):
    """
    Utility functions that checks whether the imported benchmark CSVs are valid
    :param dfs_list: list of list of CSVs represented as DataFrames
    """
    dfs_list = dfs.values()
    dfs_flat: List[pd.DataFrame] = list(chain.from_iterable(dfs_list))

    check_not_empty(dfs)
    check_same_n_rows(dfs_flat)
    check_same_outputs(dfs_flat)


def get_row_index_with_max_edges(d: int) -> int:
    """
    Return the row index of any benchmark CSV file with given dimension.
    The number of nodes considered "inseresting" are:
    - 14
    - 16
    - 22
    - 52
    - 202
    - 1000

    :param d: graph dimension in term nodes
    :return: index of line with given graph dimension
    """

    # these values correspond to the (line - 2) of the CSV line
    obj = {
        14: 1,
        16: 11,
        22: 12,
        52: 0,
        202: 6,
        1000: 4,
    }

    return obj[d]


def get_min_at_row(dfs: List[pd.DataFrame], row: int, column: str = 'ms') -> np.double:
    """
    Return the minimum value of a certain column among the given list of benchmark dataframes in a certain row
    :param dfs: list of benchmark dataframes
    :param row: index of the row to be selected
    :param column: column of the dataframe to be selected. Default: 'ms'
    :return: minimum value of column at row
    """

    values_at_row_column = [df[column].loc[row] for df in dfs]
    min_index, min_value = min(enumerate(values_at_row_column), key=lambda x: x[1])
    return min_index, min_value

def get_median_at_row(dfs: List[pd.DataFrame], row: int, column: str = 'output') -> np.double:
    """
    Return the median value of a certain column among the given list of benchmark dataframes in a certain row
    :param dfs: list of benchmark dataframes
    :param row: index of the row to be selected
    :param column: column of the dataframe to be selected. Default: 'ms'
    :return: median value of column at row
    """
    values_at_row_column = [df[column].loc[row] for df in dfs]
    return np.median(values_at_row_column)


def create_list_of_ms_at_row(dfs_list: List[pd.DataFrame], columns: List[int]) -> List[List[np.double]]:
    """
    :param dfs_list: list of benchmark dataframes
    :param columns: list of columns to be displayed in a table, aka list of "interesting" number of nodes
    :return: 2-d list of minimum execution times. For each column in columns, the row which stores the maximum
    number of edges wrt to the fixed number of nodes (column) is selected.
    """

    return [
        [
            df['ms'].loc[get_row_index_with_max_edges(n_nodes_as_str)]
            for n_nodes_as_str in columns
        ]
        for df in dfs_list
    ]


def compare_2_programs(dfs_dict: Dict[str, pd.DataFrame], program_1: str, program_2: str) -> pd.DataFrame:
    """
    Compare 2 programs, putting in evidence their difference in runtime execution and how much the 2nd program
    improves the execution time compared to the 1st program.
    :param dfs_dict: map of DataFrames minimized wrt to column 'ms'
    :param program_1: name of the 1st program to compare
    :param program_2: name of the 2nd program to compare
    :return: new DataFrame containing the comparison information between the given two programs
    """

    # subset of dfs_dict containing only the list of dataframes related to program_1 and program_2
    dfs_filter: Dict[str, pd.DataFrame] = dict((k, v) for k, v in dfs_dict.items() if k in [program_1, program_2])
    dfs_valid: List[pd.DataFrame] = [dfs_filter[program] for program in [program_1, program_2]]

    # comparison DataFrame columns
    columns = [ 14, 16, 22, 52, 202, 1000 ]

    # compute rows of program_1 and program_2
    program_rows_np = np.asarray(create_list_of_ms_at_row(dfs_valid, columns), dtype=np.double)

    # compute difference row
    diff_np = program_rows_np[0] - program_rows_np[1]

    # compute improvement row (expressed as a percentage number)
    improvement_percentage = np.around(100 * diff_np / program_rows_np[0], decimals=N_DECIMALS_PERCENTAGE).tolist()

    # remove numpy wrapper
    program_1_row, program_2_row = program_rows_np.tolist()
    diff = diff_np.tolist()

    data = [
        [program_1, *program_1_row],
        [program_2, *program_2_row],
        ['Difference', *diff],
        ['Improvement %', *improvement_percentage]
    ]

    return pd.DataFrame(data, columns=['', *columns])


def compare_n_programs(dfs_dict: Dict[str, pd.DataFrame], programs: List[str]):

    df = GROUND_TRUTH_DF

    for program in programs:
        data = dfs_dict[program][['d', 'ms', 'output', 'filename']]

        data = data[['output', 'ms']]
        data['error'] = calculate_error(df['exact'].tolist(), data['output'].tolist())

        # https://stackoverflow.com/questions/17985159/creating-dataframe-with-hierarchical-columns
        # is not working when pretty printed.
        df = pd.concat([df, data], axis=1, sort=False)

    df = df.sort_values('d')
    df = df.drop('d', axis=1)
    df = df.rename(columns={
        'instance': 'Instance',
        'exact': 'Exact',
        'output':'Solution',
        'ms': 'Time (ms)',
        'error': 'Error (%)'
    })

    return df


def calculate_error(exact, output):
    """
    :param exact: A list with ground truth values.
    :param output: A list with actual values.
    :return: The absolute error in %.
    """
    return np.abs(np.around(100 * (np.subtract(exact, output)) / exact, decimals=N_DECIMALS_PERCENTAGE))


def print_comparison_time(dfs_dict: Dict[str, pd.DataFrame], programs: List[str]):
    print(f'Comparison {names_to_vs(programs)}.')

    df_comparison = compare_2_programs(dfs_dict, programs[0], programs[1])
    pretty_print_pandas(df_comparison)


def print_comparison(dfs_dict: Dict[str, pd.DataFrame], programs: List[str]):
    print(f'Comparison {names_to_vs(programs)}.')

    df_comparison = compare_n_programs(dfs_dict, programs)
    pretty_print_pandas(df_comparison)


def pretty_print_pandas(df: pd.DataFrame, tablefmt='pretty'):
    """
    Pretty-printer utility for Pandas DataFrames.
    :param df: DataFrame to pretty-print
    :param tablefmt: 'pretty' | 'psql' | 'latex'
    """
    if FOR_LATEX:
        tablefmt = 'latex'  # print out \tabular{} LaTeX tables
    print(tabulate(df.round(N_DECIMALS), headers='keys', tablefmt=tablefmt, showindex=False))
    print('\n')


def minimize_ms_dataframes_helper(dfs: List[pd.DataFrame]) -> pd.DataFrame:
    """
    Create a new in-memory DataFrame that, for each row of all the dataframes in dfs, keeps only the row with the
    minimum 'ms' value.
    :param dfs: List of DataFrames for a single program
    :return: new DataFrame minimized row-wise wrt the value of the 'ms' column
    """
    columns = ['ms', 'output', 'd', 'weight_type', 'filename']
    min_df = pd.DataFrame(columns=columns)

    n_rows = dfs[0].shape[0]
    for row in range(n_rows):
        min_index, _ = get_min_at_row(dfs, row, column='ms')
        median_output = get_median_at_row(dfs, row, column='output')
        data = dfs[min_index].loc[row].copy()  # need to copy to avoid side effect on original dataframe.
        data['output'] = median_output
        min_df = min_df.append(data)

    return min_df.round(decimals=N_DECIMALS)


def minimize_ms_dataframes(dfs_dict: Dict[str, List[pd.DataFrame]]) -> Dict[str, pd.DataFrame]:
    """
    For each list of DataFrames stored in dfs_dict keys, retain only the rows which value at column 'ms' is minimum.
    :param dfs_dict: original map of list of benchmark DataFrames for each program
    :return: minimized map of 1 dataframe for each program wrt to column 'ms'
    """
    dfs_flat_min = [minimize_ms_dataframes_helper(dfs) for dfs in dfs_dict.values()]
    return dict(zip(programs, dfs_flat_min))


def keep_only_interesting_nodes(df: pd.DataFrame, columns: List[str]) -> pd.DataFrame:
    return pd.DataFrame([
        df[columns].loc[get_row_index_with_max_edges(14)],
        df[columns].loc[get_row_index_with_max_edges(16)],
        df[columns].loc[get_row_index_with_max_edges(22)],
        df[columns].loc[get_row_index_with_max_edges(52)],
        df[columns].loc[get_row_index_with_max_edges(202)],
        df[columns].loc[get_row_index_with_max_edges(1000)],
    ], columns=columns)


def dataframe_to_stringio(df: pd.DataFrame) -> StringIO:
    csv_buffer = StringIO()

    # removes the number of vertexes/edges
    df.drop(columns=['n', 'm']) \
        .rename(columns={'output': 'MST', 'filename': 'File'}) \
        .to_csv(csv_buffer, index=None, sep=';')

    csv_buffer.seek(0)
    return csv_buffer


def export_dataframes_min_to_latex(dfs: Dict[str, pd.DataFrame]):
    dfs_list = dfs.values()
    csv_list = list(map(dataframe_to_stringio, dfs_list))

    for i, program in enumerate(dfs.keys()):
        tably_instance = Tably(SimpleNamespace(
            files=[csv_list[i]],
            align=['l', 'l', 'c'],
            caption=f'Risultati di {program}',
            no_indent=False,
            skip=0,
            label=f'table:{program}-results',
            no_header=False,
            outfile=f'../report/{program}.min.tex',
            separate_outfiles=None,
            preamble=False,
            sep='semi',
            units='-',
            no_escape=False,
            fragment=False,
            fragment_skip_header=False,
            replace=True,
        ))
        tably_instance.run()


def show_or_save_plot(title: str):
    if IS_SAVE_PLOT_ENABLED:
        if not os.path.exists(f'./{PLOT_IMG_DIR}'):
            os.makedirs(f'./{PLOT_IMG_DIR}')
        out_title = title.translate ({ord(c): "_" for c in " !@#$%^&*()[]{};:,./<>?\|`~-=+"})
        plt.savefig(f'./{PLOT_IMG_DIR}/{out_title}.png')
    else:
        plt.show()


def names_to_vs(names: List[str]) -> str:
    """
    Return a string made by names joined by "vs".
    """
    return reduce(lambda x, y: x + ' vs ' + y, names) if len(names) > 1 else names[0]


def names_to_dfs(names: List[str], dfs) -> Dict[str, List[pd.DataFrame]]:
    """
    Return a dict where for each name the key is one of the name given and the
    value is the corresponding benchmark dataframe.
    """
    return reduce(lambda x, y: {**x, **y}, map(lambda n: {n: dfs[n]}, names))


def filter_df(df: pd.DataFrame, pred):
    """
    Return the filtered version of given dataframe w.r.t. pred, i.e. a dataframe where pred holds on each row.
    :param df: A dataframe.
    :param pred: A predicate over a dataset row.
    """
    return pd.DataFrame([df.loc[i] for i in range(len(df)) if pred(df.loc[i])], columns=df.columns)


def plot_line(dfs: Dict[str, pd.DataFrame], x: str, y: str, y_log=False):
    """
    Plots `dfs` data.
    :dfs: a dictionary with the benchmark dataframe to plot.
    :x: the x axis name.
    :y: the y axis name.
    :y_log: a boolean flag that indicates whether y should be log scaled or not.
    """
    plt.figure(figsize=(14, 7))
    for k, v in dfs.items():
        g = sns.lineplot(v[x], v[y], label=k)
        if y_log:
            g.set_yscale('log')


def plot_series(names: List[str], dfs: pd.DataFrame, y_log=False):
    """
    Plot given programs.
    """
    benchmark_subset = names_to_dfs(names, dfs)
    plot_line(benchmark_subset, x='d', y='ms', y_log=y_log)


def plot_comparison(names: List[str], dfs: Dict[str, pd.DataFrame], title, pred=lambda _: True, filename=None, y_log=False):
    """
    Plot filtered dataframes w.r.t. pred.
    :param names: A list of programs.
    :param dfs: A dictionary of dataframes of the type (name -> dataframe).
    :param pred: A predicate over a dataframe benchmark row.
    :param title: The plot title.
    :param filename: The plot filename, default is `title`.
    :param y_log: a boolean flag that indicates whether y should be log scaled or not.
    """
    d = {}
    for i in range(len(names)):
        name = names[i]
        df = filter_df(dfs[name], pred=pred)
        d = {**d, **{name: df}}

    plot_series(names, d, y_log=y_log)

    plt.title(title)

    show_or_save_plot(filename if filename is not None else title)


def plot_precision_comparison(names: List[str], dfs: Dict[str, pd.DataFrame], title: str, pred=lambda _: True, filename=None, y_log=False):
    """
    Plot filtered dataframes w.r.t. pred.
    :param names: A list of programs.
    :param dfs: A dictionary of dataframes of the type (name -> dataframe).
    :param pred: A predicate over a dataframe benchmark row.
    :param title: The plot title.
    :param filename: The plot filename, default is `title`.
    :param y_log: A flag for scaling y axis.
    """
    df = pd.DataFrame()

    for i in range(len(names)):
        name = names[i]
        dfx = dfs[name]
        dfx['error'] = calculate_error(GROUND_TRUTH_DF['exact'].tolist(), dfs[name]['output'].tolist())
        dfx['name'] = name
        dfx = filter_df(dfx, pred=pred)
        df = df.append(dfx)

    # not working properly
    # sns.set(rc={'figure.figsize':(14, 7)})

    g = sns.catplot(x='d', y='error', hue='name', data=df,  kind='bar', aspect=2,  margin_titles=True)
    g.set_ylabels("Error (%)")
    g.set_xlabels("Nodes")
    g.set_titles(title)

    if y_log:
        ax = g.axes
        ax[0,0].set_yscale('log')

    show_or_save_plot(filename if filename is not None else title)


if __name__ == '__main__':
    if IS_HELP:
        print(__doc__)
        exit(0)

    # benchmark CSVs stored in DataFrames and grouped by program name
    generator = zip(programs, [read_csvs_of_program(program) for program in programs])
    dataframes: Dict[str, List[pd.DataFrame]] = dict(generator)

    # is some CSV is invalid, throws an AssertionError
    check_validity(dataframes)

    # for each CSV, only the minimum values for the 'ms' column are retained
    # len(dataframes_min) == len(programs)
    dataframes_min = minimize_ms_dataframes(dataframes)

    if IS_TABLE_ENABLED:
        # compare multiple programs to show potential improvements
        
        # CLOSEST_INSERTION
        # print_comparison(dataframes_min, [CLOSEST_INSERTION])
        # print_comparison(dataframes_min, [CLOSEST_INSERTION, FARTHEST_INSERTION])

        # Question 1 (complete and splitted, more readable).
        print_comparison(dataframes_min, [HELD_KARP, MST_2_APPROX, FARTHEST_INSERTION])
        print_comparison(dataframes_min, [HELD_KARP])
        print_comparison(dataframes_min, [MST_2_APPROX])
        print_comparison(dataframes_min, [FARTHEST_INSERTION])

    # export minimized in-memory CSV files to LaTeX tables (they will still require some manual work tho)
    # export_dataframes_min_to_latex(dataframes_min)

    if IS_PLOT_ENABLED:
        # # OK (speed): HelpKarp with more than 52 nodes timeouts (Timeout is setted to 2 minutes)
        # plot_comparison([HELD_KARP], dataframes_min, pred=lambda x: x['d'] <= 52, title=f'{HELD_KARP} (52 nodes)')

        # # OK (speed): approximated solution vs heuristic
        # plot_comparison([MST_2_APPROX, FARTHEST_INSERTION], dataframes_min, pred=lambda x: True, title=f'{names_to_vs([MST_2_APPROX, FARTHEST_INSERTION])}', y_log=True)

        # # OK (speed): the three (y log scaled)
        # plot_comparison([MST_2_APPROX, FARTHEST_INSERTION, SIMULATED_ANNEALING], dataframes_min, pred=lambda x: True, title=f'{names_to_vs([MST_2_APPROX, FARTHEST_INSERTION, SIMULATED_ANNEALING])} (y log scaled)', y_log=True)

        # # OK (speed): simulated annealing vs approx solution
        # plot_comparison([SIMULATED_ANNEALING], dataframes_min, pred=lambda x: True, title=f'{names_to_vs([SIMULATED_ANNEALING])}')
        # plot_comparison([SIMULATED_ANNEALING, MST_2_APPROX], dataframes_min, pred=lambda x: True, title=f'{names_to_vs([SIMULATED_ANNEALING, MST_2_APPROX])} (y log scaled)', y_log=True)
        # plot_comparison([SIMULATED_ANNEALING, FARTHEST_INSERTION], dataframes_min, pred=lambda x: True, title=f'{names_to_vs([SIMULATED_ANNEALING, MST_2_APPROX])} (y log scaled)', y_log=True)

        # OK (precision): heuristic
        #plot_precision_comparison([MST_2_APPROX, SIMULATED_ANNEALING, FARTHEST_INSERTION, CLOSEST_INSERTION], dataframes_min, pred=lambda x: True, title=f'{names_to_vs([MST_2_APPROX, SIMULATED_ANNEALING, FARTHEST_INSERTION, CLOSEST_INSERTION])} test')

        # OK: HedlKarp extensions (accuracy) 
        # plot_comparison([HELD_KARP_EXTENDED_BITSET,HELD_KAR_BITMASKING,HELD_KARP_DYNAMICBITMASKING], dataframes_min, pred=lambda x: x['d'] <= 22, title=f'test')

        # OK: ClosestInsertion (accuracy)
        plot_precision_comparison([CLOSEST_INSERTION], dataframes_min, pred=lambda x: True, title=f'{names_to_vs([CLOSEST_INSERTION])} (approximation error)')
        plot_precision_comparison([CLOSEST_INSERTION, FARTHEST_INSERTION], dataframes_min, pred=lambda x: True, title=f'{names_to_vs([CLOSEST_INSERTION, FARTHEST_INSERTION])} (approximation error)')

        # Question 1 (accuracy and runtime)
        plot_precision_comparison(
            [HELD_KARP, MST_2_APPROX, FARTHEST_INSERTION], 
            dataframes_min, 
            pred=lambda x: x['d'] <= 52, 
            title=f'{names_to_vs([HELD_KARP, MST_2_APPROX, FARTHEST_INSERTION])} (approximation error, limited to 52 nodes)', 
            y_log=False)
        plot_precision_comparison(
            [HELD_KARP, MST_2_APPROX, FARTHEST_INSERTION], 
            dataframes_min, 
            pred=lambda x: True, 
            title=f'{names_to_vs([HELD_KARP, MST_2_APPROX, FARTHEST_INSERTION])} (approximation error, y log scaled)', 
            y_log=True)
