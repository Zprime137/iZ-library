import os
import re

import numpy as np
from matplotlib import pyplot as plt

output_dir = './output/'
plot_ext = 'svg'


def read_results(filename: str):
    """
    Read a results file for prime generation.  
    Expects a file with the following format:

      Target Bit Size: <bit_size>

      -----
      Algorithm: <algorithm info>
      Cores Number: <value>
      (Optional Prime Result lines)
      Time Results (seconds): [<time1>, <time2>, ...]
      Average Time: <value> seconds

      -----
      ... (next section)

    Returns:
        bit_size (int)
        results: a list of dictionaries with keys:
                 algorithm, cores, times (list of individual time measurements),
                 avg_time.
    """
    filepath = os.path.join(output_dir, f"{filename}.txt")
    with open(filepath, 'r') as file:
        lines = [line.strip() for line in file.readlines()]

    # The first non-empty line is expected to specify the target bit size.
    bit_size = None
    for line in lines:
        if line.startswith("Target Bit Size:"):
            m = re.match(r"Target Bit Size:\s*(\d+)", line)
            if m:
                bit_size = int(m.group(1))
            break

    results = []
    section_lines = []
    for line in lines:
        # Sections are separated by lines starting with "-----"
        if line.startswith("-----"):
            if section_lines:
                r = parse_section(section_lines)
                if r:
                    results.append(r)
                section_lines = []
        else:
            if line:
                section_lines.append(line)
    if section_lines:
        r = parse_section(section_lines)
        if r:
            results.append(r)

    return bit_size, results


def parse_section(lines):
    """
    Parse one section from the results file.  
    Expected are the following fields:
       - Algorithm: <algorithm info>
       - Cores Number: <value>
       - Time Results (seconds): [<time1>, <time2>, ...]
       - Average Time: <value> seconds
    Other lines (e.g. "Prime Result ..." lines) are ignored.

    Returns a dict with keys: algorithm, cores, times (list), avg_time.
    """
    algorithm = None
    cores = None
    times = []
    avg_time = None

    for line in lines:
        if line.startswith("Algorithm:"):
            algorithm = line[len("Algorithm:"):].strip()
        elif line.startswith("Cores Number:"):
            try:
                cores = int(line[len("Cores Number:"):].strip())
            except ValueError:
                cores = None
        elif line.startswith("Time Results (seconds):"):
            # Extract the list inside the brackets (e.g. [0.008732, 0.008613, ...])
            m = re.search(r"\[(.*?)\]", line)
            if m:
                times_str = m.group(1)
                times = [float(x.strip())
                         for x in times_str.split(",") if x.strip()]
        elif line.startswith("Average Time:"):
            m = re.search(r"([\d.]+)", line)
            if m:
                avg_time = float(m.group(1))
        # Any other lines (e.g. "Prime Result ...") are skipped.

    if algorithm is not None and cores is not None and times and avg_time is not None:
        return {"algorithm": algorithm, "cores": cores, "times": times, "avg_time": avg_time}
    else:
        return None


def plot_prime_gen_results(filename: str, save: bool = False):
    """
    Plot individual time measurements along with a horizontal dashed line
    representing the average time for each section.  
    Each section is plotted in its own color and marker, with the legend
    showing the algorithm name and cores number.
    """
    bit_size, results = read_results(filename)

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.set_title(f"Prime Generation Times (Target Bit Size: {bit_size})")
    ax.set_xlabel("Test Round")
    ax.set_ylabel("Time (seconds)")
    ax.grid(True)

    # set x ticks to integers from 1 to the number of time measurements
    ax.xaxis.set_major_locator(plt.MaxNLocator(integer=True))

    markers = ['o', 's', 'd', '^', 'v', 'x']
    colors = plt.rcParams['axes.prop_cycle'].by_key()['color']

    for idx, res in enumerate(results):
        algo = res["algorithm"]
        cores = res["cores"]
        times = res["times"]
        avg_time = res["avg_time"]
        x_vals = np.arange(1, len(times) + 1)

        color = colors[idx % len(colors)]
        marker = markers[idx % len(markers)]

        # Plot individual time measurements.
        ax.plot(x_vals, times, marker=marker, linestyle='solid', color=color,
                label=f"{algo} (cores: {cores})")
        # Draw a horizontal dashed line indicating the average time.
        # ax.hlines(avg_time, x_vals[0], x_vals[-1], colors=color, linestyles='dashed')

    ax.legend()
    plt.tight_layout()
    plt.show()

    if save:
        save_plot_figure(fig, output_dir, filename)


def save_plot_figure(fig, dir, filename):
    """Save the plot in SVG format in the default output directory."""
    filepath = os.path.join(dir, f"{filename}.{plot_ext}")
    fig.savefig(filepath, format=plot_ext)
    print(f"Figure saved as {filepath}")


def iZ(x: int, i: int) -> int:
    assert i in [-1, 1]  # i must be -1 or 1
    assert x > 0  # x must be positive
    return 6 * x + i


if __name__ == '__main__':
    # plot_prime_gen_results("random_prime_results_20250226103248", save=True)
    # plot_prime_gen_results("random_prime_results_20250226112408", save=True)
    # plot_prime_gen_results("random_prime_results_20250226112841", save=True)
    plot_prime_gen_results("random_prime_results_20250226114627", save=True)
