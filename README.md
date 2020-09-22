## Docker

```bash
docker build -t mapf .
docker run -dit --name mapf mapf:latest
docker exec -it mapf MAPF -h
docker exec -it mapf MAPF -d /root/MAPF/test-benchmark
```

## Build

```bash
mkdir build
cmake ..
make
```

## Run

```bash
./MAPF     # cli version
./MAPF-ui  # gui version
```

