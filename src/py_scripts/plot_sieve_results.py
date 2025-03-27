import os

import numpy as np
from matplotlib import pyplot as plt

# defaults plot values
output_dir = './output/'
plot_ext = 'svg'


# Function to read results from a file
def read_results(filename: str):
    data = {}
    with open(f"./output/{filename}.txt", 'r') as file:
        lines = file.readlines()

        # Read the test range from the first line
        test_range_line = lines[0].strip().split()

        # Extract the range part (e.g., "10^3:10^9")
        range_part = test_range_line[2].split(':')

        # Parse the exponents from the format "10^3" and "10^9"
        min_exp = int(range_part[0].split('^')[1])
        max_exp = int(range_part[1].split('^')[1])

        # Read the results
        for line in lines[1:]:
            algorithm, times = line.strip().split(': ')
            times = list(map(int, times.strip('[]').split(', ')))
            data[algorithm] = times

    return data, 10, list(np.arange(min_exp, max_exp + 1))


# Function to plot the time performance of sieve benchmarking results
def plot_sieve_results(filename: str, save: bool = False):
    data, base, exponents = read_results(filename)
    x_values = [base**exp for exp in exponents]

    # Normalize the data
    normalized_data = {}
    for algorithm, times in data.items():
        normalized_times = [(time * 1000) / n for time,
                            n in zip(times, x_values)]
        normalized_data[algorithm] = normalized_times

    # Plotting the normalized results
    fig, ax = plt.subplots(figsize=(8, 6))

    title = "Time-Performance Analysis:\nNormalized execution time over incrementing powers of 10 as limits"
    ax.set_title(title)
    ax.set_xlabel(f'N')
    ax.set_ylabel('μs/n * 1000')

    x_ticks = np.log10(x_values)
    all_y = []

    for _, (algorithm, times) in enumerate(normalized_data.items()):
        label = algorithm
        y = times
        all_y.append(y)
        ax.plot(x_ticks, y, label=label, marker='o')

    # Set y ticks
    max_y = max([max(y) for y in all_y])
    ax.set_yticks(np.arange(0, max_y + 1, 1))

    # Set x ticks
    ax.set_xticks(x_ticks)
    ax.set_xticklabels([f'$10^{{{exp}}}$' for exp in exponents])

    ax.grid(True)
    plt.legend()
    plt.show()

    if save:
        save_plot_figure(fig, dir=output_dir, filename=filename)


# Function to save the plot figure
def save_plot_figure(fig, dir, filename):
    """save plot in svg format in the default output_dir"""
    filepath = os.path.join(dir, f"{filename}.{plot_ext}")
    fig.savefig(filepath, format=plot_ext)
    print(f"Figure saved as {filepath}")


# plot n, log n, n log log n, n/log n, n/log log n, for n in [10^1, 10^2, ..., 10^9]
def plot_complexity(save: bool = False):
    """Plot various complexity functions for n in [10^1, 10^2, ..., 10^9]"""
    exponents = list(range(1, 10))
    x_values = [10**exp for exp in exponents]

    # Calculate complexity functions
    n_values = x_values
    log_log_n_values = [np.log(np.log(n)) for n in x_values]
    print(log_log_n_values)
    n_log_log_n_values = [n * np.log(np.log(n)) for n in x_values]
    print(n_log_log_n_values)
    sqrt_n_values = [n**.5 for n in x_values]
    print(sqrt_n_values)
    log_n_values = [np.log(n) for n in x_values]
    print(log_n_values)
    n_div_log_log_n_values = [n / np.log(np.log(n)) for n in x_values]
    print(n_div_log_log_n_values)

    # Create plot
    fig, ax = plt.subplots(figsize=(10, 6))

    # Plot each complexity function
    ax.plot(exponents, n_values, label='n', marker='o')
    ax.plot(exponents, n_log_log_n_values, label='n log log n', marker='^')
    ax.plot(exponents, n_div_log_log_n_values, label='n/log log n', marker='d')
    ax.plot(exponents, sqrt_n_values, label='sqrt(n)', marker='d')
    ax.plot(exponents, log_n_values, label='log n', marker='d')
    ax.plot(exponents, log_log_n_values, label='log log n', marker='s')

    # Set up the plot
    ax.set_title("Complexity Functions Growth")
    ax.set_xlabel('n (powers of 10)')
    ax.set_ylabel('Value')
    ax.set_xticks(exponents)
    ax.set_xticklabels([f'$10^{{{exp}}}$' for exp in exponents])
    ax.legend()
    ax.grid(True)

    # Use logarithmic scale for y-axis due to large value differences
    ax.set_yscale('log')

    plt.tight_layout()
    plt.show()

    if save:
        save_plot_figure(fig, dir=output_dir, filename="complexity_functions")


