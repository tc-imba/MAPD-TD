#include <iostream>
#include <fstream>
#include <limits>
#include <exception>
#include <algorithm>

#include "Manager.h"
#include "Solver.h"

int main() {
//    std::ofstream fout("4.txt");
//    std::cout.rdbuf(fout.rdbuf());

//    Manager manager("test-benchmark");
//    manager.loadScenarioFile("test/test.scen");

    Manager manager("test-benchmark");


/*    manager.loadScenarioFile("scen-even/room-32-32-4-even-1.scen");
    manager.loadScenarioFile("scen-even/random-32-32-10-even-1.scen");
    bool addConstraints = true;
    size_t successCount = 0, failedCount = 0, TLECount = 0;

    for (int i = 0; i < 200; i++) {
        auto scenario = manager.getScenario(i);
        if (!scenario) break;

        auto map = scenario->getMap();
        Solver solver(map);

        std::cout << "(" << scenario->getStart().first << "," << scenario->getStart().second << ") -> ("
                  << scenario->getEnd().first << "," << scenario->getEnd().second << ") ";

        solver.initScenario(scenario);
        size_t count = 0;
        while (!solver.success() && solver.step() && count < 100000) {
            ++count;
        }
        if (solver.success()) {
            ++successCount;
            auto vector = solver.constructPath();
            std::cout << vector[0]->leaveTime << std::endl;
            for (auto vNode:vector) {
                std::cout << "(" << vNode->pos.first << "," << vNode->pos.second << ") " << vNode->leaveTime
                          << std::endl;
            }
            if (addConstraints) {
                solver.addConstraints(vector);
            }
        } else {
            if (count == 0) {
                ++failedCount;
            } else {
                ++TLECount;
            }
            std::cout << "failed" << std::endl;
        }
        std::cout << count << " computing steps" << std::endl;
    }

    std::cout << successCount << " " << failedCount << " " << TLECount << std::endl;*/

//    auto map = manager.loadTaskFile("task/room-32-32-4-10-2.task");
    auto map = manager.loadTaskFile("task/well-formed-21-35-10-2.task");
    manager.leastFlexFirstAssign(map, 1);
    return 0;
}
