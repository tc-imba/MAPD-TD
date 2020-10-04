import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

project_root = os.path.dirname(os.path.dirname(__file__))
experiment_dir = os.path.dirname(__file__)
plots_dir = os.path.join(experiment_dir, "plots")
os.makedirs(plots_dir, exist_ok=True)


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


# for each (map, phi), draw task_num as x, success rate as y
def plot_tasks_vs_success(df, size, phi):
    cond = (df['size'] == size) & (df['phi'] == phi) & (df['time_ms'] >= 0) & \
           (df['bound'] == True) & (df['sort'] == True) & (df['mlabel'] == True) & (df['reserve'] == False)
    new_df = df[cond]
    map_size = parse_map_size(new_df)
    phi_str = str(phi)
    filename = 'SR-%s-PHI-%s.png' % (map_size, phi_str.replace('-', 'n'))
    title = 'Success Rate: %s Map, Phi=%s' % (map_size, phi_str)
    print(filename, title)

    edf_df, flex_df, window_df = parse_methods(new_df)
    plt.figure()
    plt.plot(edf_df['task_num'], edf_df['task_success'] / edf_df['task_num'], label="edf", marker="o", linestyle="")
    plt.plot(flex_df['task_num'], flex_df['task_success'] / flex_df['task_num'], label="flex", marker="x", linestyle="")
    plt.plot(window_df['task_num'], window_df['task_success'] / window_df['task_num'], label="window", marker=".",
             linestyle="")
    plt.xlabel('tasks number')
    plt.ylabel('success rate')
    plt.legend()
    plt.title(title)
    plt.savefig(os.path.join(plots_dir, filename))
    plt.close()


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
    plt.figure()
    plt.plot(new_df['task_num_all'], new_df['time_ms_dynamic'] / new_df['time_ms_all'], label="dynamic reserve", marker="o", linestyle="")
    plt.plot(new_df['task_num_all'], new_df['task_num_all'] * 0 + 1, label="reserve all (baseline) ", linestyle="-")
    plt.xlabel('tasks number')
    plt.ylabel('running time multiplier')
    plt.legend()
    plt.title(title)
    plt.savefig(os.path.join(plots_dir, filename))
    plt.close()

def main():
    small_filename = os.path.join(experiment_dir, "small", "result.csv")
    large_filename = os.path.join(experiment_dir, "big", "result.csv")
    small_df = pd.read_csv(small_filename)
    large_df = pd.read_csv(large_filename)
    all_df = pd.concat([small_df, large_df])

    # pairs = all_df.groupby(['agent', 'task_per_agent']).first()
    # for index, row in pairs.iterrows():
    #     agent, task_per_agent = index
    #     plot_phi_vs_success(all_df, agent, task_per_agent)

    # pairs = all_df.groupby(['size', 'phi']).first()
    # for index, row in pairs.iterrows():
    #     size, phi = index
    #     plot_tasks_vs_success(all_df, size, phi)

    pairs = all_df.groupby(['size', 'phi']).first()
    for index, row in pairs.iterrows():
        size, phi = index
        plot_dummy_path(all_df, size, phi)


    # print(pair)
    # agent = pair[['agent']]
    # task_per_agent = pair[['task_per_agent']]
    # print(agent, task_per_agent)


if __name__ == '__main__':
    main()