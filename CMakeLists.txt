cmake_minimum_required(VERSION 3.15)
project(MAPF)

set(CMAKE_CXX_STANDARD 14)

add_executable(MAPF main.cpp Map.cpp Map.h Scenario.cpp Scenario.h Manager.cpp Manager.h Solver.cpp Solver.h)