//
// Created by liu on 2020/3/7.
//

#include "Solver.h"

#include <iostream>
#include <limits>
#include <algorithm>


bool Solver::isOccupied(std::map<size_t, size_t> *occupied, size_t startTime, size_t endTime) {
    if (!occupied) return false;
    auto it = occupied->upper_bound(startTime);
    if (it != occupied->begin()) --it;
    for (; it != occupied->end(); ++it) {
        if (endTime <= it->first) return false;
        if (startTime < it->second) return true;
    }
    return false;
}

bool Solver::isOccupied(std::map<size_t, size_t> *occupied, size_t timeStart) {
    return isOccupied(occupied, timeStart, timeStart + 1);
}

bool Solver::isOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime) {
    std::map<size_t, size_t> *occupied = nullptr;
    if (direction == Map::Direction::NONE) {
        occupied = nodes[pos.first][pos.second].occupied;
    } else {
        occupied = nodes[pos.first][pos.second].edges[(size_t) direction].occupied;
    }
    return isOccupied(occupied, startTime, endTime);
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
        if (startTime < it->second) return {0, 0};
    }
    // startTime > last occupied interval
    --it;
    return {it->second, std::numeric_limits<size_t>::max()};
}

std::pair<size_t, size_t> Solver::findNotOccupiedInterval(std::map<size_t, size_t> *occupied, size_t startTime) {
    return findNotOccupiedInterval(occupied, startTime, startTime + 1);
}


// find the smallest newTime >= startTime so that [newTime, newTime + duration) is not occupied
size_t Solver::findFirstNotOccupiedTimestamp(std::map<size_t, size_t> *occupied, size_t startTime, size_t duration) {
    if (!occupied || occupied->empty()) return startTime;
    auto it = occupied->upper_bound(startTime);
    if (it != occupied->begin()) --it;
    size_t newTime = startTime;
    for (; it != occupied->end(); ++it) {
        if (startTime <= newTime && newTime + duration <= it->first) {
            return newTime;
        }
        newTime = it->second;
    }
    return std::max(startTime, newTime);
}

// find the smallest newTime >= startTime so that [newTime, newTime + duration) is not occupied,
// and newTime + duration is not occupied2
size_t Solver::findFirstNotOccupiedTimestamp(std::map<size_t, size_t> *occupied, std::map<size_t, size_t> *occupied2,
                                             size_t startTime, size_t duration) {
    if (!occupied || occupied->empty()) {
        return findFirstNotOccupiedTimestamp(occupied2, startTime + duration, 1) - duration;
    }
    auto it = occupied->upper_bound(startTime);
    if (it != occupied->begin()) --it;
    size_t newTime = startTime;
    for (; it != occupied->end(); ++it) {
        if (startTime <= newTime && newTime + duration <= it->first && !isOccupied(occupied2, newTime + duration)) {
            return newTime;
        }
        newTime = it->second;
    }
    return findFirstNotOccupiedTimestamp(occupied2, std::max(startTime, newTime) + duration, 1) - duration;
}

/*size_t Solver::getDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end) {
    size_t distance = 0;
    if (start.first > end.first) distance += start.first - end.first;
    else distance += end.first - start.first;
    if (start.second > end.second) distance += start.second - end.second;
    else distance += end.second - start.second;
    return distance;
}*/

Solver::VirtualNode *
Solver::createVirtualNode(std::pair<size_t, size_t> pos, size_t leaveTime, Solver::VirtualNode *parent,
                          size_t checkpoint, bool isOpen) {
    return createVirtualNode(pos, leaveTime, parent, checkpoint, std::make_pair(0, 0), isOpen, false);
//    size_t estimateTime = leaveTime + Map::getDistance(pos, scenario->getEnd());
//    return new VirtualNode{pos, leaveTime, estimateTime, parent, std::make_pair(0, 0), checkpoint, false, isOpen};
}

