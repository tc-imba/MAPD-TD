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

    struct Constraint {
        std::pair<size_t, size_t> pos;
        Map::Direction direction;
        size_t start, end;
    };

    struct Task {
        Scenario scenario;
        double maxBeta = -1;
        size_t maxBetaAgent = std::numeric_limits<size_t>::max();

        explicit Task(Scenario &&scenario) : scenario(scenario) {}
    };

    struct Flexibility {
        double beta;
        std::vector<PathNode> path;
        Task *task;
        size_t occupiedAgent;
    };

    struct Agent {
        std::pair<size_t, size_t> originPos, currentPos;
        std::vector<std::unique_ptr<Task> > tasks;
        size_t lastTimeStamp = 0;
        std::vector<PathNode> path;
        std::vector<Flexibility> flexibility;

        explicit Agent(std::pair<size_t, size_t> pos) : originPos(pos), currentPos(pos) {}
    };

private:
    std::string dataPath;
    std::unordered_map<std::string, std::unique_ptr<Map> > maps;
    std::vector<std::unique_ptr<Scenario> > scenarios;

    std::vector<Agent> agents;
    std::vector<std::unique_ptr<Task> > tasks;

//    std::list<std::unique_ptr<Scenario> > tasks;
//    std::vector<size_t> tasksMaxBetaAgent;

    size_t maxStep;
    bool boundFlag;
    bool sortFlag;
    bool occupiedFlag;

    void computeFlex(Solver &solver, int x, double phi);

    void selectTask(Map *map);

    void assignTask(Map *map, Agent &agent, std::vector<PathNode> &vector);

    std::vector<Constraint> generateConstraints(Map *map, Agent &agent, const std::vector<PathNode> &vector);

    Map *loadMapFile(const std::string &filename);

    std::pair<size_t, size_t> computePath(Solver &solver, std::vector<PathNode> &path, Scenario *task,
                                          size_t startTime, size_t deadline);

    bool isPathConflict(Solver &solver, Agent &agent, const std::vector<PathNode> &vector);

public:
    explicit Manager(std::string dataPath, size_t maxStep = 10000,
                     bool boundFlag = true, bool sortFlag = true, bool occupiedFlag = true);

    Map *getMap(const std::string &mapName);

    void loadScenarioFile(const std::string &filename);

    Scenario *getScenario(size_t index = 0);

    Map *loadTaskFile(const std::string &filename);

    void leastFlexFirstAssign(Map *map, int algorithm, double phi);
};


#endif //MAPF_MANAGER_H
