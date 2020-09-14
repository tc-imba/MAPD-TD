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
                 bool boundFlag, bool sortFlag, bool multiLabelFlag, bool occupiedFlag, bool deadlineBoundFlag,
                 bool recalculateFlag)
        : dataPath(std::move(dataPath)), maxStep(maxStep),
          boundFlag(boundFlag), sortFlag(sortFlag),
          multiLabelFlag(multiLabelFlag), occupiedFlag(occupiedFlag),
          deadlineBoundFlag(deadlineBoundFlag), recalculateFlag(recalculateFlag) {}

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
        auto scenario = std::make_unique<Scenario>(bucket, map, start, end, optimal, 0);
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
            size_t dist, startTime;
            fin >> start.first >> start.second >> end.first >> end.second >> dist >> startTime;
            auto task = std::make_unique<Task>(Scenario(i * k + j, map, start, end, dist, startTime));
            tasks.emplace_back(std::move(task));
        }
    }
    return map;
}

void Manager::leastFlexFirstAssign(Map *map, int algorithm, double phi) {
    Solver solver(map, algorithm);

    // add node constraints for parking locations
    for (size_t i = 0; i < agents.size(); i++) {
        auto &agent = agents[i];
        map->addInfiniteWaiting(agent.originPos);
        map->addWaitingAgent(agent.originPos, agent.lastTimeStamp, i);
    }

    auto start = std::chrono::system_clock::now();

    while (!tasks.empty()) {
        computeFlex(solver, 1, phi);
//        for (int i = 0; i < agents.size(); i++) {
//            for (int j = 0; j < tasks.size(); j++) {
//                std::cout << i << " " << j << " " << agents[i].flexibility[j].beta << std::endl;
//            }
//        }
        selectTask(solver, 1, phi);
    }

    applyReservedPath();

    auto end = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "time: " << time << "ms" << std::endl;

}

void Manager::earliestDeadlineFirstAssign(Map *map, int algorithm, double phi) {
    Solver solver(map, algorithm);
    std::vector<std::pair<size_t, double> > sortAgent(agents.size());

    // add node constraints for parking locations
    for (size_t i = 0; i < agents.size(); i++) {
        auto &agent = agents[i];
        map->addInfiniteWaiting(agent.originPos);
        map->addWaitingAgent(agent.originPos, agent.lastTimeStamp, i);
        agent.flexibility.resize(tasks.size());
        sortAgent[i] = std::make_pair(i, -1);
    }

    auto start = std::chrono::system_clock::now();


    std::sort(tasks.begin(), tasks.end(),
              [](const auto &a, const auto &b) { return a->scenario.getOptimal() < b->scenario.getOptimal(); });

    for (size_t j = 0; j < tasks.size(); j++) {
        double minBeta = -1;
        size_t minBetaTask = std::numeric_limits<size_t>::max();
        Count count;
        auto selectedAgent = computeAgentForTask(solver, j, sortAgent, phi, minBeta, minBetaTask, count, recalculateFlag);
        std::cout << "calculate: " << count.calculate << ", skip: " << count.skip << ", step: " << count.step
                  << std::endl;

        bool successTask = selectedAgent < agents.size();
        if (successTask) {
            auto &flex = agents[selectedAgent].flexibility[j];
            std::cout << "agent: " << selectedAgent << ", task: "
                      << tasks[j]->scenario.getBucket() << ", flex: "
                      << flex.beta << std::endl;
            successTask = assignTask(solver, selectedAgent, flex.path, flex.occupiedAgent);
        }
        if (successTask) {
            std::cout << "complete task " << tasks[j]->scenario.getBucket() << std::endl;
        } else {
            std::cout << "fail task " << tasks[j]->scenario.getBucket() << std::endl;
        }
    }

    applyReservedPath();

    auto end = std::chrono::system_clock::now();
    auto time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "time: " << time << "ms" << std::endl;
}

void Manager::applyReservedPath() {
    for (size_t i = 0; i < agents.size(); i++) {
        auto &agent = agents[i];
        if (!agent.reservedPath.empty()) {
            std::cout << "apply: " << i << std::endl;
            agent.path.insert(agent.path.end(), agent.reservedPath.begin(), agent.reservedPath.end());
        }
    }
}

