//
// Created by liu on 2020/3/7.
//

#include "Manager.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <algorithm>
#include <chrono>

Manager::Manager(std::string dataPath, size_t maxStep,
                 bool boundFlag, bool sortFlag, bool occupiedFlag)
        : dataPath(std::move(dataPath)), maxStep(maxStep),
          boundFlag(boundFlag), sortFlag(sortFlag), occupiedFlag(occupiedFlag) {}

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
            auto task = std::make_unique<Task>(Scenario(i * k + j, map, start, end, dist));
            tasks.emplace_back(std::move(task));
        }
    }
    return map;
}

void Manager::leastFlexFirstAssign(Map *map, int algorithm, double phi) {
    Solver solver(map, algorithm);

    // add node constraints for parking locations
    for (auto &agent: agents) {
        map->addNodeOccupied(agent.originPos, agent.lastTimeStamp, std::numeric_limits<size_t>::max() / 2);
    }

    auto start = std::chrono::system_clock::now();

    while (!tasks.empty()) {
        computeFlex(solver, 1, phi);
        selectTask(map);
//        exit(0);
//        break;
    }

    auto end = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "time: " << time << "ms" << std::endl;

}


void Manager::assignTask(Map *map, Agent &agent, std::vector<PathNode> &vector) {
    if (vector[0].pos != agent.currentPos) {
        throw std::runtime_error("agent position error!");
    }

    // remove node constraint for agent
    map->removeNodeOccupied(agent.currentPos, agent.lastTimeStamp);

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
//    int j = 0;
//    std::vector<int> maxFlexAgent(tasks.size(), -1);
    double minFlex = std::numeric_limits<double>::max();
//    int selectedTask = -1;
//
//    for (auto &task :tasks) {
//        double maxFlex = -1;
//        for (int i = 0; i < agents.size(); i++) {
//            auto &flex = agents[i].flexibility[j];
//            if (flex.beta > maxFlex) {
//                maxFlex = flex.beta;
//                maxFlexAgent[j] = i;
//            }
//        }
//        if (maxFlex >= 0 && maxFlex < minFlex) {
//            minFlex = maxFlex;
//            selectedTask = j;
//        }
//        j++;
//    }

    size_t selectedTask = std::numeric_limits<size_t>::max();
    for (size_t j = 0; j < tasks.size(); j++) {
        auto &task = tasks[j];
        if (task->maxBetaAgent < agents.size() && task->maxBeta < minFlex) {
            minFlex = task->maxBeta;
            selectedTask = j;
        }
    }

    if (selectedTask >= 0) {
        auto selectedAgent = tasks[selectedTask]->maxBetaAgent;
        auto &flex = agents[selectedAgent].flexibility[selectedTask];
        std::cout << "agent: " << selectedAgent << ", task: "
                  << selectedTask << "/" << tasks.size() << ", flex: "
                  << flex.beta << std::endl;
        if (occupiedFlag && flex.occupiedAgent < agents.size() && flex.occupiedAgent != selectedAgent) {
            // TODO: move occupiedAgent to parking location
            exit(0);
        }
        assignTask(map, agents[selectedAgent], flex.path);
        agents[selectedAgent].flexibility.clear();
    }

    std::vector<std::unique_ptr<Task> > newTasks;
    for (size_t j = 0; j < tasks.size(); j++) {
        auto &task = tasks[j];
        if (task->maxBetaAgent >= agents.size()) {
            std::cout << "fail task " << task->scenario.getBucket() << std::endl;
        } else if (j == selectedTask) {
            std::cout << "complete task " << task->scenario.getBucket() << std::endl;
//                std::cout <<  << "(" it->get()->getStart().first << "," << it->get()->getStart().second << " -> "
//                          << it->get()->getEnd().first << "," << it->get()->getEnd().second << ")" << std::endl;
            agents[task->maxBetaAgent].tasks.emplace_back(std::move(task));
        } else {
            newTasks.emplace_back(std::move(task));
        }
    }
    tasks.swap(newTasks);
}

