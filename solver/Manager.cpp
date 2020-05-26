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

void Manager::leastFlexFirstAssign(Map *map, double phi) {
    Solver solver(map);

    // add node constraints for parking locations
    for (auto &agent: agents) {
        map->addNodeOccupied(agent.originPos, agent.lastTimeStamp, std::numeric_limits<size_t>::max() / 2);
    }

    while (!tasks.empty()) {
        computeFlex(solver, 1, phi);
        selectTask(map);
//        exit(0);
//        break;
    }

}


void Manager::assignTask(Map *map, Agent &agent, std::vector<PathNode> &vector) {
    if (vector[0].pos != agent.currentPos) {
        throw std::runtime_error("agent position error!");
    }
    auto constraints = generateConstraints(map, agent, vector);
    for (auto &constraint: constraints) {
        map->addEdgeOccupied(constraint.pos, constraint.direction, constraint.start, constraint.end);
    }
//    for (auto &node:vector) {
//        std::cout << node.pos.first << " " << node.pos.second << " " << node.leaveTime << std::endl;
//    }
    agent.path.insert(agent.path.end(), vector.begin(), vector.end());
    agent.currentPos = vector.back().pos;
    agent.lastTimeStamp = vector.back().leaveTime;
//    map->printOccupiedMap();
}

std::vector<Manager::Constraint>
Manager::generateConstraints(Map *map, Agent &agent, const std::vector<PathNode> &vector) {
    std::vector<Constraint> result;
    if (vector.empty()) return result;
    if (vector[0].leaveTime + 1 > agent.lastTimeStamp) {
        result.emplace_back(
                Constraint{vector[0].pos, Map::Direction::NONE, agent.lastTimeStamp, vector[0].leaveTime + 1});
//        map->addNodeOccupied(vector[0].pos, agent.lastTimeStamp, vector[0].leaveTime + 1);
    }
    for (size_t j = 1; j < vector.size(); j++) {
        size_t endTime = vector[j].leaveTime + 1;
        if (j == vector.size() - 1) endTime = std::numeric_limits<size_t>::max() / 2;
        result.emplace_back(Constraint{vector[j].pos, Map::Direction::NONE, vector[j - 1].leaveTime + 1, endTime});
//        map->addNodeOccupied(vector[j].pos, vector[j - 1].leaveTime + 1, endTime);
        auto dir = map->getDirectionByPos(vector[j - 1].pos, vector[j].pos);
        if (dir == Map::Direction::NONE) {
            continue;
//            throw std::runtime_error("");
        }
        result.emplace_back(Constraint{vector[j - 1].pos, dir, vector[j - 1].leaveTime,
                                       vector[j - 1].leaveTime + 1});
//        map->addEdgeOccupied(vector[j - 1].pos, dir, vector[j - 1].leaveTime,
//                             vector[j - 1].leaveTime + 1);
    }
    return result;
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
            if (flex.beta > maxFlex) {
                maxFlex = flex.beta;
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
                  << agents[selectedAgent].flexibility[selectedTask].beta << std::endl;
        assignTask(map, agents[selectedAgent], agents[selectedAgent].flexibility[selectedTask].path);
        agents[selectedAgent].flexibility.clear();
    }

    j = 0;
    for (auto it = tasks.begin(); it != tasks.end(); j++) {
        if (maxFlexAgent[j] < 0 || j == selectedTask) {
            if (maxFlexAgent[j] < 0) {
                std::cout << "fail task " << it->get()->getBucket() << std::endl;
            } else {
                std::cout << "complete task " << it->get()->getBucket() << std::endl;
                agents[maxFlexAgent[selectedTask]].tasks.emplace_back(std::move(*it));
            }
            it = tasks.erase(it);
        } else {
            ++it;
        }
    }
}