size_t Manager::computeAgentForTask(Solver &solver, size_t j, const std::vector<std::pair<size_t, double> > &sortAgent,
                                    double phi, double &minBeta, size_t &minBetaTask, Count &count, bool recalculate) {
    auto map = solver.getMap();
    auto &task = tasks[j];

    // set branch and bound
    double deadline = (1 + phi) * task->scenario.getOptimal();
    size_t upperBound;
    size_t infinite = std::numeric_limits<size_t>::max() / 2;
    if (boundFlag) {
        upperBound = deadline + 1;
    } else {
        upperBound = infinite;
    }

    bool skipFlag = false;
    double taskMaxBeta = -1;
    size_t taskMinAgentTime = std::numeric_limits<size_t>::max();
    size_t taskSelectedAgent = std::numeric_limits<size_t>::max();


    if (recalculate) {
        for (auto &p : sortAgent) {
            auto i = p.first;
            auto &agent = agents[i];
            if (agent.flexibility[j].beta >= 0 && !agent.flexibility[j].path.empty()) {
                // recalculate based on time spent by agent, not flexibility
                size_t agentTime = agent.flexibility[j].path.back().leaveTime + 1 - agent.lastTimeStamp;
                // break tie with smaller agent id
                if (agentTime < taskMinAgentTime || (agentTime == taskMinAgentTime && i < taskSelectedAgent)) {
                    taskSelectedAgent = i;
                    taskMinAgentTime = agentTime;
                }
            }
        }
    }

    task->released = true;

    for (auto &p : sortAgent) {
        auto i = p.first;
        auto &agent = agents[i];
        auto agentLeaveTime = agent.lastTimeStamp;
        std::vector<PathNode> path;
//        const auto deliveryOccupiedAgent = occupiedAgent.second;

        if (recalculate) {
            // skip already calculated agent
            if (agent.flexibility[j].beta >= 0 && !agent.flexibility[j].path.empty()) {
                continue;
            }
            // recalculate based on time spent by agent, not flexibility
            if (taskMinAgentTime < std::numeric_limits<size_t>::max() / 2) {
                upperBound = agent.lastTimeStamp + taskMinAgentTime;
            }
            upperBound = std::min(upperBound, (size_t) deadline + 1);
        }

        if (skipFlag) {
            auto beta = p.second;
            if (beta < 0) beta = -1;
            else if (beta < minBeta) beta = minBeta;
            agent.flexibility[j] = Flexibility{beta, path, task.get(), 0};
            count.skip++;
            continue;
        }

        // if an agent is at start pos, skip other agents
        /*if (occupiedFlag && occupiedAgent[j].first < agents.size() && occupiedAgent[j].first != i) {
            agent.flexibility.emplace_back(Flexibility{-1, path, task.get(), deliveryOccupiedAgent});
            skipCount++;
            continue;
        }*/

        // clear node constraint for parking location of the current agent
//        map->removeNodeOccupied(agent.currentPos, agentLeaveTime);
        if (agent.reservedPath.empty()) {
            map->removeNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
        }
        const auto infiniteWaiting = map->removeInfiniteWaiting(agent.originPos);
//        std::cout << infiniteWaiting << std::endl;
        if (!agent.reservedPath.empty()) {
//            for (auto item : agent.reservedPath) {
//                std::cerr << item.pos.first << " " << item.pos.second << " " << item.leaveTime << std::endl;
//            }
            removeAgentPathConstraints(map, agent, agent.reservedPath);
        }

//        if (infiniteWaiting != infinite) {
//            std::cerr << i << " " << j << std::endl;
//        }
//        map->removeWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);

        const auto deliveryOccupiedAgent = map->getLastWaitingAgent(task->scenario.getEnd());

        // if an agent is at end pos, remove its node constraint
//        if (occupiedFlag && deliveryOccupiedAgent < agents.size()) {
//            map->removeNodeOccupied(agents[deliveryOccupiedAgent].currentPos,
//                                    agents[deliveryOccupiedAgent].lastTimeStamp);
//            map->removeWaitingAgent(agents[deliveryOccupiedAgent].currentPos,
//                                    agents[deliveryOccupiedAgent].lastTimeStamp, deliveryOccupiedAgent);
//        }

//            double deadline = (1 + phi) * task->getOptimal();
//            auto &upperBound = upperBounds[j];


        size_t agentStartTime = 0, agentEndTime = 0;
        size_t distance = Map::getDistance(agent.currentPos, task->scenario.getStart());
        if (deadlineBoundFlag && agentMaxTimestamp + distance < task->scenario.getStartTime()) {
            task->released = false;
        } else if (multiLabelFlag) {
            std::vector<std::pair<size_t, size_t> > positions = {
                    agent.currentPos, task->scenario.getStart(), task->scenario.getEnd()
            };
            auto scenario = Scenario(i, map, positions, 0, 0);
            auto scenarioPath = computePath(solver, path, &scenario, agentLeaveTime, upperBound);
            for (auto &node: path) {
                if (node.pos == task->scenario.getStart()) {
                    agentStartTime = node.leaveTime;
                    break;
                }
            }
            if (agentStartTime > 0 && agentStartTime >= task->scenario.getStartTime()) {
                agentEndTime = scenarioPath.first;
            }
            count.step += scenarioPath.second;
        } else {
            // agent go to task start position
            auto scenario = Scenario(i, map, agent.currentPos, task->scenario.getStart(), 0, 0);
            auto scenarioPath = computePath(solver, path, &scenario, agentLeaveTime, upperBound);

            agentStartTime = scenarioPath.first;
            count.step += scenarioPath.second;
            if (agentStartTime > 0 && agentStartTime >= task->scenario.getStartTime()) {
                scenarioPath = computePath(solver, path, &task->scenario, agentStartTime, upperBound);
                agentEndTime = scenarioPath.first;
                count.step += scenarioPath.second;
            }
        }

        if (agentStartTime > 0 && agentStartTime < task->scenario.getStartTime()) {
            task->released = false;
        }
        if (agentEndTime == 0) {
            agent.flexibility[j] = Flexibility{-1, path, task.get(), deliveryOccupiedAgent};
        } else {
            size_t pathLength = agentEndTime - agentLeaveTime;
            double beta = deadline;
            beta -= (double) (agentLeaveTime + pathLength);
            agent.flexibility[j] = Flexibility{beta, path, task.get(), deliveryOccupiedAgent};
            if (beta >= 0) {
                if (boundFlag) {
                    if (minBeta >= 0 && beta > minBeta) {
                        skipFlag = true;
                    }
                    upperBound = std::min(upperBound, (size_t) (deadline - beta + 1));
                }
                if (!recalculate) {
                    if (beta > taskMaxBeta || (beta == taskMaxBeta && i < taskSelectedAgent)) {
                        taskMaxBeta = beta;
                        taskSelectedAgent = i;
                    }
                } else {
                    size_t agentTime = agent.flexibility[j].path.back().leaveTime + 1 - agent.lastTimeStamp;
                    // break tie with smaller agent id
                    if (agentTime < taskMinAgentTime || (agentTime == taskMinAgentTime && i < taskSelectedAgent)) {
                        taskSelectedAgent = i;
                        taskMinAgentTime = agentTime;
                    }
                }
            }
        }

        // add back node constraint for parking location of the current agent
//        map->addNodeOccupied(agent.currentPos, agentLeaveTime, std::numeric_limits<size_t>::max() / 2);
        if (!agent.reservedPath.empty()) {
            addAgentPathConstraints(map, agent, agent.reservedPath);
        }
        map->addInfiniteWaiting(agent.originPos, infiniteWaiting);
        if (agent.reservedPath.empty()) {
            map->addNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
        }


//        map->addWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);

//        if (occupiedFlag && deliveryOccupiedAgent < agents.size()) {
//            map->addNodeOccupied(agents[deliveryOccupiedAgent].currentPos,
//                                 agents[deliveryOccupiedAgent].lastTimeStamp,
//                                 std::numeric_limits<size_t>::max() / 2);
//            map->addWaitingAgent(agents[deliveryOccupiedAgent].currentPos,
//                                 agents[deliveryOccupiedAgent].lastTimeStamp, deliveryOccupiedAgent);
//        }
        count.calculate++;
        //            if (agent.flexibility.back().beta >= 0) {
//        std::cout << "calculate: " << i << " " << j << " " << agent.flexibility[j].beta << std::endl;
//        for (auto item :agent.flexibility[j].path) {
//            std::cerr << item.pos.first << " " << item.pos.second << " " << item.leaveTime << std::endl;
//        }
//        exit(0);
//            }
    }

    task->maxBeta = taskMaxBeta;
    task->maxBetaAgent = taskSelectedAgent;


    if (!skipFlag && taskMaxBeta >= 0 &&
        (minBeta < 0 || taskMaxBeta < minBeta || (taskMaxBeta == minBeta && j < minBetaTask))) {
        minBeta = taskMaxBeta;
        minBetaTask = j;
//            std::cout << task->getBucket() << " " << minBeta << std::endl;
    }

//    std::cout << "step: " << stepCount << std::endl;
    return taskSelectedAgent;
}