def read_w_op(filename: str):
    """
    Read the sieve_w_op results file.
    The file format is a pipe-separated table, for example:

    | n                       | Sieve-Eratosthenes      | Sieve-iZ                | Sieve-iZm               
    | 1000                    | 902                     | 197                     | 86                      
    | 10000                   | 11958                   | 2892                    | 1451                    
    | 100000                  | 143015                  | 36605                   | 19591                   
    | ... (more rows)

    Returns:
        data: dict mapping column headers to lists of values.
              For example: 
              { "n": [1000, 10000, ...],
                "Sieve-Eratosthenes": [902, 11958, ...],
                ... }
    """
    filepath = os.path.join(output_dir, f"{filename}.txt")
    data = {}
    header = None
    with open(filepath, 'r') as file:
        lines = file.readlines()
        for line in lines:
            line = line.strip()
            if not line:
                continue
            # Remove leading and trailing '|' characters and split by '|'
            parts = [part.strip() for part in line.strip('|').split('|')]
            if header is None:
                header = parts
                # Initialize lists for each column.
                for col in header:
                    data[col] = []
            else:
                # Ensure the row has the same number of columns as the header.
                if len(parts) == len(header):
                    for i, value in enumerate(parts):
                        # For the "n" column and algorithm columns, try converting to int.
                        try:
                            num = int(value)
                            data[header[i]].append(num)
                        except ValueError:
                            data[header[i]].append(value)
    return data


def plot_w_op(filename: str, save: bool = False):
    """
    Plot the normalized Mark-Composite Write Operations W(n) from the sieve results file.

    The file is expected to have columns:
      n, Sieve-Eratosthenes, Sieve-iZ, Sieve-iZm.

    This function normalizes each algorithm's values by dividing by n for each row,
    then plots log10(n) on the x-axis and the normalized values on the y-axis.
    A horizontal dashed line at y = 1 (i.e. n/n) is added.
    """
    data = read_w_op("sieve_w_op")
    # x-axis values are from column "n"
    x_vals = data["n"]
    # Transform n values to log10(n)
    x_vals_log = [np.log10(n) for n in x_vals]

    fig, ax = plt.subplots(figsize=(8, 6))
    title = "Normalized Mark-Composite Write Operations W(n)/n"
    ax.set_title(title)
    ax.set_xlabel("limit n")
    ax.set_ylabel("Normalized Write Operations (W(n)/n)")
    ax.grid(True)

    # Define markers for each algorithm
    algorithms = ["Sieve-Eratosthenes", "Sieve-iZ", "Sieve-iZm"]
    markers = {"Sieve-Eratosthenes": "o", "Sieve-iZ": "s", "Sieve-iZm": "d"}
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

    for i, algo in enumerate(algorithms):
        if algo in data:
            y_vals = data[algo]
            # Normalize by dividing each value by corresponding n.
            normalized = [y / n for y, n in zip(y_vals, x_vals)]
            ax.plot(x_vals_log, normalized, marker=markers[algo],
                    color=colors[i % len(colors)],
                    label=algo, linewidth=2)

    # Draw a horizontal dashed line at y = 1
    ax.hlines(1, min(x_vals_log), max(x_vals_log), colors='black',
              linestyles='dashed', label='n/n = 1')
    # Set x ticks to the log10(n) values and label them as 10^exp
    ax.set_xticks(x_vals_log)
    ax.set_xticklabels([f"$10^{{{int(np.log10(n))}}}$" for n in x_vals])
    ax.legend()
    plt.tight_layout()
    plt.show()

    if save:
        save_plot_figure(fig, output_dir, "sieve_w_op")


if __name__ == '__main__':
    # plot_complexity(save=True)
    # plot_w_op("sieve_w_op", save=True)
    plot_sieve_results("sieve_results_20250327221757", save=True)
