import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import math
import sys
from to_precision import std_notation, sci_notation

project_root = os.path.dirname(os.path.dirname(__file__))
experiment_dir = os.path.dirname(__file__)
plots_dir = os.path.join(experiment_dir, "plots")
os.makedirs(plots_dir, exist_ok=True)


def plot(baseline_df: pd.DataFrame, exp_df: pd.DataFrame, value, ylabel, title):
    phis = baseline_df["phi"].unique()
    agents = baseline_df["agent"].unique()
    plt.figure(dpi=200)
    for phi in sorted(phis, reverse=True):
        new_df = exp_df[exp_df['phi'] == phi]
        label = "phi=%s" % phi
        plt.plot(new_df["agent"], new_df[value], label=label, marker="x", linestyle='-')

        new_df = baseline_df[baseline_df['phi'] == phi]
        label = "phi=%s (baseline)" % phi
        plt.plot(new_df["agent"], new_df[value], label=label, marker="o", linestyle='--')

    if value == "time_ms":
        plt.yscale('log')

    plt.xticks(agents)
    plt.xlabel('agent number')
    plt.ylabel(ylabel)
    # plt.legend(bbox_to_anchor=(1, 0.5))
    plt.title(title)
    plt.tight_layout()
    # plt.show()

    filename = title.replace(' ', '_') + ".png"
    plt.savefig(os.path.join(plots_dir, filename))
    plt.close()


def main():
    baseline_s_df = pd.read_csv(os.path.join(experiment_dir, "result-baseline-S.csv"))
    baseline_l_df = pd.read_csv(os.path.join(experiment_dir, "result-baseline-L.csv"))
    exp_s_df = pd.read_csv(os.path.join(experiment_dir, "result-exp-S.csv"))
    exp_l_df = pd.read_csv(os.path.join(experiment_dir, "result-exp-L.csv"))

    plot(baseline_s_df, exp_s_df, "ratio", "success ratio", "success ratio in small warehouse")
    plot(baseline_s_df, exp_s_df, "time_ms", "runtime (ms)", "runtime in small warehouse")
    plot(baseline_l_df, exp_l_df, "ratio", "success ratio", "success ratio in large warehouse")
    plot(baseline_l_df, exp_l_df, "time_ms", "runtime (ms)", "runtime in large warehouse")


if __name__ == '__main__':
    main()
