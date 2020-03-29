#include <iostream>
#include <limits>
#include <exception>

#include "Manager.h"
#include "Solver.h"

int main() {
//    Manager manager("test-benchmark");
//    manager.loadScenarioFile("test/test.scen");

    Manager manager("MAPF-benchmark");
    manager.loadScenarioFile("scen-even/room-32-32-4-even-1.scen");
    bool addConstraints = true;


    auto tempScenario = manager.getScenario(0);
    auto map = tempScenario->getMap();
    Solver solver(map);

    for (int i = 0; i < 100; i++) {
        auto scenario = manager.getScenario(i);
        std::cout << "(" << scenario->getStart().first << "," << scenario->getStart().second << ") -> ("
                  << scenario->getEnd().first << "," << scenario->getEnd().second << ") ";

        solver.initScenario(scenario);
        size_t count = 0;
        while (!solver.success() && solver.step() && count < 100000) {
            ++count;
        }
        if (solver.success()) {
            auto vector = solver.constructPath();
            if (addConstraints) {
                map->addNodeOccupied(vector[0]->pos, 0, vector[0]->leaveTime + 1);
                for (size_t j = 1; j < vector.size(); j++) {
                    size_t endTime = vector[j]->leaveTime + 1;
                    if (j == vector.size() - 1) endTime = std::numeric_limits<size_t>::max() / 2;
                    map->addNodeOccupied(vector[j]->pos, vector[j - 1]->leaveTime + 1, endTime);
                    auto dir = map->getDirectionByPos(vector[j - 1]->pos, vector[j]->pos);
                    if (dir == Map::Direction::NONE) {
                        throw std::runtime_error("");
                    }
                    map->addEdgeOccupied(vector[j - 1]->pos, dir, vector[j - 1]->leaveTime,
                                         vector[j - 1]->leaveTime + 1);
                }
            }
            std::cout << vector[0]->leaveTime << std::endl;
            /*for (auto vNode:vector) {
                std::cout << "(" << vNode->pos.first << "," << vNode->pos.second << ") " << vNode->leaveTime
                          << std::endl;
            }*/
        } else {
            std::cout << "failed" << std::endl;
        }
        std::cout << count << " computing steps" << std::endl;
    }

//    solver.addNodeOccupied({0,1},0, 3);
//    solver.addNodeOccupied({0,2},4, 5);

//     map->addEdgeOccupied({0, 1}, Map::Direction::RIGHT, 0, 3);

    return 0;
}
