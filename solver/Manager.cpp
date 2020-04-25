//
// Created by liu on 2020/3/7.
//

#include "Manager.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

Manager::Manager(std::string dataPath) : dataPath(std::move(dataPath)) {

}

Map *Manager::loadMapFile(const std::string &mapName) {
    auto filePath = dataPath + "/map/" + mapName;
    auto map = std::make_unique<Map>(filePath);
    auto mapPtr = map.get();
    this->maps.emplace(mapName, std::move(map));
    filePath = dataPath + "/constraints/" + mapName;
    if (mapPtr->loadConstraints(filePath)) {
        std::cerr << "constraints loaded" << std::endl;
    } else {
        std::cerr << "no constraints" << std::endl;
    }
    return mapPtr;
}

void Manager::loadScenarioFile(const std::string &filename) {
    auto filePath = dataPath + "/" + filename;
    std::ifstream fin(filePath);
    if (!fin.is_open()) {
        throw std::runtime_error("scenario file not found");
    }
    std::string line, temp, version;
    std::istringstream iss;

    std::getline(fin, line);
    iss.str(line);
    iss >> temp >> version;
    if (temp != "version") {
        throw std::runtime_error("scenario version error");
    }

    while (std::getline(fin, line)) {
        if (line.length() == 0) continue;
        iss.clear();
        iss.str(line);
        std::string mapName;
        size_t bucket, width, height;
        std::pair<size_t, size_t> start, end;
        double optimal;
        iss >> bucket >> mapName >> width >> height
            >> start.first >> start.second >> end.first >> end.second >> optimal;
        auto map = this->getMap(mapName);
        if (map->getHeight() != height || map->getWidth() != width) {
            throw std::runtime_error("scenario map size error");
        }
        auto scenario = std::make_unique<Scenario>(bucket, map, start, end, optimal);
        this->scenarios.emplace_back(std::move(scenario));
    }

    std::cerr << "Scenario " << filename << " imported" << std::endl;
}

Map *Manager::getMap(const std::string &mapName) {
    auto it = this->maps.find(mapName);
    if (it != this->maps.end()) return it->second.get();
    return this->loadMapFile(mapName);
}

Scenario *Manager::getScenario(size_t index) {
    if (index >= 0 && index < this->scenarios.size()) {
        return this->scenarios[index].get();
    }
    return nullptr;
}

Map *Manager::loadTaskFile(const std::string &filename) {
    auto filePath = dataPath + "/" + filename;
    std::ifstream fin(filePath);
    if (!fin.is_open()) {
        throw std::runtime_error("task file not found");
    }
    size_t agentNum = 10;
    size_t k = 2;
//    size_t taskNum = agentNum * k;
    fin >> agentNum >> k;

    std::string mapName;
    fin >> mapName;
    auto map = this->getMap(mapName);

    std::pair<size_t, size_t> pos;
    for (size_t i = 0; i < agentNum; i++) {
        fin >> pos.first >> pos.second;
        agents.emplace_back(pos);
    }

    for (size_t i = 0; i < agentNum; i++) {
        for (size_t j = 0; j < k; j++) {
            std::pair<size_t, size_t> start, end;
            size_t dist;
            fin >> start.first >> start.second >> end.first >> end.second >> dist;
            auto task = std::make_unique<Scenario>(i * k + j, map, start, end, dist);
            tasks.emplace_back(std::move(task));
        }
    }
    return map;
}

void Manager::leastFlexFirstAssign(Map *map) {
    Solver solver(map);

    // add node constraints for parking locations
    for (auto &agent: agents) {
        map->addNodeOccupied(agent.originPos, agent.lastTimeStamp, std::numeric_limits<size_t>::max() / 2);
    }

    while (!tasks.empty()) {
        computeFlex(solver, 1);
        selectTask(map);
//        exit(0);
//        break;
    }

}


void Manager::assignTask(Map *map, Agent &agent, std::vector<PathNode> &vector) {
    if (vector[0].pos != agent.currentPos) {
        throw std::runtime_error("agent position error!");
    }
    if (vector[0].leaveTime + 1 > agent.lastTimeStamp) {
        map->addNodeOccupied(vector[0].pos, agent.lastTimeStamp, vector[0].leaveTime + 1);
    }
    for (size_t j = 1; j < vector.size(); j++) {
        size_t endTime = vector[j].leaveTime + 1;
        if (j == vector.size() - 1) endTime = std::numeric_limits<size_t>::max() / 2;
        map->addNodeOccupied(vector[j].pos, vector[j - 1].leaveTime + 1, endTime);
        auto dir = map->getDirectionByPos(vector[j - 1].pos, vector[j].pos);
        if (dir == Map::Direction::NONE) {
            continue;
//            throw std::runtime_error("");
        }
        map->addEdgeOccupied(vector[j - 1].pos, dir, vector[j - 1].leaveTime,
                             vector[j - 1].leaveTime + 1);
    }
    agent.path.insert(agent.path.end(), vector.begin(), vector.end());
    agent.currentPos = vector.back().pos;
    agent.lastTimeStamp = vector.back().leaveTime;
//    map->printOccupiedMap();
}