bool Manager::reservePath(Solver &solver, size_t i) {
    auto map = solver.getMap();
    auto &agent = agents[i];
    Scenario task(0, map, agent.currentPos, agent.originPos, 0, 0);
    std::vector<PathNode> path;

    map->removeNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
    auto infiniteWaiting = map->removeInfiniteWaiting(agent.originPos);
    auto result = computePath(solver, path, &task, agent.lastTimeStamp, std::numeric_limits<size_t>::max() / 2);
    map->addNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
    map->addInfiniteWaiting(agent.originPos, infiniteWaiting);

    if (result.first == 0) {
        return false;
    }

    map->removeNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
    map->removeInfiniteWaiting(agent.originPos);
    addAgentPathConstraints(map, agent, path);
    map->addInfiniteWaiting(agent.originPos);

//    agentMaxTimestamp = std::max(agentMaxTimestamp, path.back().leaveTime);


//    agent.waitingPos = path.back().pos;
//    agent.waitingTimeStamp = path.back().leaveTime;

    agent.reservedPath.swap(path);
//    map->addWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);
    std::cout << "reserve: " << i << std::endl;
    return true;
}

std::pair<size_t, size_t> Manager::computeReservedPath(Solver &solver, size_t i, std::vector<PathNode> &path) {
    auto map = solver.getMap();
    auto &agent = agents[i];
    Scenario task(0, map, agent.currentPos, agent.originPos, 0, 0);

    map->removeNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
    auto infiniteWaiting = map->removeInfiniteWaiting(agent.originPos);
    auto result = computePath(solver, path, &task, agent.lastTimeStamp, std::numeric_limits<size_t>::max() / 2);
    map->addNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
    map->addInfiniteWaiting(agent.originPos, infiniteWaiting);
    return result;
}