Solver::VirtualNode *
Solver::createVirtualNode(std::pair<size_t, size_t> pos, size_t leaveTime, Solver::VirtualNode *parent,
                          size_t checkpoint, std::pair<size_t, size_t> child, bool isOpen, bool hasChild) {
//    size_t checkpoint = parent ? parent->checkpoint : 0;
    size_t estimateTime = leaveTime;
    if (checkpoint < scenario->size()) {
        estimateTime += Map::getDistance(pos, scenario->getEnd(checkpoint));
        estimateTime += scenario->getDistance(checkpoint);
    } else {
        estimateTime += Map::getDistance(pos, scenario->getEnd());
    }
//    std::cout << pos.first << " " << pos.second << " " << checkpoint << " " << leaveTime << " " << estimateTime << std::endl;
//    size_t estimateTime = leaveTime + Map::getDistance(pos, scenario->getEnd());
    return new VirtualNode{pos, leaveTime, estimateTime, parent, child, checkpoint, hasChild, isOpen};
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
    if (vNode->estimateTime >= deadline) {
        delete vNode;
        return;
    }
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
    for (const auto &item: map->getOccupiedMap()) {
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


Solver::Solver(Map *map, int algorithmId) : map(map), algorithmId(algorithmId) {

}

Solver::~Solver() {
    clean();
}

void Solver::initScenario(const Scenario *scenario, size_t startTime, size_t deadline) {
    this->scenario = scenario;
    this->deadline = deadline;

    // skip the algorithm if start or end point is blocked
    auto start = scenario->getStart();
    auto end = scenario->getEnd();
    if ((*map)[start.first][start.second] != '.' || (*map)[end.first][end.second] != '.') {
        clean();
        return;
    }

    // Initialize the OPEN and CLOSED lists to be empty
    initialize();

    // Construct a virtual node (v', h_v', null), added into the OPEN list
    auto startVNode = createVirtualNode(scenario->getStart(), startTime, nullptr, 0, true);
    addVirtualNodeToList(open, startVNode, true);
}

void Solver::replaceNode(VirtualNode *vNode, std::pair<size_t, size_t> pos,
                         Node &neighborNode, Edge &edge, bool needExamine) {
    auto arrivalTime = vNode->leaveTime + 1; // h_v + L_e (L_e = 1 now)

    // if h_v + L_e not in O_{\bar{v}} and (h_v, h_v + L_e) /\ O_e = 0
    auto arrivalInterval = findNotOccupiedInterval(neighborNode.occupied, arrivalTime);
    if (!needExamine || (arrivalInterval.first != arrivalInterval.second &&
                         !isOccupied(edge.occupied, vNode->leaveTime, arrivalTime))) {


        // create the new node first to help search in the virtualNodes of a node
        // use arrivalTime + 1 to prevent corner condition mistakes
        auto newNode = createVirtualNode(pos, arrivalTime + 1, vNode, vNode->checkpoint, true);

        // if there exists any virtual node in the OPEN or CLOSED list such that
        // it is in the same or future checkpoint and
        // h' and h_v+L_e are in the same interval and h' <= h_v+L_e, flag will be false
        auto flag = true;
        for (auto it2 = neighborNode.virtualNodes.begin();
             it2 != neighborNode.virtualNodes.lower_bound(newNode); ++it2) {
            if ((*it2)->leaveTime <= arrivalTime && (*it2)->leaveTime >= arrivalInterval.first
                && !(*it2)->hasChild && (*it2)->checkpoint >= vNode->checkpoint) {
                flag = false;
                break;
            }
        }

        if (flag) {
            // set the leaveTime back to arrivalTime (-1)
            size_t heuristicTime = newNode->estimateTime - newNode->leaveTime;
            newNode->leaveTime = arrivalTime;

            // delete all virtual node in the OPEN list such that
            // h' and h_v+L_e are in the same interval and h' > h_v+L_e
            size_t deleteCount = 0;
            for (auto it2 = neighborNode.virtualNodes.upper_bound(newNode);
                 it2 != neighborNode.virtualNodes.end();) {
                if ((*it2)->isOpen && (*it2)->leaveTime > arrivalTime &&
                    (*it2)->leaveTime < arrivalInterval.second && !(*it2)->hasChild &&
                    (*it2)->checkpoint <= vNode->checkpoint) {
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
                    ++deleteCount;
                } else {
                    ++it2;
                }
            }
            if (deleteCount > 1) {
                std::cerr << "delete > 1" << std::endl;
            }
            newNode->estimateTime = newNode->leaveTime + heuristicTime;
            addVirtualNodeToList(open, newNode, true);
        } else {
            delete newNode;
        }
    }
}


Solver::VirtualNode *Solver::step() {
    if (open.empty()) return nullptr;

    // Get a virtual node (v, h_v, v_p) off the OPEN list with the minimum h + g(v) value
    auto it = open.begin();
    auto vNode = removeVirtualNodeFromList(open, it, false);
    auto &node = nodes[vNode->pos.first][vNode->pos.second];

//    if (vNode->leaveTime >= deadline) {
//        return nullptr;
//    }

    // Add (v, h_v, v_p) to the CLOSED list;
    vNode->isOpen = false;
    addVirtualNodeToList(closed, vNode, false);


    while (vNode->pos == scenario->getEnd(vNode->checkpoint)) {
        // if v is the goal location v''
        if (vNode->checkpoint == scenario->size() - 1) {
            if (!vNode->hasChild) {
                // we have found the solution and exit the algorithm
                successNode = vNode;
                return vNode;
            }
            break;
        } else {
            vNode->checkpoint++;
        }
    }



    if (algorithmId == 0) {
        bool waitFlag = false;
//        bool waitFlag = true;

        // for each neighbouring node \bar{v} of v do
        for (auto direction : Map::directions) {
            auto &edge = node.edges[(size_t) direction];
            if (!edge.available) continue; // no node
            auto p = map->getPosByDirection(vNode->pos, direction);
            auto &neighborNode = nodes[p.second.first][p.second.second];

            size_t cv = 0;
            if (neighborNode.occupied && !neighborNode.occupied->empty()) {
                auto it2 = neighborNode.occupied->rbegin();
                cv = it2->second;
            }
            if (((vNode->parent && p.second != vNode->parent->pos) || !vNode->parent) && vNode->leaveTime + 1 < cv) {
                waitFlag = true;
            }

//            if (vNode->checkpoint == 1 && p.second.first == 19) {
//                std::cout << p.second.first << " " << p.second.second << " " << vNode->estimateTime << std::endl;
//            }

            replaceNode(vNode, p.second, neighborNode, edge, true);
        }

        // if h_v + 1 not in O_v
        if (waitFlag && !isOccupied(node.occupied, vNode->leaveTime + 1)) {
            // Add (v, h_v+1, v_p) to the OPEN list;
            auto newNode = createVirtualNode(vNode->pos, vNode->leaveTime + 1, vNode->parent, vNode->checkpoint, true);
            addVirtualNodeToList(open, newNode, true);
        }

    } else if (algorithmId == 1) {
        if (!vNode->hasChild) {
            for (auto direction : Map::directions) {
                auto &edge = node.edges[(size_t) direction];
                if (!edge.available) continue; // no node
                auto p = map->getPosByDirection(vNode->pos, direction);
                auto &neighborNode = nodes[p.second.first][p.second.second];
                if (vNode->parent && p.second == vNode->parent->pos) continue; // v_n=v_p

                if (logging && p.second.first == 9 && p.second.second == 25) {
                    std::cerr << "debug" << std::endl;
                }

                auto newTime = findFirstNotOccupiedTimestamp(edge.occupied, neighborNode.occupied, vNode->leaveTime, 1);
                auto waitInterval = findNotOccupiedInterval(node.occupied, vNode->leaveTime, newTime);

                if (logging) {
                    std::cout << vNode->pos.first << " " << vNode->pos.second << " " << vNode->leaveTime << " -> "
                              << p.second.first << " " << p.second.second << " " << newTime << std::endl;
                }

                if (newTime < std::numeric_limits<size_t>::max() / 2 && waitInterval.first < waitInterval.second) {
                    auto newNode = createVirtualNode(vNode->pos, newTime, vNode->parent, vNode->checkpoint, p.second, true);
                    addVirtualNodeToList(open, newNode, true);
                }
            }
        } else {
            auto direction = map->getDirectionByPos(vNode->pos, vNode->child);
            auto &edge = node.edges[(size_t) direction];
            auto &neighborNode = nodes[vNode->child.first][vNode->child.second];

            if (neighborNode.occupied && !neighborNode.occupied->empty()) {
                auto it2 = neighborNode.occupied->upper_bound(vNode->leaveTime + 1);
                if (it2 != neighborNode.occupied->end()) {
                    auto newTime = findFirstNotOccupiedTimestamp(edge.occupied, neighborNode.occupied, it2->first, 1);
                    auto waitInterval = findNotOccupiedInterval(node.occupied, vNode->leaveTime, newTime);
//                    std::cout << vNode->leaveTime << " " << it2->first << " " << newTime << std::endl;

                    if (newTime < std::numeric_limits<size_t>::max() / 2 && waitInterval.first < waitInterval.second) {
                        auto newNode = createVirtualNode(vNode->pos, newTime, vNode->parent, vNode->checkpoint, vNode->child, true);
                        addVirtualNodeToList(open, newNode, true);
                    }
                }
            }

            replaceNode(vNode, vNode->child, neighborNode, edge, false);
        }
    }


    return vNode;
}

void Solver::addConstraints(std::vector<Solver::VirtualNode *> vector) {
    std::reverse(vector.begin(), vector.end());
    map->addNodeOccupied(vector[0]->pos, 0, vector[0]->leaveTime + 1);
    for (size_t j = 1; j < vector.size(); j++) {
        size_t endTime = vector[j]->leaveTime + 1;
        if (j == vector.size() - 1) endTime = std::numeric_limits<size_t>::max() / 2;
        map->addNodeOccupied(vector[j]->pos, vector[j - 1]->leaveTime + 1, endTime);
        auto dir = map->getDirectionByPos(vector[j - 1]->pos, vector[j]->pos);
        if (dir == Map::Direction::NONE) {
            throw std::runtime_error("");
        }
        map->addEdgeOccupied(vector[j - 1]->pos, dir, vector[j - 1]->leaveTime,
                             vector[j - 1]->leaveTime + 1);
    }
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