std::pair<size_t, size_t> Manager::computePath(Solver &solver, std::vector<PathNode> &path, Scenario *task, size_t startTime) {
    solver.initScenario(task, startTime);
    size_t count = 0;
    while (!solver.success() && solver.step() && count < 100000) {
        ++count;
    }
    if (!solver.success()) {
        return std::make_pair(0, count);
    }
    auto vNodePath = solver.constructPath();
    for (auto it = vNodePath.rbegin(); it != vNodePath.rend(); ++it) {
        auto vNode = *it;
//        std::cout << vNode->pos.first << " " << vNode->pos.second << " " << vNode->leaveTime << std::endl;
        path.emplace_back(PathNode{vNode->pos, vNode->leaveTime});
    }
//    std::cout << vNodePath.size() << std::endl;
    return std::make_pair(path.back().leaveTime, count);
}

bool Manager::isPathConflict(Solver &solver, Agent &agent, const std::vector<PathNode> &vector) {
    // if no path we need recalculate?
    if (vector.empty()) return true;
    auto map = solver.getMap();
    auto constraints = generateConstraints(map, agent, vector);
    for (auto &constraint:constraints) {
        if (solver.isOccupied(constraint.pos, constraint.direction, constraint.start, constraint.end)) {
            return true;
        }
    }
    return false;
}

void Manager::computeFlex(Solver &solver, int x, double phi) {
    auto map = solver.getMap();
    size_t calculateCount = 0, skipCount = 0;
    size_t stepCount = 0;
    for (size_t i = 0; i < agents.size(); i++) {

        // remember previous flexibility
        std::vector<Flexibility> prevFlexibility;
        prevFlexibility.swap(agents[i].flexibility);
        size_t prevIndex = 0;

        auto agentLeaveTime = agents[i].lastTimeStamp;

        // clear node constraint for parking location of the current agent
        map->removeNodeOccupied(agents[i].currentPos, agentLeaveTime);

        size_t j = 0;
        for (auto &task: tasks) {
            // skip removed tasks
            while (prevIndex < prevFlexibility.size() && prevFlexibility[prevIndex].task != task.get()) {
                ++prevIndex;
            }
            // skip not conflict path
            if (prevIndex < prevFlexibility.size()) {
                auto &path = prevFlexibility[prevIndex].path;
                if (!isPathConflict(solver, agents[i], prevFlexibility[prevIndex].path)) {
                    agents[i].flexibility.emplace_back(prevFlexibility[prevIndex]);
                    j++;
                    skipCount++;
                    continue;
                }
            }
            std::vector<PathNode> path;

            // agent go to task start position
            auto task1 = Scenario(i, map, agents[i].currentPos, task->getStart(), 0);
            auto path1 = computePath(solver, path, &task1, agentLeaveTime);
            auto agentStartTime = path1.first;
            stepCount += path1.second;
            if (agentStartTime == 0) {
                agents[i].flexibility.emplace_back(Flexibility{-1, path, task.get()});
            } else {
                auto path2 = computePath(solver, path, task.get(), agentStartTime);
                auto agentEndTime = path2.first;
                stepCount += path2.second;
                if (agentEndTime == 0) {
                    agents[i].flexibility.emplace_back(Flexibility{-1, path, task.get()});
                } else {
                    size_t pathLength = agentEndTime - agentLeaveTime;
                    double beta = (1 + phi) * task->getOptimal();
                    if (x == 1) {
                        beta -= (double) (agentLeaveTime + pathLength);
                    }
                    agents[i].flexibility.emplace_back(Flexibility{beta, path, task.get()});
                }
            }
//            std::cout << i << " " << j << " " << agents[i].flexibility.back().first << std::endl;
//            exit(0);
            j++;
            calculateCount++;
        }

        // add back node constraint for parking location of the current agent
        map->addNodeOccupied(agents[i].currentPos, agentLeaveTime, std::numeric_limits<size_t>::max() / 2);
    }
    std::cout << "calculate: " << calculateCount << ", skip: " << skipCount << ", step: " << stepCount << std::endl;
}

