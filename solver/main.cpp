#include <iostream>
#include "Manager.h"
#include "Solver.h"

int main() {
//    Manager manager("test-benchmark");
//    manager.loadScenarioFile("test/test.scen");

    Manager manager("MAPF-benchmark");
    manager.loadScenarioFile("scen-even/room-32-32-4-even-1.scen");


    auto scenario = manager.getScenario();
    Solver solver(scenario->getMap());

//    solver.addNodeOccupied({0,1},0, 3);
//    solver.addNodeOccupied({0,2},4, 5);

    solver.getConstraints().addEdgeOccupied({0, 1}, Map::Direction::RIGHT, 0, 3);

    solver.initScenario(scenario);
    while (!solver.success() && solver.step()) {}
    if (solver.success()) {
        auto vector = solver.constructPath();
        for (auto vNode:vector) {
            std::cout << "(" << vNode->pos.first << "," << vNode->pos.second << ") " << vNode->leaveTime << std::endl;
        }
    } else {
        std::cout << "failed" << std::endl;
    }

    return 0;
}
