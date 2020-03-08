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

class Solver {
private:
    enum class Direction {
        UP, RIGHT, DOWN, LEFT, NONE
    };
    const Direction directions[4] = {Direction::UP, Direction::RIGHT, Direction::DOWN, Direction::LEFT};


/*    struct TimeSegment {
        size_t startTime;
        size_t duration;

        bool operator()(const TimeSegment &a, const TimeSegment &b) {
            return a.startTime < b.startTime;
        }
    };*/



    struct VirtualNode {
        std::pair<size_t, size_t> pos;  // v
        size_t leaveTime;               // h_v
        size_t estimateTime;            // h_v + g(v), estimated by manhattan distance
        VirtualNode *parent;            // v_p
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
//        std::pair<size_t, size_t> start, end;
        bool available = true;
        std::map<size_t, size_t> *occupied = nullptr;
    };

    struct Node {
//        std::pair<size_t, size_t> pos;
        std::map<size_t, size_t> *occupied = nullptr;
        std::set<VirtualNode *, VirtualNodeSameNodeComp> virtualNodes;
        std::array<Edge, 4> edges;
    };

    struct OccupiedKey {
        std::pair<size_t, size_t> pos;
        Direction direction;
    };

    struct OccupiedKeyHash {
        size_t operator()(OccupiedKey const &occupiedKey) const noexcept {
            auto hash = std::hash<std::size_t>{};
            return hash(occupiedKey.pos.first) ^ (hash(occupiedKey.pos.second) << 1) ^
                   (hash(size_t(occupiedKey.direction)) << 2);
        }
    };

    struct OccupiedKeyEqual {
        constexpr bool operator()(const OccupiedKey &a, const OccupiedKey &b) const {
            return a.pos == b.pos && a.direction == b.direction;
        }
    };

    std::unordered_map<OccupiedKey, std::unique_ptr<std::map<size_t, size_t> >, OccupiedKeyHash, OccupiedKeyEqual> occupiedMap;


    // Use a multimap instead of priority_queue to support node replace
    // key: estimateTime (ascent)
    // value: VirtualNode *
    std::multimap<size_t, VirtualNode *> open, closed;

//    std::priority_queue<VirtualNode *, std::vector<VirtualNode *>, VirtualNode> open, closed;
    std::vector<std::vector<Node>> nodes;
    const Map *map;
    const Scenario *scenario;

    // isOccupied in [startTime, endTime)
    bool isOccupied(std::map<size_t, size_t> *occupied, size_t startTime, size_t endTime);

    // isOccupied in [startTime, timeStart + 1)
    bool isOccupied(std::map<size_t, size_t> *occupied, size_t timeStart);

    std::pair<size_t, size_t>
    findNotOccupiedInterval(std::map<size_t, size_t> *occupied, size_t startTime, size_t endTime);

    std::pair<size_t, size_t>
    findNotOccupiedInterval(std::map<size_t, size_t> *occupied, size_t startTime);

    std::pair<bool, std::pair<size_t, size_t>> getPosByDirection(std::pair<size_t, size_t> pos, Direction direction);

    size_t getDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end);

    VirtualNode *createVirtualNode(std::pair<size_t, size_t> pos, size_t leaveTime, VirtualNode *parent, bool isOpen);

    VirtualNode *removeVirtualNodeFromList(std::multimap<size_t, VirtualNode *> &list,
                                           std::multimap<size_t, VirtualNode *>::iterator it,
                                           bool editNode);

    void addVirtualNodeToList(std::multimap<size_t, VirtualNode *> &list, VirtualNode *vNode, bool editNode);

    void constructPath(VirtualNode *vNode);

    void initialize();

    void clean();

public:
    explicit Solver(const Map *map);

    ~Solver();

    void solve(const Scenario *scenario);

    void addNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime);

    void addEdgeOccupied(std::pair<size_t, size_t> pos, Direction direction, size_t startTime, size_t endTime);
};


#endif //MAPF_SOLVER_H
