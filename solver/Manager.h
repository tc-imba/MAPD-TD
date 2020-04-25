//
// Created by liu on 2020/3/7.
//

#ifndef MAPF_MANAGER_H
#define MAPF_MANAGER_H

#include "Map.h"
#include "Scenario.h"
#include "Solver.h"

#include <unordered_map>
#include <list>
#include <memory>

class Manager {
public:
    struct PathNode {
        std::pair<size_t, size_t> pos;
        size_t leaveTime;
    };

    struct Agent {
        std::pair<size_t, size_t> originPos, currentPos;
        std::vector<std::unique_ptr<Scenario> > tasks;
        size_t lastTimeStamp = 0;
        std::vector<PathNode> path;
        std::vector<std::pair<double, std::vector<PathNode>>> flexibility;

        explicit Agent(std::pair<size_t, size_t> pos) : originPos(pos), currentPos(pos) {}
    };

private:
    std::string dataPath;
    std::unordered_map<std::string, std::unique_ptr<Map> > maps;
    std::vector<std::unique_ptr<Scenario> > scenarios;

    std::vector<Agent> agents;
    std::list<std::unique_ptr<Scenario> > tasks;

    void computeFlex(Solver &solver, int x, double phi=0.25);

    void selectTask(Map *map);

    void assignTask(Map *map, Agent &agent, std::vector<PathNode> &vector);

    Map *loadMapFile(const std::string &filename);

    size_t computePath(Solver &solver, std::vector<PathNode> &path, Scenario *task, size_t startTime);

public:
    explicit Manager(std::string dataPath);

    Map *getMap(const std::string &mapName);

    void loadScenarioFile(const std::string &filename);

    Scenario *getScenario(size_t index = 0);

    Map *loadTaskFile(const std::string &filename);

    void leastFlexFirstAssign(Map *map);
};


#endif //MAPF_MANAGER_H