void Manager::selectTask(Map *map) {
    int j = 0;
    std::vector<int> maxFlexAgent(tasks.size(), -1);
    double minFlex = std::numeric_limits<double>::max();
    int selectedTask = -1;

    for (auto &task :tasks) {
        double maxFlex = -1;
        for (int i = 0; i < agents.size(); i++) {
            auto &flex = agents[i].flexibility[j];
            if (flex.first > maxFlex) {
                maxFlex = flex.first;
                maxFlexAgent[j] = i;
            }
        }
        if (maxFlex >= 0 && maxFlex < minFlex) {
            minFlex = maxFlex;
            selectedTask = j;
        }
        j++;
    }

    if (selectedTask >= 0) {
        int selectedAgent = maxFlexAgent[selectedTask];
        std::cout << "agent: " << selectedAgent << ", task: "
                  << selectedTask << "/" << tasks.size() << ", flex: "
                  << agents[selectedAgent].flexibility[selectedTask].first << std::endl;
        assignTask(map, agents[selectedAgent], agents[selectedAgent].flexibility[selectedTask].second);
    }

    j = 0;
    for (auto it = tasks.begin(); it != tasks.end(); j++) {
        if (maxFlexAgent[j] < 0 || j == selectedTask) {
            if (maxFlexAgent[j] < 0) {
                std::cout << "fail task " << it->get()->getBucket() << std::endl;
            } else {
                std::cout << "complete task " << it->get()->getBucket() << std::endl;
            }
            it = tasks.erase(it);
        } else {
            ++it;
        }
    }
}

size_t Manager::computePath(Solver &solver, std::vector<PathNode> &path, Scenario *task, size_t startTime) {
    solver.initScenario(task, startTime);
    size_t count = 0;
    while (!solver.success() && solver.step() && count < 10000) {
        ++count;
    }
    if (!solver.success()) {
        return 0;
    }
    auto vNodePath = solver.constructPath();
    for (auto it = vNodePath.rbegin(); it != vNodePath.rend(); ++it) {
        auto vNode = *it;
//        std::cout << vNode->pos.first << " " << vNode->pos.second << " " << vNode->leaveTime << std::endl;
        path.emplace_back(PathNode{vNode->pos, vNode->leaveTime});
    }
//    std::cout << vNodePath.size() << std::endl;
    return path.back().leaveTime;
}

void Manager::computeFlex(Solver &solver, int x, double phi) {
    auto map = solver.getMap();
    for (size_t i = 0; i < agents.size(); i++) {
        agents[i].flexibility.clear();

        auto agentLeaveTime = agents[i].lastTimeStamp;

        // clear node constraint for parking location of the current agent
        map->removeNodeOccupied(agents[i].currentPos, agentLeaveTime);

        size_t j = 0;
        for (auto &task: tasks) {
            std::vector<PathNode> path;

            // agent go to task start position
            auto task1 = Scenario(i, map, agents[i].currentPos, task->getStart(), 0);
            auto agentStartTime = computePath(solver, path, &task1, agentLeaveTime);
            if (agentStartTime == 0) {
                agents[i].flexibility.emplace_back(-1, path);
            } else {
                auto agentEndTime = computePath(solver, path, task.get(), agentStartTime);
                if (agentEndTime == 0) {
                    agents[i].flexibility.emplace_back(-1, path);
                } else {
                    size_t pathLength = agentEndTime - agentLeaveTime;
                    double beta = (1 + phi) * task->getOptimal();
                    if (x == 1) {
                        beta -= (double) (agentLeaveTime + pathLength);
                    }
                    agents[i].flexibility.emplace_back(beta, path);
                }
            }
//            std::cout << i << " " << j << " " << agents[i].flexibility.back().first << std::endl;

//            exit(0);
            j++;
        }

        // add back node constraint for parking location of the current agent
        map->addNodeOccupied(agents[i].currentPos, agentLeaveTime, std::numeric_limits<size_t>::max() / 2);
    }
}