bool Manager::assignTask(Solver &solver, size_t i, std::vector<PathNode> &vector, size_t occupiedAgent) {
    auto map = solver.getMap();
    auto &agent = agents[i];
    bool result = true;
    if (vector[0].pos != agent.currentPos) {
        throw std::runtime_error("agent position error!");
    }

    // remove node constraint for agent
//    map->removeNodeOccupied(agent.currentPos, agent.lastTimeStamp);
    map->removeWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);

    if (agent.reservedPath.empty()) {
        map->removeNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
    }
    map->removeInfiniteWaiting(agent.originPos);
    if (!agent.reservedPath.empty()) {
        removeAgentPathConstraints(map, agent, agent.reservedPath);
        std::cout << "clear: " << i << std::endl;
    }
    addAgentPathConstraints(map, agent, vector);

    map->addInfiniteWaiting(agent.originPos);

    std::set<size_t> reservingAgentSet;
    if (occupiedFlag) {
        for (auto &node : vector) {
            auto j = map->getLastWaitingAgent(node.pos);
            if (j >= agents.size()) continue;
            if (!agents[j].reservedPath.empty() || agents[j].lastTimeStamp > node.leaveTime) continue;
            if (agents[j].originPos == node.pos) continue;
            reservingAgentSet.emplace(j);
        }
        if (occupiedAgent < agents.size() && occupiedAgent != i) {
            size_t reservingAgent;
            if (agent.lastTimeStamp < agents[occupiedAgent].lastTimeStamp) {
                reservingAgent = i;
            } else {
                reservingAgent = occupiedAgent;
            }
            reservingAgentSet.emplace(reservingAgent);
        }
    }
    if (reservingAgentSet.empty()) {
        // no overlapping
        agent.currentPos = vector.back().pos;
        agent.lastTimeStamp = vector.back().leaveTime;
        agent.reservedPath.clear();
        map->addWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);
        agent.path.insert(agent.path.end(), vector.begin(), vector.end());
    } else {
        auto tempPos = agent.currentPos;
        auto tempTimeStamp = agent.lastTimeStamp;
        std::vector<PathNode> tempReservedPath;
        tempReservedPath.swap(agent.reservedPath);

        agent.currentPos = vector.back().pos;
        agent.lastTimeStamp = vector.back().leaveTime;

        std::vector<std::pair<size_t, Agent> > savedAgents;
        savedAgents.reserve(reservingAgentSet.size());
        size_t successAgents = 0;
        for (auto j : reservingAgentSet) {
            savedAgents.emplace_back(j, agents[j]);
            if (reservePath(solver, j)) {
                successAgents++;
            } else break;
        }
        if (successAgents == reservingAgentSet.size()) {
            // reserve agent successfully
            for (auto &p : savedAgents) {
                agentMaxTimestamp = std::max(agentMaxTimestamp, agents[p.first].reservedPath.back().leaveTime);
            }
            agent.path.insert(agent.path.end(), vector.begin(), vector.end());
            map->addWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);
//            std::cerr << "success: " << i << " reserved: ";
            for (auto &p : savedAgents) {
                agentMaxTimestamp = std::max(agentMaxTimestamp, agents[p.first].reservedPath.back().leaveTime);
//                std::cerr << p.first << " ";
            }
//            std::cerr << std::endl;
        } else {
//            exit(-1);
            // revert the agent position for failed task
            for (auto k = successAgents; k != 0; k--) {
                auto j = savedAgents[k].first;
                auto &restoreAgent = agents[j];
                auto &savedAgent = savedAgents[k].second;

                map->removeInfiniteWaiting(savedAgent.originPos);
                addAgentPathConstraints(map, savedAgent, restoreAgent.reservedPath);
                map->addInfiniteWaiting(savedAgent.originPos);
                map->addNodeOccupied(savedAgent.currentPos, savedAgent.lastTimeStamp, savedAgent.lastTimeStamp + 1);

                restoreAgent.currentPos = savedAgent.currentPos;
                restoreAgent.lastTimeStamp = savedAgent.lastTimeStamp;
                restoreAgent.reservedPath.swap(savedAgent.reservedPath);
            }

            agent.currentPos = tempPos;
            agent.lastTimeStamp = tempTimeStamp;
            agent.reservedPath.swap(tempReservedPath);

            map->removeInfiniteWaiting(agent.originPos);
            removeAgentPathConstraints(map, agent, vector);
            if (!agent.reservedPath.empty()) {
                addAgentPathConstraints(map, agent, agent.reservedPath);
            }
            map->addInfiniteWaiting(agent.originPos);
            if (agent.reservedPath.empty()) {
                map->addNodeOccupied(agent.currentPos, agent.lastTimeStamp, agent.lastTimeStamp + 1);
            }
            map->addWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);
            std::cout << "fail: " << i << " " << occupiedAgent << std::endl;
            result = false;
        }
    }
    agentMaxTimestamp = std::max(agentMaxTimestamp, agent.lastTimeStamp);
    return result;
