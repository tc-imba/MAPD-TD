cmake_minimum_required(VERSION 3.10)
project(MAPF)

set(CMAKE_CXX_STANDARD 14)

find_package(Boost)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(
        MAPF
        solver/main.cpp solver/Map.cpp solver/Map.h
        solver/Scenario.cpp solver/Scenario.h solver/Manager.cpp
        solver/Manager.h solver/Solver.cpp solver/Solver.h
)

add_executable(MAPF-generate generate/main.cpp)
#add_executable(MAPF-generate-task generate/task.cpp solver/Map.cpp solver/Manager.cpp
#        solver/Scenario.cpp solver/Solver.cpp)

add_executable(MAPF-generate-well-formed generate/well-formed.cpp solver/Map.cpp solver/Manager.cpp
        solver/Scenario.cpp solver/Solver.cpp)

#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)
#
#if(CMAKE_VERSION VERSION_LESS "3.7.0")
#    set(CMAKE_INCLUDE_CURRENT_DIR ON)
#endif()

#find_package(Qt5 COMPONENTS Widgets REQUIRED)
#
#add_executable(MAPF-ui
#        ui/main.cpp ui/GraphWidget.cpp ui/GraphWidget.h ui/Node.cpp ui/Node.h
#        solver/Map.cpp solver/Map.h solver/Scenario.cpp solver/Scenario.h
#        solver/Manager.cpp solver/Manager.h solver/Solver.cpp solver/Solver.h
#        ui/Edge.cpp ui/Edge.h)
#target_link_libraries(MAPF-ui Qt5::Widgets)