std::pair<size_t, size_t> Manager::computePath(Solver &solver, std::vector<PathNode> &path,
                                               Scenario *task, size_t startTime, size_t deadline) {
    solver.initScenario(task, startTime, deadline);
    size_t count = 0;
    while (!solver.success() && solver.step() && count < maxStep) {
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
/*    std::vector<size_t> upperBounds;
    for (auto &task: tasks) {
        if (boundFlag) {
            double deadline = (1 + phi) * task->getOptimal();
            upperBounds.emplace_back(deadline + 1);
        } else {
            upperBounds.emplace_back(std::numeric_limits<size_t>::max() / 2);
        }
    }*/

    // the upper bound of the flexibility of a task
    // if a task can be completed with flexibility larger than this, we can skip this task
    double minBeta = -1;
    std::vector<std::pair<size_t, double> > sortTasks(tasks.size());
    for (size_t j = 0; j < tasks.size(); j++) {
        if (tasks[j]->maxBeta < 0) {
            sortTasks[j] = std::make_pair(j, std::numeric_limits<double>::max());
        } else {
            sortTasks[j] = std::make_pair(j, tasks[j]->maxBeta);
        }
    }
    if (sortFlag) {
        std::sort(sortTasks.begin(), sortTasks.end(),
                  [](const auto &a, const auto &b) { return a.second < b.second; });
    }

    // a vector to sort the agents of each task by flexibility
    std::vector<std::vector<std::pair<size_t, double> > >
            sortAgents(tasks.size(),
                       std::vector<std::pair<size_t, double> >(agents.size()));

    // test whether <start, end> of a task is occupied by an agent
    const auto notOccupied = std::make_pair(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max());
    std::vector<std::pair<size_t, size_t >> occupiedAgent(tasks.size(), notOccupied);

    for (size_t i = 0; i < agents.size(); i++) {

        // remember previous flexibility
        std::vector<Flexibility> prevFlexibility;
        prevFlexibility.swap(agents[i].flexibility);
        agents[i].flexibility.resize(tasks.size());
        size_t prevIndex = 0;

        for (size_t j = 0; j < tasks.size(); j++) {
            auto &task = tasks[j];

            // skip removed tasks
            while (prevIndex < prevFlexibility.size() && prevFlexibility[prevIndex].task != task.get()) {
                ++prevIndex;
            }
            // skip not conflict path
/*            if (prevIndex < prevFlexibility.size()) {
                auto &path = prevFlexibility[prevIndex].path;
                if (!isPathConflict(solver, agents[i], prevFlexibility[prevIndex].path)) {
                    auto &prev = prevFlexibility[prevIndex];
                    if (prev.beta >= 0) {
                        upperBound = std::min(upperBound, (size_t) (deadline - prev.beta + 1));
                    }
                    agents[i].flexibility.emplace_back(prev);
//                    std::cout << "skip: " << i << " " << j << " " << agents[i].flexibility.back().beta << std::endl;
                    j++;
                    skipCount++;
//                    recalculateTasks[j] = false;
                    continue;
                }
            }*/

            // test occupied
            if (occupiedFlag) {
                if (agents[i].currentPos == task->scenario.getStart()) {
                    occupiedAgent[j].first = i;
                }
                if (agents[i].currentPos == task->scenario.getEnd()) {
                    occupiedAgent[j].second = i;
                }
            }

            // sort the tasks
            double beta = -1;
            if (prevIndex < prevFlexibility.size()) {
                beta = prevFlexibility[prevIndex].beta;
            } else {
                // here we use the manhattan distance to sort for beta < 0
                beta -= std::abs((double) agents[i].currentPos.first - task->scenario.getStart().first);
                beta -= std::abs((double) agents[i].currentPos.second - task->scenario.getStart().second);
            }
            sortAgents[j][i] = std::make_pair(i, beta);
        }
    }


    for (auto _p : sortTasks) {
        auto j = _p.first;
        auto &task = tasks[j];
        // sort the agents
        if (sortFlag) {
            std::sort(sortAgents[j].begin(), sortAgents[j].end(),
                      [](const auto &a, const auto &b) { return a.second > b.second; });
        }

        // set branch and bound
        double deadline = (1 + phi) * task->scenario.getOptimal();
        size_t upperBound;
        if (boundFlag) {
            upperBound = deadline + 1;
        } else {
            upperBound = std::numeric_limits<size_t>::max() / 2;
        }
        double taskMaxBeta = -1;
        size_t taskMaxAgent = std::numeric_limits<size_t>::max();

        bool skipFlag = false;
        for (auto &p : sortAgents[j]) {
            auto i = p.first;
            auto agentLeaveTime = agents[i].lastTimeStamp;
            std::vector<PathNode> path;
            const auto deliveryOccupiedAgent = occupiedAgent[j].second;

            if (skipFlag) {
                auto beta = p.second;
                if (beta < 0) beta = -1;
                else if (beta < minBeta) beta = minBeta;
                agents[i].flexibility[j] = Flexibility{beta, path, task.get(), deliveryOccupiedAgent};
                skipCount++;
                continue;
            }

            // if an agent is at start pos, skip other agents
            /*if (occupiedFlag && occupiedAgent[j].first < agents.size() && occupiedAgent[j].first != i) {
                agents[i].flexibility.emplace_back(Flexibility{-1, path, task.get(), deliveryOccupiedAgent});
                skipCount++;
                continue;
            }*/

            // clear node constraint for parking location of the current agent
            map->removeNodeOccupied(agents[i].currentPos, agentLeaveTime);

            // if an agent is at end pos, remove its node constraint
            if (occupiedFlag && deliveryOccupiedAgent < agents.size() && deliveryOccupiedAgent != i) {
                map->removeNodeOccupied(agents[deliveryOccupiedAgent].currentPos,
                                        agents[deliveryOccupiedAgent].lastTimeStamp);
            }

//            double deadline = (1 + phi) * task->getOptimal();
//            auto &upperBound = upperBounds[j];



            // agent go to task start position
            auto task1 = Scenario(i, map, agents[i].currentPos, task->scenario.getStart(), 0);
            auto path1 = computePath(solver, path, &task1, agentLeaveTime, upperBound);


            auto agentStartTime = path1.first;
            stepCount += path1.second;
            if (agentStartTime == 0) {
                agents[i].flexibility[j] = Flexibility{-1, path, task.get(), deliveryOccupiedAgent};
            } else {
                auto path2 = computePath(solver, path, &task->scenario, agentStartTime, upperBound);
                auto agentEndTime = path2.first;
                stepCount += path2.second;
                if (agentEndTime == 0) {
                    agents[i].flexibility[j] = Flexibility{-1, path, task.get(), deliveryOccupiedAgent};
                } else {
                    size_t pathLength = agentEndTime - agentLeaveTime;
                    double beta = deadline;
                    if (x == 1) {
                        beta -= (double) (agentLeaveTime + pathLength);
                    }
                    agents[i].flexibility[j] = Flexibility{beta, path, task.get(), deliveryOccupiedAgent};
                    if (beta > 0 && boundFlag) {
                        if (minBeta > 0 && beta >= minBeta) {
                            skipFlag = true;
                        }
                        if (beta > taskMaxBeta) {
                            taskMaxBeta = beta;
                            taskMaxAgent = i;
                        }
                        upperBound = std::min(upperBound, (size_t) (deadline - beta + 1));
                    }
                }
            }

            // add back node constraint for parking location of the current agent
            map->addNodeOccupied(agents[i].currentPos, agentLeaveTime, std::numeric_limits<size_t>::max() / 2);

            if (occupiedFlag && deliveryOccupiedAgent < agents.size() && deliveryOccupiedAgent != i) {
                map->addNodeOccupied(agents[deliveryOccupiedAgent].currentPos,
                                     agents[deliveryOccupiedAgent].lastTimeStamp,
                                     std::numeric_limits<size_t>::max() / 2);
            }

            calculateCount++;
//            if (agents[i].flexibility.back().beta >= 0) {
//                std::cout << "calculate: " << i << " " << task->getBucket() << " " << agents[i].flexibility.back().beta << std::endl;
//            }
        }

        task->maxBeta = taskMaxBeta;
        task->maxBetaAgent = taskMaxAgent;

        if (!skipFlag && taskMaxBeta >= 0 && (minBeta < 0 || taskMaxBeta < minBeta)) {
            minBeta = taskMaxBeta;
//            minBetaTask = j;
//            std::cout << task->getBucket() << " " << minBeta << std::endl;
        }
    }
    std::cout << "calculate: " << calculateCount << ", skip: " << skipCount << ", step: " << stepCount << std::endl;


}

