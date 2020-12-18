import os
import subprocess
import functools

project_root = os.path.dirname(os.path.dirname(__file__))
program = os.path.join(project_root, "cmake-build-release", "MAPF-generate-well-formed-new")


def generate(x, y, agent, agent_per_task, seed, release=False):
    for seed in range(10,20):
        args = [
            program,
            "-x", str(x),
            "-y", str(y),
            "--agent", str(agent),
            "--agent-per-task", str(agent_per_task),
            "--seed", str(seed),
        ]
        if release:
            args.append("--release")
        print(' '.join(args))
        subprocess.run(args, stderr=subprocess.PIPE)


def main():
    os.chdir(project_root)
    # for m in [10, 20, 30, 40, 50]:
    #     for k in [2, 5, 10]:
    #         generate(5, 2, m, k, True)
    #         generate(5, 2, m, k, False)
    # for m in [60, 90, 120, 150, 180]:
    #     for k in [2, 5, 10]:
    #         generate(8, 3, m, k, True)
    #         generate(8, 3, m, k, False)
    generate(8, 3, 60, 10, False)

if __name__ == '__main__':
    main()
