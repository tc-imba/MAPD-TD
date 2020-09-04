import os
import subprocess
import functools

project_root = os.path.dirname(os.path.dirname(__file__))
program = os.path.join(project_root, "cmake-build-release", "MAPF")
data_root = os.path.join(project_root, "test-benchmark")
result_dir = os.path.join(project_root, "result")
os.makedirs(result_dir, exist_ok=True)

def run(size=(21, 35), agent=10, task_per_agent=2, scheduler="flex",
        phi=0.2, bound=True, sort=True, mlabel=True):
    # os.chdir(project_root)
    base_filename = "%d-%d-%d-%d" % (size[0], size[1], agent, task_per_agent)
    task_filename = "task/well-formed-%s.task" % base_filename
    output_filename = "%s-%s-%s" % (base_filename, scheduler, phi)
    args = [
        program,
        "--data", data_root,
        "--task", task_filename,
        "--scheduler", scheduler,
        "--phi", str(phi),
    ]
    if bound:
        args.append("--bound")
        output_filename += "-bound"
    if sort:
        args.append("--sort")
        output_filename += "-sort"
    if mlabel:
        args.append("--mlabel")
        output_filename += "-mlabel"
    args += ["--output", os.path.join(result_dir, output_filename)]
    print(args)
    print(output_filename)
    subprocess.run(args, stderr=subprocess.PIPE)


def run_task(size=(21, 35), agent=10, task_per_agent=2, scheduler="flex"):
    for phi in [0, 0.05, 0.2]:
        _run = functools.partial(run, size=size, agent=agent, task_per_agent=task_per_agent, scheduler=scheduler,
                                 phi=phi)
        _run(bound=False, sort=False, mlabel=True)
        _run(bound=True, sort=False, mlabel=True)
        _run(bound=True, sort=True, mlabel=True)


def run_scheduler(size=(21, 35), agent=10, task_per_agent=2):
    run_task(size=size, agent=agent, task_per_agent=task_per_agent, scheduler="flex")
    run_task(size=size, agent=agent, task_per_agent=task_per_agent, scheduler="edf")


def main():
    for agent in [10, 20, 50]:
        run_scheduler(size=(21, 35), agent=agent, task_per_agent=2)


if __name__ == '__main__':
    main()
