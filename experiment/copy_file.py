import os
import shutil

project_root = os.path.dirname(os.path.dirname(__file__))
experiment_dir = os.path.dirname(__file__)

from_dir = os.path.join(project_root, 'result-new-small')
to_dir = os.path.join(project_root, 'result-new-small-no-bound')
os.makedirs(to_dir, exist_ok=True)

for filename in sorted(os.listdir(from_dir)):
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

    if float(phi) >= 0 and bound == "False" and agent_per_task == "10":
        from_file = os.path.join(from_dir, filename)
        to_file = os.path.join(to_dir, filename)
        shutil.copy(from_file, to_file)
        print(filename)

