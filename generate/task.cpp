//
// Created by liu on 24/4/2020.
//

#include <vector>
#include <random>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>

#include "../solver/Manager.h"
#include "../solver/Solver.h"

using namespace std;

size_t calculateDistance(Solver &solver) {
    size_t count = 0;
    while (!solver.success() && solver.step() && count < 100000) {
        ++count;
    }
    if (!solver.success()) {
        throw runtime_error("solver error");
    }
    auto path = solver.constructPath();
    return path[0]->leaveTime;
}

int main() {
    string dataPath = "test-benchmark";
    string mapName = "room-32-32-4";

    Manager manager(dataPath);
    auto map = manager.getMap(mapName + ".map");

    size_t agentNum = 10;
    size_t k = 2;
    size_t taskNum = agentNum * k;

    ofstream fout(dataPath + "/task/" + mapName + "-" + to_string(agentNum) + "-" + to_string(k) + ".task");


//    std::random_device rd;
    std::mt19937 g;

    Solver solver(map);

    std::vector<std::pair<size_t, size_t> > availablePoints;
    for (size_t i = 0; i < map->getHeight(); i++) {
        for (size_t j = 0; j < map->getWidth(); j++) {
            if ((*map)[i][j] == '.') {
                availablePoints.emplace_back(i, j);
            }
        }
    }
    std::shuffle(availablePoints.begin(), availablePoints.end(), g);

    ostringstream agentConfigs;
    ostringstream taskConfigs;

    for (size_t i = 0; i < agentNum; i++) {
        auto firstPoint = availablePoints.back();
        availablePoints.pop_back();
        agentConfigs << firstPoint.first << " " << firstPoint.second << endl;
        size_t dist = 0;
        for (size_t j = 0; j < k; j++) {
            auto evenPoint = availablePoints.back();
            availablePoints.pop_back();
            auto scenario = Scenario(i, map, firstPoint, evenPoint, 0);
            solver.initScenario(&scenario);
            dist += calculateDistance(solver);

            auto oddPoint = availablePoints.back();
            availablePoints.pop_back();
            scenario = Scenario(i, map, evenPoint, oddPoint, 0);
            solver.initScenario(&scenario);
            dist += calculateDistance(solver);

            taskConfigs << evenPoint.first << " " << evenPoint.second << " "
                        << oddPoint.first << " " << oddPoint.second << " " << dist << endl;

            firstPoint = oddPoint;
        }
//        cout << i << " " << dist << endl;
    }


    fout << agentNum << " " << k << endl;
    fout << mapName << ".map" << endl;
    fout << agentConfigs.str();
    fout << taskConfigs.str();

    fout.close();
    return 0;
}