/*
    if (occupiedFlag && occupiedAgent < agents.size() && occupiedAgent != i) {
        auto tempPos = agent.currentPos;
        auto tempTimeStamp = agent.lastTimeStamp;
        std::vector<PathNode> tempReservedPath;
        tempReservedPath.swap(agent.reservedPath);

        agent.currentPos = vector.back().pos;
        agent.lastTimeStamp = vector.back().leaveTime;

        size_t reservingAgent;
//        size_t reservingAgent, stayingAgent;
        if (agent.lastTimeStamp < agents[occupiedAgent].lastTimeStamp) {
            reservingAgent = i;
//            stayingAgent = occupiedAgent;
        } else {
            reservingAgent = occupiedAgent;
//            stayingAgent = i;
        }

        if (reservePath(solver, reservingAgent)) {
//            agents[stayingAgent].waitingPos = agents[stayingAgent].currentPos;
//            agents[stayingAgent].waitingTimeStamp = agents[stayingAgent].lastTimeStamp;
//            map->addWaitingAgent(agents[stayingAgent].currentPos, agents[stayingAgent].lastTimeStamp, stayingAgent);
            agent.path.insert(agent.path.end(), vector.begin(), vector.end());
            map->addWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);
            std::cerr << "success: " << i << " " << occupiedAgent << std::endl;
        } else {
            // revert the agent position for failed task
            agent.currentPos = tempPos;
            agent.lastTimeStamp = tempTimeStamp;
            agent.reservedPath.swap(tempReservedPath);
            map->addWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);
            std::cerr << "fail: " << i << " " << occupiedAgent << std::endl;
        }
    } else {
        // no overlapping
//        agent.currentPos = agent.waitingPos = vector.back().pos;
        agent.currentPos = vector.back().pos;
//        agent.lastTimeStamp = agent.waitingTimeStamp = vector.back().leaveTime;
        agent.lastTimeStamp = vector.back().leaveTime;
        agent.reservedPath.clear();
        map->addWaitingAgent(agent.currentPos, agent.lastTimeStamp, i);
        agent.path.insert(agent.path.end(), vector.begin(), vector.end());
    }
    agentMaxTimestamp = std::max(agentMaxTimestamp, agent.lastTimeStamp);
*/
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
        if (j == vector.size() - 1) {
//            endTime = std::numeric_limits<size_t>::max() / 2;
        }
        result.emplace_back(Constraint{vector[j].pos, Map::Direction::NONE, vector[j - 1].leaveTime + 1, endTime});
