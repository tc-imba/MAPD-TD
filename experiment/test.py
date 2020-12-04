import asyncio
import os
import subprocess
import functools

project_root = os.path.dirname(os.path.dirname(__file__))
program = os.path.join(project_root, "cmake-build-release", "MAPF")
data_root = os.path.join(project_root, "test-benchmark")
result_dir = os.path.join(project_root, "result")
os.makedirs(result_dir, exist_ok=True)
workers = 20

TIMEOUT = 36000

MAP = "large"

if MAP == "small":
    MAP_SIZE = (21, 35)
    AGENTS = [10, 20, 30, 40, 50]
    EXPERIMENT_TIMES = 10
else:
    MAP_SIZE = (33, 46)
    # AGENTS = [60, 90, 120, 150]
    # AGENTS = [60, 90, 120, 150, 180]
    AGENTS = [180]
    EXPERIMENT_TIMES = 10
TASKS_PER_AGENT = [2, 5, 10]
# TASKS_PER_AGENT = [10]
PHIS = [-0.25, -0.1, 0, 0.1, 0.25]
# PHIS_180 = [-0.25, -0.1, 0.25]

EXPERIMENT_JOBS = EXPERIMENT_TIMES * len(AGENTS) * len(TASKS_PER_AGENT) * len(PHIS) * 2
count = 0


async def run(size=(21, 35), agent=10, task_per_agent=2, seed=0, scheduler="flex", window_size=0,
              phi=0.2, bound=True, sort=True, mlabel=True, reserve=False, skip=False, task_bound=False):
    # os.chdir(project_root)
    base_filename = "%d-%d-%d-%d-%d" % (size[0], size[1], agent, task_per_agent, seed)
    task_filename = "task/well-formed-%s.task" % base_filename
    phi_output = phi >= 0 and str(phi) or 'n' + str(-phi)
    output_filename = "%s-%s-%s-%s" % (base_filename, scheduler, phi_output, window_size)
    args = [
        program,
        "--data", data_root,
        "--task", task_filename,
        "--scheduler", scheduler,
        "--phi", str(phi),
        "-w", str(window_size),
        "--recalculate",
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
    if reserve:
        args.append("--reserve-all")
        output_filename += "-reserve"
    if task_bound:
        args.append("--task-bound")
        output_filename += "-tb"
    if skip:
        args.append("-skip")
        output_filename += "-skip"
    args += ["--output", os.path.join(result_dir, output_filename)]
    # print(args)

    global workers, count
    while workers <= 0:
        await asyncio.sleep(1)

    workers -= 1
    p = None
    try:
        p = await asyncio.create_subprocess_exec(program, *args, stderr=subprocess.PIPE)
        await asyncio.wait_for(p.communicate(), timeout=TIMEOUT)
    except asyncio.TimeoutError:
        print('timeout!')
    except:
        pass
    try:
        p.kill()
    except:
        pass
    workers += 1
    count += 1
    print('%s (%d/%d)' % (output_filename, count, EXPERIMENT_JOBS))


async def run_task(size=(21, 35), agent=10, task_per_agent=2, scheduler="flex", window_size=0):
    tasks = []
    for seed in range(EXPERIMENT_TIMES):
        # if agent == 180:
        #     _PHIS = PHIS_180
        # else:
        #     _PHIS = PHIS
        for phi in PHIS:
            _run = functools.partial(run, size=size, agent=agent, task_per_agent=task_per_agent, seed=seed,
                                     scheduler=scheduler, window_size=window_size, phi=phi)
            tasks += [
                # _run(bound=True, sort=True, mlabel=True, reserve=True),
                # _run(bound=True, sort=True, mlabel=True, reserve=False, skip=True),
                _run(bound=True, sort=True, mlabel=True, reserve=False, skip=True, task_bound=True),
                # _run(bound=False, sort=False, mlabel=True, reserve=False),
                # _run(bound=True, sort=False, mlabel=True, reserve=False),
            ]
    await asyncio.gather(*tasks)


async def run_scheduler(size=(21, 35), agent=10, task_per_agent=2):
    await asyncio.gather(
        # run_task(size=size, agent=agent, task_per_agent=task_per_agent, scheduler="flex"),
        # run_task(size=size, agent=agent, task_per_agent=task_per_agent, scheduler="flex", window_size=20),
        run_task(size=size, agent=agent, task_per_agent=task_per_agent, scheduler="edf"),
    )


async def run_agent(size=(21, 35), agent=10):
    tasks = [run_scheduler(size=size, agent=agent, task_per_agent=i) for i in TASKS_PER_AGENT]
    await asyncio.gather(*tasks)


async def main():
    tasks = [run_agent(size=MAP_SIZE, agent=agent) for agent in AGENTS]
    await asyncio.gather(*tasks)


if __name__ == '__main__':
    asyncio.run(main())
