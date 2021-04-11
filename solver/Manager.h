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
        bool released = true;

        explicit Task(Scenario &&scenario) : scenario(scenario) {}
    };

    struct Flexibility {
        double beta;
        std::vector<PathNode> path;
        Task *task;
        size_t occupiedAgent;
    };

    struct Agent {
        std::pair<size_t, size_t> originPos, currentPos, reservePos;
        std::vector<std::unique_ptr<Task> > tasks;
        size_t lastTimeStamp = 0;
        std::vector<PathNode> path;
        std::vector<PathNode> reservedPath;
        std::vector<Flexibility> flexibility;

        explicit Agent(std::pair<size_t, size_t> pos) : originPos(pos), currentPos(pos), reservePos(pos) {}

        Agent(const Agent &that) :
                originPos(that.originPos), currentPos(that.currentPos), reservePos(that.reservePos),
                lastTimeStamp(that.lastTimeStamp), reservedPath(that.reservedPath) {}
    };

    struct Count {
        size_t step = 0;
        size_t skip = 0;
        size_t calculate = 0;
    };

private:
    std::string dataPath;
    std::unordered_map<std::string, std::unique_ptr<Map> > maps;
    std::vector<std::unique_ptr<Scenario> > scenarios;

    std::vector<Agent> agents;
    std::vector<std::unique_ptr<Task> > tasks;
    size_t agentMaxReserveTimestamp = 0;
    size_t agentMaxTimestamp = 0;
    size_t agentMaxTimestampAgent = 0;

//    std::list<std::unique_ptr<Scenario> > tasks;
//    std::vector<size_t> tasksMaxBetaAgent;

    size_t maxStep, windowSize;
    int extraCostId;
    bool boundFlag;
    bool sortFlag;
    bool multiLabelFlag;
    bool occupiedFlag;
    bool deadlineBoundFlag;
    bool taskBoundFlag;
    bool recalculateFlag;
    bool reserveAllFlag;
    bool skipFlag;
    bool reserveNearestFlag;

    void applyReservedPath();

    void computeFlex(Solver &solver, int x, double phi);

    void selectTask(Solver &solver, int x, double phi);

    bool assignTask(Solver &solver, size_t i, std::vector<PathNode> &vector, size_t occupiedAgent);

    std::vector<Constraint> generateConstraints(Map *map, Agent &agent, const std::vector<PathNode> &vector);

    void addAgentPathConstraints(Map *map, Agent &agent, const std::vector<PathNode> &vector);

    void removeAgentPathConstraints(Map *map, Agent &agent, const std::vector<PathNode> &vector);

    Map *loadMapFile(const std::string &filename);
    
    bool reservePath(Solver &solver, size_t i);

    std::pair<size_t, size_t> computePath(Solver &solver, std::vector<PathNode> &path, Scenario *task,
                                          size_t startTime, size_t deadline);

    size_t computeAgentForTask(Solver &solver, size_t j, const std::vector<std::pair<size_t, double> > &sortAgent,
                               double phi, double &minBeta, size_t &minBetaTask, Count &count, bool recalculate = false);

    bool isPathConflict(Solver &solver, Agent &agent, const std::vector<PathNode> &vector);

public:
    explicit Manager(std::string dataPath, size_t maxStep = 10000, size_t windowSize = 0, int extraCostId = 0,
                     bool boundFlag = true, bool sortFlag = true,
                     bool multiLabelFlag = true, bool occupiedFlag = true,
                     bool deadlineBoundFlag = true, bool taskBoundFlag = true,
                     bool recalculateFlag = true, bool reserveAllFlag = true,
                     bool skipFlag = false, bool reserveNearestFlag = false);

    Map *getMap(const std::string &mapName);

    void loadScenarioFile(const std::string &filename);

    Scenario *getScenario(size_t index = 0);

    Map *loadTaskFile(const std::string &filename);

    void leastFlexFirstAssign(Map *map, int algorithm, double phi);

    void earliestDeadlineFirstAssign(Map *map, int algorithm, double phi);

    void printPaths();
};


#endif //MAPF_MANAGER_H