//        std::cerr << vector[j].pos.first << " " << vector[j].pos.second << " " << vector[j - 1].leaveTime + 1 << std::endl;
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


void Manager::addAgentPathConstraints(Map *map, Agent &agent, const std::vector<PathNode> &vector) {
    auto constraints = generateConstraints(map, agent, vector);
    for (auto &constraint: constraints) {
        map->addEdgeOccupied(constraint.pos, constraint.direction, constraint.start, constraint.end);
    }
}

void Manager::removeAgentPathConstraints(Map *map, Agent &agent, const std::vector<PathNode> &vector) {
    auto constraints = generateConstraints(map, agent, vector);
    for (auto &constraint: constraints) {
        map->removeEdgeOccupied(constraint.pos, constraint.direction, constraint.start, constraint.end);
    }
}

void Manager::selectTask(Solver &solver, int x, double phi) {
    auto map = solver.getMap();
    double minFlex = std::numeric_limits<double>::max();

    size_t selectedTask = std::numeric_limits<size_t>::max();
    for (size_t j = 0; j < tasks.size(); j++) {
        auto &task = tasks[j];
        if (task->maxBetaAgent < agents.size() &&
            (task->maxBeta < minFlex || (task->maxBeta == minFlex && j < selectedTask))) {
            minFlex = task->maxBeta;
            selectedTask = j;
        }
    }
    std::cerr << " " << selectedTask <<std::endl;

    bool taskSuccess = selectedTask < tasks.size();
    if (taskSuccess) {
        auto &task = tasks[selectedTask];
        auto selectedAgent = task->maxBetaAgent;
        auto taskBeta = agents[selectedAgent].flexibility[selectedTask].beta;

        if (recalculateFlag) {
            std::vector<std::pair<size_t, double> > tempAgents(agents.size());
            for (size_t i = 0; i < agents.size(); i++) {
                double beta = -1;
                if (agents[i].flexibility[selectedTask].beta >= 0) {
                    beta = agents[i].flexibility[selectedTask].beta;
                } else {
                    // here we use the manhattan distance to sort for beta < 0
                    beta -= std::abs((double) agents[i].currentPos.first - task->scenario.getStart().first);
                    beta -= std::abs((double) agents[i].currentPos.second - task->scenario.getStart().second);
                }
                tempAgents[i] = std::make_pair(i, beta);
            }
            if (sortFlag) {
                std::sort(tempAgents.begin(), tempAgents.end(),
                          [](const auto &a, const auto &b) { return a.second > b.second; });
            }

            double minBeta = -1;
            size_t minBetaTask = std::numeric_limits<size_t>::max();
            Count count;
            auto newSelectedAgent = computeAgentForTask(solver, selectedTask, tempAgents, phi, minBeta, minBetaTask,
                                                        count, true);
            if (selectedAgent != newSelectedAgent) {
                std::cerr << "reselect agent: " << selectedAgent << " -> " << newSelectedAgent << std::endl;
            }
            selectedAgent = newSelectedAgent;
        }

        auto &flex = agents[selectedAgent].flexibility[selectedTask];
        std::cout << "agent: " << selectedAgent << ", task: "
                  << tasks[selectedTask]->scenario.getBucket() << ", flex: "
                  << taskBeta << "(" << flex.beta << ")" << std::endl;
//        for (auto &p : flex.path) {
//            std::cout << p.pos.first << " " << p.pos.second << " " << p.leaveTime << std::endl;
//        }
        taskSuccess = assignTask(solver, selectedAgent, flex.path, flex.occupiedAgent);
        agents[selectedAgent].flexibility.clear();
    }

    std::vector<std::unique_ptr<Task> > newTasks;
    for (size_t j = 0; j < tasks.size(); j++) {
        auto &task = tasks[j];
        if ((task->maxBetaAgent >= agents.size() && task->released) || (j == selectedTask && !taskSuccess)) {
            std::cout << "fail task " << task->scenario.getBucket() << std::endl;
        } else if (j == selectedTask) {
            std::cout << "complete task " << task->scenario.getBucket() << std::endl;
//                std::cout <<  << "(" it->get()->getStart().first << "," << it->get()->getStart().second << " -> "
//                          << it->get()->getEnd().first << "," << it->get()->getEnd().second << ")" << std::endl;
//            agents[task->maxBetaAgent].tasks.emplace_back(std::move(task));
        } else {
            newTasks.emplace_back(std::move(task));
        }
    }
    tasks.swap(newTasks);
}


