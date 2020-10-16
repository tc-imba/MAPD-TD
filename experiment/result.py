import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

project_root = os.path.dirname(os.path.dirname(__file__))
experiment_dir = os.path.dirname(__file__)
plots_dir = os.path.join(experiment_dir, "plots")
os.makedirs(plots_dir, exist_ok=True)

pd.set_option("display.max_rows", None, "display.max_columns", None, 'display.width', None)


def parse_map_size(df: pd.DataFrame):
    arr = df['size'].unique()
    assert len(arr) == 1
    if arr[0] == '21x35':
        return 'S'
    else:
        return 'L'


def parse_methods(df: pd.DataFrame):
    edf_df = df[(df['scheduler'] == 'edf') & (df['window'] == 0)]
    flex_df = df[(df['scheduler'] == 'flex') & (df['window'] == 0)]
    window_df = df[(df['scheduler'] == 'flex') & (df['window'] == 20)]
    return edf_df, flex_df, window_df


def parse_branch_and_bound(df: pd.DataFrame):
    sort_df = df[(df['bound'] == True) & (df['sort'] == True)].set_index(['agent', 'task_per_agent'])
    bound_df = df[(df['bound'] == True) & (df['sort'] == False)].set_index(['agent', 'task_per_agent'])
    no_df = df[(df['bound'] == False) & (df['sort'] == False)].set_index(['agent', 'task_per_agent'])
    new_bound_df = bound_df.join(sort_df, on=['agent', 'task_per_agent'], lsuffix='_bound', rsuffix='_sort')
    new_no_df = no_df.join(sort_df, on=['agent', 'task_per_agent'], lsuffix='_no', rsuffix='_sort')
    return sort_df, new_bound_df, new_no_df


def parse_dummy_path(df: pd.DataFrame):
    reserve_dynamic_df = df[df['reserve'] == False].set_index(['agent', 'task_per_agent'])
    reserve_all_df = df[df['reserve'] == True].set_index(['agent', 'task_per_agent'])
    new_df = reserve_dynamic_df.join(reserve_all_df, on=['agent', 'task_per_agent'], lsuffix='_dynamic', rsuffix='_all')
    return new_df


# for each (agent, task_per_agent), draw phi as x, success rate as y
def plot_phi_vs_success(df, agent, task_per_agent):
    cond = (df['agent'] == agent) & (df['task_per_agent'] == task_per_agent) & (df['time_ms'] >= 0) & \
           (df['bound'] == True) & (df['sort'] == True) & (df['mlabel'] == True) & (df['reserve'] == False)
    new_df = df[cond]
    map_size = parse_map_size(new_df)
    filename = 'SR-%s-M-%d-N-%d.png' % (map_size, agent, task_per_agent)
    title = 'Success Rate: %s Map, M=%d, N=%d' % (map_size, agent, task_per_agent)
    print(filename, title)

    edf_df, flex_df, window_df = parse_methods(new_df)
    plt.figure()
    plt.plot(edf_df['phi'], edf_df['task_success'] / edf_df['task_num'], label="edf", marker="o", linestyle="")
    plt.plot(flex_df['phi'], flex_df['task_success'] / flex_df['task_num'], label="flex", marker="x", linestyle="")
    plt.plot(window_df['phi'], window_df['task_success'] / window_df['task_num'], label="window", marker=".",
             linestyle="")
    plt.xlabel('phi')
    plt.ylabel('success rate')
    plt.legend()
    plt.title(title)
    plt.savefig(os.path.join(plots_dir, filename))
    plt.close()


# for each (agent, task_per_agent), draw phi as x, success rate as y
def plot_phi(df):
    cond = (df['time_ms'] >= 0) & \
           (df['bound'] == True) & (df['sort'] == True) & (df['mlabel'] == True) & (df['reserve'] == False)
    new_df = df[cond]
    map_size = parse_map_size(new_df)
    filename = 'SR-%s.png' % map_size
    title = 'Success Rate: %s Map' % map_size
    print(filename, title)

    edf_df, flex_df, window_df = parse_methods(new_df)
    edf_df = edf_df.groupby(['phi'], as_index=False).mean()
    flex_df = flex_df.groupby(['phi'], as_index=False).mean()
    window_df = window_df.groupby(['phi'], as_index=False).mean()
    print("edf", edf_df)
    print("flex", flex_df)
    print("window", window_df)

    plt.figure()
    plt.plot(edf_df['phi'], edf_df['success_rate'], label="edf", marker="o", linestyle="")
    plt.plot(flex_df['phi'], flex_df['success_rate'], label="flex", marker="x", linestyle="")
    plt.plot(window_df['phi'], window_df['success_rate'], label="window", marker=".",
             linestyle="")
    plt.xlabel('phi')
    plt.ylabel('success rate')
    plt.legend()
    plt.title(title)
    plt.savefig(os.path.join(plots_dir, filename))
    plt.close()


# for each (map, phi, k), draw task_num as x, success rate as y
def plot_tasks_vs_success(df, size, phi, k):
    df['ratio'] = df['task_success'] / df['task_num']
    cond = (df['size'] == size) & (df['phi'] == phi) & (df['task_per_agent'] == k) & (df['time_ms'] >= 0) & \
           (df['bound'] == True) & (df['sort'] == True) & (df['mlabel'] == True) & (df['reserve'] == False)
    new_df = df[cond]
    map_size = parse_map_size(new_df)
    phi_str = str(phi)
    filename = 'SR-%s-PHI-%s-k-%s.png' % (map_size, phi_str.replace('-', 'n'), k)
    title = 'Success Rate: %s Map, Phi=%s, k=%s' % (map_size, phi_str, k)
    print(filename, title)

    edf_df, flex_df, window_df = parse_methods(new_df)
    plt.figure()
    # plt.plot(edf_df['task_num'], edf_df['task_success'] / edf_df['task_num'], label="edf", marker="o", linestyle="")
    plt.plot(flex_df['agent'], flex_df['task_success'] / flex_df['task_num'], label="flex", marker="x", linestyle="")
    plt.plot(window_df['agent'], window_df['task_success'] / window_df['task_num'], label="window", marker=".",
             linestyle="")
    plt.xlabel('agent number')
    plt.ylabel('success rate')
    plt.legend()
    plt.title(title)
    plt.savefig(os.path.join(plots_dir, filename))
    plt.close()

    print(new_df[['agent', 'task_per_agent', 'scheduler', 'window', 'task_num', 'task_success', 'ratio', 'time_ms']])

