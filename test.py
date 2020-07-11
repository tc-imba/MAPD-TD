import subprocess


def run_program(name, args):
    p = subprocess.run(args=args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    lines = p.stdout.decode('utf-8').splitlines(keepends=False)
    time = lines[-1][6:]
    print(name, time)


def run_task(task, name):
    program = 'cmake-build-release/MAPF'
    args = [program, '-t', task]
    run_program(name + '0-bound', args + ['-a', '0', '-b'])
    run_program(name + '0-bound-sort', args + ['-a', '0', '-b', '-s'])
    run_program(name + '0-bound-mlabel', args + ['-a', '0', '-b', '-m'])
    run_program(name + '0-bound-sort-mlabel', args + ['-a', '0', '-b', '-s', ' -m'])
    run_program(name + '1-bound', args + ['-a', '1', '-b'])
    run_program(name + '1-bound-sort', args + ['-a', '1', '-b', '-s'])
    run_program(name + '1-bound-mlabel', args + ['-a', '1', '-b', '-m'])
    run_program(name + '1-bound-sort-mlabel', args + ['-a', '1', '-b', '-s', ' -m'])


def main():
    run_task("task/well-formed-21-35-10-2.task", "agent-10-task-20-")
    run_task("task/well-formed-21-35-20-2.task", "agent-10-task-40-")


if __name__ == '__main__':
    main()
