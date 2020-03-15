//
// Created by liu on 2020/3/7.
//

#include "Solver.h"

#include <iostream>
#include <limits>



bool Solver::isOccupied(std::map<size_t, size_t> *occupied, size_t startTime, size_t endTime) {
    if (!occupied) return false;
    auto it = occupied->upper_bound(startTime);
    if (it != occupied->begin()) --it;
    for (; it != occupied->end(); ++it) {
        if (endTime <= it->first) return false;
        if (startTime < it->first + it->second) return true;
    }
    return false;
}

bool Solver::isOccupied(std::map<size_t, size_t> *occupied, size_t timeStart) {
    return isOccupied(occupied, timeStart, timeStart + 1);
}

std::pair<size_t, size_t>
Solver::findNotOccupiedInterval(std::map<size_t, size_t> *occupied, size_t startTime, size_t endTime) {
    if (!occupied || occupied->empty()) return {0, std::numeric_limits<size_t>::max()};
    auto it = occupied->upper_bound(startTime);
    if (it != occupied->begin()) --it;
    for (; it != occupied->end(); ++it) {
        if (endTime <= it->first) {
            if (it == occupied->begin()) return {0, it->first};
            auto it2 = std::prev(it);
            return {it2->second, it->first};
        }
        if (startTime < it->first + it->second) return {0, 0};
    }
    // startTime > last occupied interval
    --it;
    return {it->second, std::numeric_limits<size_t>::max()};
}

std::pair<size_t, size_t> Solver::findNotOccupiedInterval(std::map<size_t, size_t> *occupied, size_t startTime) {
    return findNotOccupiedInterval(occupied, startTime, startTime + 1);
}

size_t Solver::getDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end) {
    size_t distance = 0;
    if (start.first > end.first) distance += start.first - end.first;
    else distance += end.first - start.first;
    if (start.second > end.second) distance += start.second - end.second;
    else distance += end.second - start.second;
    return distance;
}

Solver::VirtualNode *
Solver::createVirtualNode(std::pair<size_t, size_t> pos, size_t leaveTime, Solver::VirtualNode *parent, bool isOpen) {
    size_t estimateTime = leaveTime + getDistance(pos, scenario->getEnd());
    return new VirtualNode{pos, leaveTime, estimateTime, parent, isOpen};
}


Solver::VirtualNode *Solver::removeVirtualNodeFromList(std::multimap<size_t, VirtualNode *> &list,
                                                       std::multimap<size_t, VirtualNode *>::iterator it,
                                                       bool editNode) {
    auto vNode = it->second;
    list.erase(it);
    if (editNode) {
        auto &node = nodes[vNode->pos.first][vNode->pos.second];
        node.virtualNodes.erase(vNode);
    }
    return vNode;
}

void Solver::addVirtualNodeToList(std::multimap<size_t, VirtualNode *> &list,
                                  Solver::VirtualNode *vNode, bool editNode) {
    list.emplace(vNode->estimateTime, vNode);
    if (editNode) {
        auto &node = nodes[vNode->pos.first][vNode->pos.second];
        node.virtualNodes.emplace(vNode);
    }
}

std::vector<Solver::VirtualNode *> Solver::constructPath(VirtualNode *vNode) {
    if (!vNode) vNode = successNode;
    std::vector<VirtualNode *> vector;
    while (vNode) {
        vector.emplace_back(vNode);
        vNode = vNode->parent;
    }
    return vector;
}

void Solver::initialize() {
    clean();
    nodes.resize(map->getHeight());
    for (size_t i = 0; i < map->getHeight(); i++) {
        nodes[i].resize(map->getWidth());
        for (size_t j = 0; j < map->getWidth(); j++) {
            for (auto direction : Map::directions) {
                auto p = map->getPosByDirection({i, j}, direction);
                if (!p.first || (*map)[p.second.first][p.second.second] != '.') {
                    nodes[i][j].edges[(size_t) direction].available = false;
                }
            }
        }
    }
    for (const auto &item: constraints.getOccupiedMap()) {
        if (item.first.direction == Map::Direction::NONE) {
            nodes[item.first.pos.first][item.first.pos.second].occupied = item.second.get();
        } else {
            auto &node1 = nodes[item.first.pos.first][item.first.pos.second];
            auto dir1 = (size_t) item.first.direction;
            node1.edges[dir1].occupied = item.second.get();
            auto p = map->getPosByDirection(item.first.pos, item.first.direction);
            if (p.first) {
                auto &node2 = nodes[p.second.first][p.second.second];
                auto dir2 = (dir1 + 2) % 4;
                node2.edges[dir2].occupied = item.second.get();
            }
        }
    }
}

void Solver::clean() {
    nodes.clear();
    for (auto item : open) {
        delete item.second;
    }
    for (auto item : closed) {
        delete item.second;
    }
    open.clear();
    closed.clear();
    successNode = nullptr;
}


Solver::Solver(const Map *map) : map(map), constraints(map) {

}

Solver::~Solver() {
    clean();
}

void Solver::initScenario(const Scenario *scenario) {
    this->scenario = scenario;

    // Initialize the OPEN and CLOSED lists to be empty
    initialize();

    // Construct a virtual node (v', h_v', null), added into the OPEN list
    auto startVNode = createVirtualNode(scenario->getStart(), 0, nullptr, true);
    addVirtualNodeToList(open, startVNode, true);

    // while the OPEN list is not empty
//    while (!open.empty()) {
//
//
//    }
}