std::pair<size_t, size_t> Manager::computePath(Solver &solver, std::vector<PathNode> &path,
                                               Scenario *task, size_t startTime, size_t deadline) {
    size_t count = 0;
    // skip if not possible to succeed
    if (startTime + task->getDistance() > deadline) {
        return std::make_pair(0, count);
    }
    solver.initScenario(task, startTime, deadline);
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
//    size_t calculateCount = 0, skipCount = 0;
//    size_t stepCount = 0;
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
    size_t minBetaTask = std::numeric_limits<size_t>::max();
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
//    const auto notOccupied = std::make_pair(std::numeric_limits<size_t>::max(), std::numeric_limits<size_t>::max());
//    std::vector<std::pair<size_t, size_t >> occupiedAgent(tasks.size(), notOccupied);

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

    Count count;
    for (auto _p : sortTasks) {
        auto j = _p.first;
        auto &task = tasks[j];
        // sort the agents
        if (sortFlag) {
            std::sort(sortAgents[j].begin(), sortAgents[j].end(),
                      [](const auto &a, const auto &b) { return a.second > b.second; });
        }
        computeAgentForTask(solver, j, sortAgents[j], phi, minBeta, minBetaTask, count);
    }
    std::cerr << minBetaTask;
//    for (size_t i = 0; i < tasks.size(); i++) {
//        std::cout << "task " << i << ": " << tasks[i]->maxBeta << " " << tasks[i]->maxBetaAgent << std::endl;
//    }
//    map->printOccupiedMap();
    std::cout << "calculate: " << count.calculate << ", skip: " << count.skip << ", step: " << count.step << std::endl;

}

