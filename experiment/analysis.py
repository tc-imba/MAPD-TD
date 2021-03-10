import os
import matplotlib.pyplot as plt

name = "result"
project_root = os.path.dirname(os.path.dirname(__file__))
experiment_dir = os.path.dirname(__file__)
result_dir = os.path.join(project_root, name)


def parse(filename):
    args = filename.split("-")
    size = args[0] + "x" + args[1]
    agent = args[2]
    agent_per_task = args[3]
    seed = args[4]
    scheduler = args[5]
    phi = args[6].replace('n', '-')
    window = args[7]
    bound = str("bound" in args)
    sort = str("sort" in args)
    mlabel = str("mlabel" in args)
    reserve_all = str("reserve" in args)
    skip = str("skip" in args)
    task_bound = str("tb" in args)
    recalc = str("recalc" in args)
    nearest = str("nearest" in args)
    ec = str("ec" in args)
    # print(size, agent, agent_per_task, phi, scheduler, bound, sort, mlabel)

    task_num = int(agent) * int(agent_per_task)
    task_success = 0
    reserve = 0
    reserve_a = 0
    reserve_b = 1
    time_ms = -1
    with open(os.path.join(result_dir, filename)) as f:
        for line in f.readlines():
            if "complete" in line:
                task_success += 1
            if "time: " in line:
                time_ms = line[6:-3]
            if "reserve" in line and "nearest" not in line:
                reserve += 1
                arr = line.split(' ')
                if len(arr) >= 3:
                    if arr[-1] == '1':
                        reserve_a += 1
                    elif arr[-1] == '2':
                        reserve_b += 1

    success_rate = task_success / task_num
    # print(task_success, task_num, time_ms)
    return [size, agent, agent_per_task, seed, phi, scheduler, window, bound, sort, mlabel, skip, recalc, nearest, ec,
            task_bound, reserve_all, str(task_num), str(task_success), str(success_rate), str(reserve), str(reserve_a),
            str(reserve_b), str(time_ms)]


def plot(phi, data):
    filename = os.path.join(experiment_dir, name + '-' + phi.replace('-', 'n') + ".png")
    data.sort(key=lambda x: int(x[0]))
    task = list(map(lambda x: int(x[0]), data))
    edf = list(map(lambda x: float(x[1]["edf-0"]) / int(x[0]), data))
    flex = list(map(lambda x: float(x[1]["flex-0"]) / int(x[0]), data))
    window = list(map(lambda x: float(x[1]["flex-20"]) / int(x[0]), data))
    plt.figure()
    plt.plot(task, edf, label="edf", marker="o", linestyle="")
    plt.plot(task, flex, label="flex", marker="x", linestyle="")
    plt.plot(task, window, label="window", marker=".", linestyle="")
    plt.xlabel('tasks')
    plt.ylabel('success rate')
    plt.legend()
    plt.title(name + ': phi = ' + phi)
    plt.savefig(filename)


def main():
    header = ["size", "agent", "task_per_agent", "seed", "phi", "scheduler", "window", "bound", "sort",
              "mlabel", "skip", "recalc", "nearest", "ec",
              "task_bound", "reserve_all", "task_num", "task_success", "success_rate",
              "reserve", "reserve_a", "reserve_b", "time_ms"]

    data = {}
    result_dict = {}

    with open(os.path.join(experiment_dir, name + ".csv"), "w") as f:
        f.write(",".join(header) + "\n")
        for filename in sorted(os.listdir(result_dir)):
            row = parse(filename)
            f.write("%s\n" % ','.join(row))
    exit(0)

    for filename in sorted(os.listdir(result_dir)):
        row = parse(filename)
        # print(row)
        if float(row[-1]) < 0:
            continue
        row_signature = ','.join(row[:3] + row[4:19])
        row_data = row[-4:]
        if row_signature not in data:
            data[row_signature] = []
        data[row_signature].append(row_data)
        # if row[5] == 'True' and row[6] == 'True' and row[7] == 'True':
        #     data.append(row)
        result_dict[row[4]] = []

    for key, value in data.items():
        temp = [0] * len(value[0])
        for x in value:
            for i in range(len(x)):
                temp[i] += float(x[i]) / len(value)
        data[key] = temp

    with open(os.path.join(experiment_dir, name + ".csv"), "w") as f:
        f.write(",".join(header) + "\n")
        for key, value in data.items():
            f.write("%s,%f,%f,%f,%f\n" % (key, value[0], value[1], value[2], value[3]))

    exit(0)

    data_dict = {}
    for _key, value in data.items():
        row = _key.split(',')
        if row[-2] == 'True':
            continue
        key = ','.join(row[:4])
        if key not in data_dict:
            data_dict[key] = []

        data_dict[key].append((row[4] + '-' + row[5], value[0]))
        # else:
        #     prev = data_dict[key]
        #     if row[4] == 'flex':
        #         result_dict[row[3]].append((row[-1], prev, value[0]))
        #     else:
        #         result_dict[row[3]].append((row[-1], value[0], prev))

    for _key, value in data_dict.items():
        row = _key.split(',')
        phi = row[-1]
        tasks = int(row[1]) * int(row[2])
        result_dict[phi].append((tasks, dict(value)))

    print(result_dict)

    for i, (phi, data) in enumerate(result_dict.items()):
        plot(phi, data)


if __name__ == '__main__':
    main()