Solver::VirtualNode *Solver::step() {
    if (open.empty()) return nullptr;

    // Get a virtual node (v, h_v, v_p) off the OPEN list with the minimum h + g(v) value
    auto it = open.begin();
    auto vNode = removeVirtualNodeFromList(open, it, false);
    auto &node = nodes[vNode->pos.first][vNode->pos.second];

    // Add (v, h_v, v_p) to the CLOSED list;
    vNode->isOpen = false;
    addVirtualNodeToList(closed, vNode, false);

    // if v is the goal location v''
    if (vNode->pos == scenario->getEnd()) {
        // we have found the solution and exit the algorithm
        successNode = vNode;
//        constructPath(vNode);
        return vNode;
    }

    // if h_v + 1 not in O_v
    if (!isOccupied(node.occupied, vNode->leaveTime + 1)) {
        // Add (v, h_v+1, v_p) to the OPEN list;
        auto newNode = createVirtualNode(vNode->pos, vNode->leaveTime + 1, vNode->parent, true);
        addVirtualNodeToList(open, newNode, true);
    }

    // for each neighbouring node \bar{v} of v do
    for (auto direction : Map::directions) {
        auto &edge = node.edges[(size_t) direction];
        if (!edge.available) continue; // no node
        auto p = map->getPosByDirection(vNode->pos, direction);
        auto &neighborNode = nodes[p.second.first][p.second.second];
        auto arrivalTime = vNode->leaveTime + 1; // h_v + L_e (L_e = 1 now)

        // if h_v + L_e not in O_{\bar{v}} and (h_v, h_v + L_e) /\ O_e = 0
        auto arrivalInterval = findNotOccupiedInterval(neighborNode.occupied, arrivalTime);
        if (arrivalInterval.first != arrivalInterval.second &&
            !isOccupied(edge.occupied, vNode->leaveTime, arrivalTime)) {

            // create the new node first to help search in the virtualNodes of a node
            // use arrivalTime + 1 to prevent corner condition mistakes
            auto newNode = createVirtualNode(p.second, arrivalTime + 1, vNode, true);

            // if there exists any virtual node in the OPEN or CLOSED list such that
            // h' and h_v+L_e are in the same interval and h' <= h_v+L_e, flag will be false
            auto flag = true;
            for (auto it2 = neighborNode.virtualNodes.begin();
                 it2 != neighborNode.virtualNodes.lower_bound(newNode); ++it2) {
                if ((*it2)->leaveTime <= arrivalTime && (*it2)->leaveTime >= arrivalInterval.first) {
                    flag = false;
                    break;
                }
            }

            if (flag) {
                // set the leaveTime back to arrivalTime (-1)
                newNode->leaveTime = arrivalTime;

                // delete all virtual node in the OPEN list such that h′ and
                // h' and h_v+L_e are in the same interval and h' > h_v+L_e
                for (auto it2 = neighborNode.virtualNodes.upper_bound(newNode);
                     it2 != neighborNode.virtualNodes.end();) {
                    if ((*it2)->isOpen && (*it2)->leaveTime > arrivalTime &&
                        (*it2)->leaveTime < arrivalInterval.second) {
                        auto range = open.equal_range((*it2)->estimateTime);
                        for (auto it3 = range.first; it3 != range.second; ++it3) {
                            if (it3->second == *it2) {
                                open.erase(it3);
                                break;
                            }
                        }
                        auto deleteVNode = *it2;
                        it2 = neighborNode.virtualNodes.erase(it2);
                        delete deleteVNode;
                    } else {
                        ++it2;
                    }
                }
                addVirtualNodeToList(open, newNode, true);
            } else {
                delete newNode;
            }
        }
    }
    return vNode;
}

/*
void Solver::addNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime) {
    addEdgeOccupied(pos, Direction::NONE, startTime, endTime);
}

void
Solver::addEdgeOccupied(std::pair<size_t, size_t> pos, Direction direction, size_t startTime, size_t endTime) {
    if (direction == Direction::LEFT) {
        pos = getPosByDirection(pos, direction).second;
        direction = Direction::RIGHT;
    } else if (direction == Direction::UP) {
        pos = getPosByDirection(pos, direction).second;
        direction = Direction::DOWN;
    }

    OccupiedKey key = {pos, direction};
    auto it = occupiedMap.find(key);
    if (it == occupiedMap.end()) {
        auto occupied = std::make_unique<std::map<size_t, size_t>>();
        occupied->emplace(startTime, endTime - startTime);
        occupiedMap.emplace_hint(it, key, std::move(occupied));
    } else {
        auto occupied = it->second.get();
        auto it2 = occupied->upper_bound(startTime);
        if (it2 != occupied->begin()) --it2;
        while (it2 != occupied->end()) {
            if (it2->first <= endTime) {
                endTime = std::max(it2->second, endTime);
                it2 = occupied->erase(it2);
            } else if (startTime > it2->second) {
                break;
            }
        }
        occupied->emplace(startTime, endTime - startTime);
    }
}


*/









