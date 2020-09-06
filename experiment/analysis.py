import os

name = "result"
project_root = os.path.dirname(os.path.dirname(__file__))
experiment_dir = os.path.dirname(__file__)
result_dir = os.path.join(project_root, name)


def parse(filename):
    args = filename.split("-")
    size = args[0] + "x" + args[1]
    agent = args[2]
    agent_per_task = args[3]
    phi = args[4]
    scheduler = args[5]
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
    header = ["size", "agent", "agent_per_task", "phi", "scheduler",
              "bound", "sort", "mlabel", "task_num", "task_success", "time_ms"]

    with open(os.path.join(experiment_dir, name+".csv"), "w") as f:
        f.write(",".join(header) + "\n")
        for filename in sorted(os.listdir(result_dir)):
            row = parse(filename)
            f.write(",".join(row) + "\n")


if __name__ == '__main__':
    main()
