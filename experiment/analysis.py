import os

name = "result-big-new"
project_root = os.path.dirname(os.path.dirname(__file__))
experiment_dir = os.path.dirname(__file__)
result_dir = os.path.join(project_root, name)


def parse(filename):
    args = filename.split("-")
    size = args[0] + "x" + args[1]
    agent = args[2]
    agent_per_task = args[3]
    scheduler = args[4]
    phi = args[5].replace('n', '-')
    bound = str("bound" in args)
    sort = str("sort" in args)
    mlabel = str("mlabel" in args)
    # print(size, agent, agent_per_task, phi, scheduler, bound, sort, mlabel)

    task_num = int(agent) * int(agent_per_task)
    task_success = 0
    time_ms = -1
    with open(os.path.join(result_dir, filename)) as f:
        for line in f.readlines():
            if "complete" in line:
                task_success += 1
            if "time: " in line:
                time_ms = line[6:-3]
    # print(task_success, task_num, time_ms)
    return [size, agent, agent_per_task, phi, scheduler, bound, sort, mlabel,
            str(task_num), str(task_success), str(time_ms)]


def main():
    header = ["size", "agent", "task_per_agent", "phi", "scheduler",
              "bound", "sort", "mlabel", "task_num", "task_success", "time_ms"]

    data = []
    result_dict = {}

    with open(os.path.join(experiment_dir, name + ".csv"), "w") as f:
        f.write(",".join(header) + "\n")
        for filename in sorted(os.listdir(result_dir)):
            row = parse(filename)
            if row[5] == 'True' and row[6] == 'True' and row[7] == 'True':
                data.append(row)
                result_dict[row[3]] = 0
            f.write(",".join(row) + "\n")

    data_dict = {}
    for row in data:
        key = '-'.join(row[:4])
        if key not in data_dict:
            data_dict[key] = row[-2]
        else:
            prev = data_dict[key]
            if row[-2] == prev:
                continue
            if (row[-2] > prev and row[4] == 'flex') or (row[-2] < prev and row[4] == 'edf'):
                result_dict[row[3]] += 1
            else:
                result_dict[row[3]] -= 1

    print(result_dict)


if __name__ == '__main__':
    main()
