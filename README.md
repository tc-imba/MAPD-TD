# MAPF

## Build and Execution

### Docker (Deprecated)

```bash
docker build -t mapf .
docker run -dit --name mapf mapf:latest
docker exec -it mapf MAPF -h
docker exec -it mapf MAPF -d /root/MAPF/test-benchmark
```

### Local Environment

#### Build

```bash
# install cmake boost qt5
mkdir build
cmake ..
make
```

#### Generate Tests

Make sure you have build all binaries first, then

```bash
python3 experiment/generate.py
```

Thsi will generate all tasks and maps in the `test-benchmark` directory

#### Run Tests

First modify the arguments used in `experiment/test.py`, then run it

```bash
python3 experiment/test.py
```

#### Analyze Results

```bash
python3 experiment/analysis.py
python3 experiment/result.py
```

If you add new flags, these two files should also be updated.


## Usage

```bash
Multi Agent Path Finding

USAGE: ./MAPF [OPTIONS]

OPTIONS:

-a, --algorithm ARG     Algorithm (deprecated, only 0 working)
-b, --bound             Use Branch and Bound
-d, --data ARG          Data Path
-db, --deadline-bound   Use Deadline Bound
-h, --help              Display this Message.
-m, --mlabel            Use Multi Label
-o, --output ARG        Output File
-ra, --reserve-all      Reserve all
-re, --recalculate      Recalculate After Flex
-s, --sort              Use Sort
-t, --task ARG          Task File (Relative to Data Path)
-w, --window ARG        Window Size (0 means no limit)
--max-step ARG          Max Step
--phi ARG               Phi
--scheduler ARG         Scheduler (flex/edf)

EXAMPLES:

./MAPF --flex -a 0 --phi 0 -b -s -m -db -re -d test-benchmark -t task/well-formed-21-35-10-2.task -o auto
```