def plot_branch_and_bound(df, size, phi):
    cond = (df['size'] == size) & (df['phi'] == phi) & (df['time_ms'] >= 0) & \
           (df['mlabel'] == True) & (df['reserve'] == False) & \
           (df['scheduler'] == 'flex') & (df['window'] == 0)
    new_df = df[cond]
    map_size = parse_map_size(new_df)
    phi_str = str(phi)
    filename = 'BB-%s-PHI-%s.png' % (map_size, phi_str.replace('-', 'n'))
    title = 'Branch and Bound: %s Map, Phi=%s' % (map_size, phi_str)
    print(filename, title)

    sort_df, bound_df, no_df = parse_branch_and_bound(new_df)
    no_df.reset_index(level=no_df.index.names, inplace=True)
    no_df['ratio'] = no_df['time_ms_no'] / no_df['time_ms_sort']

    plt.figure()
    plt.plot(sort_df['task_num'], sort_df['task_num'] * 0 + 1, label="branch and bound (baseline) ", linestyle="-")
    # plt.plot(bound_df['task_num_sort'], bound_df['time_ms_bound'] / bound_df['time_ms_sort'], label="only bound", marker="o", linestyle="")
    plt.plot(no_df['task_num_sort'], no_df['time_ms_no'] / no_df['time_ms_sort'], label="no branch and bound",
             marker="x", linestyle="")

    plt.yscale('log')
    plt.xlabel('tasks number')
    plt.ylabel('running time multiplier')
    plt.legend()
    plt.title(title)
    plt.savefig(os.path.join(plots_dir, filename))
    plt.close()

    print(no_df[['agent', 'task_per_agent', 'task_num_sort', 'time_ms_no', 'time_ms_sort', 'ratio']])


def plot_dummy_path(df, size, phi):
    cond = (df['size'] == size) & (df['phi'] == phi) & (df['time_ms'] >= 0) & \
           (df['bound'] == True) & (df['sort'] == True) & (df['mlabel'] == True) & \
           (df['scheduler'] == 'flex') & (df['window'] == 0)
    new_df = df[cond]
    parse_dummy_path(new_df)
    map_size = parse_map_size(new_df)
    phi_str = str(phi)
    filename = 'DP-%s-PHI-%s.png' % (map_size, phi_str.replace('-', 'n'))
    title = 'Dummy Path: %s Map, Phi=%s' % (map_size, phi_str)
    print(filename, title)

    new_df = parse_dummy_path(new_df)
    new_df.reset_index(level=new_df.index.names, inplace=True)
    new_df['ratio'] = new_df['time_ms_all'] / new_df['time_ms_dynamic']

    plt.figure()
    plt.plot(new_df['task_num_dynamic'], new_df['task_num_dynamic'] * 0 + 1, label="dynamic reserve (baseline) ",
             linestyle="-")
    for agent in sorted(new_df['agent'].unique()):
        plot_df = new_df[new_df['agent'] == agent]
        label = "reserve all (N=%d)" % agent
        plt.plot(plot_df['task_num_dynamic'], plot_df['ratio'], label=label, marker="x", linestyle="")

    plt.xlabel('tasks number')
    plt.ylabel('running time multiplier')
    plt.legend()
    plt.title(title)
    plt.savefig(os.path.join(plots_dir, filename))
    plt.close()

    print(new_df[['agent', 'task_per_agent', 'task_num_all', 'time_ms_all', 'time_ms_dynamic', 'ratio']])


def main():
    small_filename = os.path.join(experiment_dir, "small", "result.csv")
    large_filename = os.path.join(experiment_dir, "big", "result.csv")
    small_df = pd.read_csv(small_filename)
    small_df['success_rate'] = small_df['task_success'] / small_df['task_num']
    large_df = pd.read_csv(large_filename)
    large_df['success_rate'] = large_df['task_success'] / large_df['task_num']
    all_df = pd.concat([small_df, large_df])

    # plot_phi(small_df)
    # plot_phi(large_df)

    # pairs = all_df.groupby(['agent', 'task_per_agent']).first()
    # for index, row in pairs.iterrows():
    #     agent, task_per_agent = index
    #     plot_phi_vs_success(all_df, agent, task_per_agent)
    #
    pairs = all_df.groupby(['size', 'phi']).first()
    for index, row in pairs.iterrows():
        size, phi = index
        for k in [2, 5, 10]:
            plot_tasks_vs_success(all_df, size, phi, k)

    pairs = all_df.groupby(['size', 'phi']).first()
    for index, row in pairs.iterrows():
        size, phi = index
        plot_dummy_path(all_df, size, phi)

    pairs = small_df.groupby(['size', 'phi']).first()
    for index, row in pairs.iterrows():
        size, phi = index
        plot_branch_and_bound(small_df, size, phi)

    # print(pair)
    # agent = pair[['agent']]
    # task_per_agent = pair[['task_per_agent']]
    # print(agent, task_per_agent)


if __name__ == '__main__':
    main()
