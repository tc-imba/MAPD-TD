FROM ubuntu:18.04

ENV HOME="/root"

RUN apt-get update && \
    apt-get install -y g++ cmake libboost-all-dev

COPY ./ $HOME/MAPF

RUN mkdir -p $HOME/MAPF/build
WORKDIR $HOME/MAPF/build
RUN cmake -DCMAKE_BUILD_TYPE=Release ..
RUN make -j4

ENV PATH="/root/MAPF/build:$PATH"
