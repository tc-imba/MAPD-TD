//
// Created by liu on 2020/3/7.
//

#ifndef MAPF_SOLVER_H
#define MAPF_SOLVER_H

#include "Scenario.h"

#include <queue>
#include <set>
#include <array>
#include <map>
#include <unordered_map>
#include <memory>
#include <limits>

#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/interval_set.hpp>


class Solver {
public:

    struct VirtualNode {
        std::pair<size_t, size_t> pos;  // v
        size_t leaveTime;               // h_v
        size_t estimateTime;            // h_v + g(v), estimated by manhattan distance
        VirtualNode *parent;            // v_p
        std::pair<size_t, size_t> child;// v_c
        size_t checkpoint;
        size_t extraCost;
        bool hasChild;
        bool isOpen;
    };

    struct VirtualNodeSameNodeComp {
        bool operator()(const VirtualNode *a, const VirtualNode *b) const {
            if (a == b) return false;
            if (a->leaveTime != b->leaveTime) return a->leaveTime < b->leaveTime;
            return a->parent < b->parent;
        }
    };

    struct Edge {
//        std::pair<size_t, siz e_t> start, end;
        bool available = true;
//        std::map<size_t, size_t> *occupied = nullptr;
        boost::icl::interval_set<size_t> *occupied = nullptr;
    };

    struct Node {
//        std::pair<size_t, size_t> pos;
//        std::map<size_t, size_t> *occupied = nullptr;
        boost::icl::interval_set<size_t> *occupied = nullptr;
        std::set<VirtualNode *, VirtualNodeSameNodeComp> virtualNodes;
        std::array<Edge, 4> edges;
    };

private:
    // Use a multimap instead of priority_queue to support node replace
    // key: estimateTime (ascent)
    // value: VirtualNode *
    std::multimap<size_t, VirtualNode *> open, closed;

//    std::priority_queue<VirtualNode *, std::vector<VirtualNode *>, VirtualNode> open, closed;
    std::vector<std::vector<Node>> nodes;
    Map *map;
    const Scenario *scenario;
    VirtualNode *successNode = nullptr;
    VirtualNode *maybeSuccessNode = nullptr;
    const int algorithmId;
    bool logging = false;
    size_t deadline;
    bool extraCostFlag;

public:
    bool isOccupied(boost::icl::interval_set<size_t> *occupied, boost::icl::discrete_interval<size_t> interval);

    // isOccupied in [startTime, endTime)
    bool isOccupied(boost::icl::interval_set<size_t> *occupied, size_t startTime, size_t endTime);

    // isOccupied in [startTime, timeStart + 1)
    bool isOccupied(boost::icl::interval_set<size_t> *occupied, size_t timeStart);

    std::pair<size_t, size_t>
    findNotOccupiedInterval(boost::icl::interval_set<size_t> *occupied, size_t startTime, size_t endTime);

    std::pair<size_t, size_t>
    findNotOccupiedInterval(boost::icl::interval_set<size_t> *occupied, size_t startTime);

    size_t findFirstNotOccupiedTimestamp(boost::icl::interval_set<size_t> *occupied, size_t startTime, size_t duration);

    size_t findFirstNotOccupiedTimestamp(boost::icl::interval_set<size_t> *occupied, boost::icl::interval_set<size_t> *occupied2,
                                         size_t startTime, size_t duration);

private:

//    size_t getDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end);

    VirtualNode *createVirtualNode(std::pair<size_t, size_t> pos, size_t leaveTime, VirtualNode *parent,
                                   size_t checkpoint, bool isOpen);

    VirtualNode *createVirtualNode(std::pair<size_t, size_t> pos, size_t leaveTime, VirtualNode *parent,
                                   size_t checkpoint, std::pair<size_t, size_t> child, bool isOpen, bool hasChild = true);

    VirtualNode *removeVirtualNodeFromList(std::multimap<size_t, VirtualNode *> &list,
                                           std::multimap<size_t, VirtualNode *>::iterator it,
                                           bool editNode);

    void addVirtualNodeToList(std::multimap<size_t, VirtualNode *> &list, VirtualNode *vNode, bool editNode);

    void initialize();

    void clean();

    void
    replaceNode(VirtualNode *vNode, std::pair<size_t, size_t> pos, Node &neighborNode, Edge &edge, bool needExamine);

public:
    explicit Solver(Map *map, int algorithmId = 0, bool extraCostFlag = false);

    ~Solver();

    void initScenario(const Scenario *scenario, size_t startTime = 0,
                      size_t deadline = std::numeric_limits<std::size_t>::max() / 2);

    bool success() { return successNode != nullptr; };

    VirtualNode *step();

    std::vector<VirtualNode *> constructPath(VirtualNode *vNode = nullptr);

    void addConstraints(std::vector<Solver::VirtualNode *> vector);

//    void addNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime);

//    void addEdgeOccupied(std::pair<size_t, size_t> pos, Direction direction, size_t startTime, size_t endTime);

    bool isOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime);

    void setLogging(bool flag) { logging = flag; };

    auto getMap() const { return this->map; };

    auto getScenario() const { return this->scenario; };

    auto &getNodes() const { return this->nodes; };

    auto &getOpen() const { return this->open; };

    auto &getClosed() const { return this->closed; };
};


#endif //MAPF_SOLVER_H
