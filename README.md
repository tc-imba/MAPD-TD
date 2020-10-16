# MAPF

## Build and Execution

### Docker

```bash
docker build -t mapf .
docker run -dit --name mapf mapf:latest
docker exec -it mapf MAPF -h
docker exec -it mapf MAPF -d /root/MAPF/test-benchmark
```

### Local Environment

```bash
# install cmake boost qt5
mkdir build
cmake ..
make
./MAPF     # cli version
./MAPF-ui  # gui version
```

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


