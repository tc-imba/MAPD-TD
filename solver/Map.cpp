//
// Created by liu on 2020/3/7.
//

#include "Map.h"
#include "Solver.h"

#include <fstream>
#include <sstream>
#include <exception>
#include <iostream>
#include <algorithm>

const Map::Direction Map::directions[4] = {Map::Direction::UP, Map::Direction::RIGHT,
                                           Map::Direction::DOWN, Map::Direction::LEFT};

const int Map::DIRECTION_X[4] = {-1, 0, 1, 0};
const int Map::DIRECTION_Y[4] = {0, 1, 0, -1};

template<typename T>
void Map::parseHeader(const std::string &line, const std::string &key, T &value) {
    std::string temp;
    std::istringstream iss;
    iss.str(line);
    iss >> temp >> value;
    if (temp != key) {
        throw std::runtime_error("map key error");
    }
}

void Map::calculateDistances() {
    size_t size = width * height;
    distances.resize(size, std::vector<size_t>(size, std::numeric_limits<size_t>::max() / 2));
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            size_t a = i * width + j;
            distances[a][a] = 0;
            if (map[i][j] != '@') {
                for (auto direction : Map::directions) {
                    auto p = getPosByDirection({i, j}, direction);
                    if (p.first && map[p.second.first][p.second.second] != '@') {
                        size_t b = p.second.first * width + p.second.second;
                        distances[a][b] = 1;
                    }
                }
            }
        }
    }
    for (size_t k = 0; k < size; k++) {
        for (size_t i = 0; i < size; i++) {
            for (size_t j = 0; j < size; j++) {
                if (distances[i][j] > distances[i][k] + distances[k][j]) {
                    distances[i][j] = distances[i][k] + distances[k][j];
                }
            }
        }
    }
}


void Map::readDistances(const std::string &filename, std::vector<std::vector<size_t>> &result) {
    size_t size = width * height;
    std::istringstream iss;
    std::ifstream fin;
    std::string line;

    result.resize(size, std::vector<size_t>(size, std::numeric_limits<size_t>::max() / 2));
    fin.open(filename);
    if (!fin.is_open()) {
        throw std::runtime_error("map distance file not found: " + filename);
    }
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        iss.clear();
        iss.str(line);
        size_t x1, y1, x2, y2, distance;
        iss >> x1 >> y1 >> x2 >> y2 >> distance;
        result[x1 * width + y1][x2 * width + y2] = distance;
    }
    fin.close();
    std::cerr << "Map " << filename << " distances imported" << std::endl;
}

Map::Map(const std::string &filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        throw std::runtime_error("map file not found");
    }
    std::string line;

    // Get map type
    std::getline(fin, line);
    parseHeader(line, "type", this->type);
    if (this->type != "octile") {
        throw std::runtime_error("map type error");
    }

    // Get map height
    std::getline(fin, line);
    parseHeader(line, "height", this->height);
    if (this->height == 0) {
        throw std::runtime_error("map height error");
    }
    std::getline(fin, line);
    parseHeader(line, "width", this->width);
    if (this->width == 0) {
        throw std::runtime_error("map width error");
    }

    // Get map
    std::getline(fin, line);
    if (line != "map") {
        throw std::runtime_error("map format error");
    }
    this->map.resize(this->height);
    this->extraCost.resize(this->height);
    for (size_t i = 0; i < this->height; i++) {
        this->map[i].resize(this->width, '.');
        this->extraCost[i].resize(this->width, false);
        if (!std::getline(fin, line) || line.length() < this->width) {
            throw std::runtime_error("map format error");
        }
        for (size_t j = 0; j < this->width; j++) {
            this->map[i][j] = line[j];
            if (this->map[i][j] == 'p') {
                parkingLocations.emplace_back(i, j);
            }
        }
    }
    fin.close();
    std::cerr << "Map " << filename << " imported" << std::endl;

    readDistances(filename + ".distance", distances);
    readDistances(filename + ".endpoint.distance", distancesEndpoint);
}

const std::vector<char> &Map::operator[](size_t index) const {
    return this->map[index];
}

std::pair<bool, std::pair<size_t, size_t>>
Map::getPosByDirection(std::pair<size_t, size_t> pos, Direction direction) const {
    bool flag = true;
    pos.first += DIRECTION_X[(size_t) direction];
    pos.second += DIRECTION_Y[(size_t) direction];
    if (pos.first >= height || pos.second >= width) flag = false;
    return std::make_pair(flag, pos);
}

void Map::addNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime) {
    addEdgeOccupied(pos, Map::Direction::NONE, startTime, endTime);
}

void Map::removeNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime) {
    removeEdgeOccupied(pos, Map::Direction::NONE, startTime, endTime);
}

static void addOccupied(std::pair<size_t, size_t> pos, Map::Direction direction,
                        boost::icl::interval_set<size_t> &occupied,
                        boost::icl::discrete_interval<size_t> &interval) {
/*    if (pos.first == 7 && pos.second == 5 && direction == Map::Direction::NONE) {
        std::cerr << "add " << interval << " to " << occupied << std::endl;
    }
    if (boost::icl::intersects(occupied, interval)) {
        std::cerr << "add error: " << pos.first << " " << pos.second << " "
                  << occupied << " " << interval << std::endl;
        exit(0);
    }*/
//    std::cout << "add " << interval << " to " << occupied << std::endl;
    occupied.add(interval);
}

static void removeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction,
                           boost::icl::interval_set<size_t> &occupied,
                           boost::icl::discrete_interval<size_t> &interval) {
/*    if (!boost::icl::contains(occupied, interval)) {
        std::cerr << "remove error: " << pos.first << " " << pos.second << " "
                  << occupied << " " << interval << std::endl;
        exit(0);
    }*/
//    std::cout << "remove " << interval << " from " << occupied << std::endl;
    occupied.subtract(interval);
}

size_t Map::addInfiniteWaiting(std::pair<size_t, size_t> pos, size_t startTime) {
    extraCost[pos.first][pos.second] = startTime;
    OccupiedKey key = {pos, Map::Direction::NONE};
    size_t infinite = std::numeric_limits<size_t>::max() / 2;
    if (startTime == 0) {
        auto it = occupiedMap.find(key);
        if (it != occupiedMap.end()) {
            auto occupied = &it->second->rangeConstraints;
            if (!occupied->empty()) {
                auto it2 = occupied->rbegin();
                if (it2->upper() < infinite) {
                    startTime = it2->upper() + 1;
                    auto interval = boost::icl::discrete_interval<size_t>(startTime, infinite);
                    addOccupied(pos, Map::Direction::NONE, *occupied, interval);
                    it->second->infiniteWaiting = startTime;
                    return startTime;
                }
            }
        }
    }
    addNodeOccupied(pos, startTime, infinite);
    auto it = occupiedMap.find(key);
    if (it != occupiedMap.end()) {
        it->second->infiniteWaiting = startTime;
    }
    return startTime;
}

size_t Map::removeInfiniteWaiting(std::pair<size_t, size_t> pos) {
    extraCost[pos.first][pos.second] = std::numeric_limits<size_t>::max();
    OccupiedKey key = {pos, Map::Direction::NONE};
    size_t infinite = std::numeric_limits<size_t>::max() / 2;
    auto it = occupiedMap.find(key);
    if (it != occupiedMap.end()) {
        auto occupied = &it->second->rangeConstraints;
        if (!occupied->empty()) {
            auto it2 = occupied->rbegin();
            if (it2->upper() >= infinite) {
                size_t result = it2->lower();
                if (result < it->second->infiniteWaiting) {
                    result = it->second->infiniteWaiting;
                }
                auto interval = boost::icl::discrete_interval<size_t>(result, it2->upper());
//                    std::cerr << "infinite waiting error: " << it2->lower() << " " << it->second->infiniteWaiting << std::endl;
                removeOccupied(pos, Map::Direction::NONE, *occupied, interval);
//                occupied->subtract(interval);
//                occupied->erase(it2);
                return result;
            }
        }
    }
    return infinite;
}

size_t Map::getExtraCostTime(std::pair<size_t, size_t> pos) {
    return extraCost[pos.first][pos.second];
}

void Map::addEdgeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime) {
    if (endTime <= startTime) return;

//    if (pos.first == 15 && pos.second == 20 && startTime == 44) {
//        std::cerr << startTime << " " << endTime << std::endl;
//    }

    if (direction == Map::Direction::LEFT) {
        pos = getPosByDirection(pos, direction).second;
        direction = Map::Direction::RIGHT;
    } else if (direction == Map::Direction::UP) {
        pos = getPosByDirection(pos, direction).second;
        direction = Map::Direction::DOWN;
    }

    OccupiedKey key = {pos, direction};
    auto interval = boost::icl::discrete_interval<size_t>(startTime, endTime);

    auto it = occupiedMap.find(key);
    if (it == occupiedMap.end()) {
        auto occupied = std::make_unique<OccupiedValue>();
        addOccupied(pos, direction, occupied->rangeConstraints, interval);
        occupiedMap.emplace_hint(it, key, std::move(occupied));
    } else {
        auto occupied = it->second.get();
//        if (boost::icl::intersects(occupied->rangeConstraints, interval)) {
//            std::cerr << "add error: " << pos.first << " " << pos.second << " "
//                      << occupied->rangeConstraints << " " << startTime << " " << endTime
//                      << std::endl;
//            exit(0);
//        }
        addOccupied(pos, direction, occupied->rangeConstraints, interval);
    }
}

void Map::removeEdgeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime,
                             size_t endTime) {
    if (endTime <= startTime) return;

    if (direction == Map::Direction::LEFT) {
        pos = getPosByDirection(pos, direction).second;
        direction = Map::Direction::RIGHT;
    } else if (direction == Map::Direction::UP) {
        pos = getPosByDirection(pos, direction).second;
        direction = Map::Direction::DOWN;
    }

    OccupiedKey key = {pos, direction};
    auto interval = boost::icl::discrete_interval<size_t>(startTime, endTime);

    auto it = occupiedMap.find(key);
    if (it != occupiedMap.end()) {
        auto &occupied = it->second->rangeConstraints;
//        if (!boost::icl::contains(occupied, interval)) {
//            std::cerr << "remove error: " << pos.first << " " << pos.second << " "
//                      << occupied << " " << startTime << " " << endTime << std::endl;
//            exit(0);
//        }
        removeOccupied(pos, direction, occupied, interval);
    } else {
        std::cerr << "remove error: not found" << std::endl;
    }
}

void Map::addWaitingAgent(std::pair<size_t, size_t> pos, size_t startTime, size_t agent) {
    OccupiedKey key = {pos, Map::Direction::NONE};
    auto it = occupiedMap.find(key);
    if (it == occupiedMap.end()) {
        auto occupied = std::make_unique<OccupiedValue>();
        it = occupiedMap.emplace_hint(it, key, std::move(occupied));
    }
    auto &waitingAgents = it->second->waitingAgents;
    auto it2 = waitingAgents.find(startTime);
    if (it2 == waitingAgents.end()) {
        it2 = waitingAgents.emplace_hint(it2, startTime, agent);
        // update node constraint
//        if (++it2 == waitingAgents.end()) {
//            if (waitingAgents.size() > 1) {
//                auto it3 = ++waitingAgents.rbegin();
//                removeNodeOccupied(pos, it3->first);
//            }
//            addNodeOccupied(pos, startTime, std::numeric_limits<size_t>::max() / 2);
//        }
    } else {
        printOccupied(&waitingAgents);
        std::cerr << "warning: adding duplicate waiting agent" << std::endl;
    }
}

void Map::removeWaitingAgent(std::pair<size_t, size_t> pos, size_t startTime, size_t agent) {
    OccupiedKey key = {pos, Map::Direction::NONE};
    auto it = occupiedMap.find(key);
    if (it == occupiedMap.end()) {
        std::cerr << "warning: removing non-exist waiting agent" << std::endl;
        return;
    }
    auto &waitingAgents = it->second->waitingAgents;
    auto it2 = waitingAgents.find(startTime);
    if (it2 == waitingAgents.end()) {
        std::cerr << "warning: removing non-exist waiting agent" << std::endl;
    } else if (it2->second != agent) {
        std::cerr << "warning: removing wrong waiting agent" << std::endl;
    } else {
        it2 = waitingAgents.erase(it2);
        // update node constraint
//        if (it2 == waitingAgents.end()) {
//            removeNodeOccupied(pos, startTime);
//            if (!waitingAgents.empty()) {
//                auto it3 = waitingAgents.rbegin();
//                addNodeOccupied(pos, it3->first, std::numeric_limits<size_t>::max() / 2);
//            }
//        }
    }
}

size_t Map::getLastWaitingAgent(std::pair<size_t, size_t> pos) {
    constexpr size_t noAgent = std::numeric_limits<size_t>::max() / 2;
    OccupiedKey key = {pos, Map::Direction::NONE};
    auto it = occupiedMap.find(key);
    if (it == occupiedMap.end()) {
        return noAgent;
    }
    auto &waitingAgents = it->second->waitingAgents;
    if (waitingAgents.empty()) {
        return noAgent;
    }
    return waitingAgents.rbegin()->second;
}


bool Map::loadConstraints(const std::string &filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) return false;
    std::string line;
    std::istringstream iss;
    while (std::getline(fin, line)) {
        if (line.empty()) continue;
        iss.clear();
        iss.str(line);
        std::pair<size_t, size_t> pos;
        int direction;
        size_t startTime, endTime;
        iss >> pos.first >> pos.second >> direction >> startTime >> endTime;
        if (direction >= 0) {
            addEdgeOccupied(pos, Direction(direction), startTime, endTime);
        } else {
            addNodeOccupied(pos, startTime, endTime);
        }
    }
    return true;
}

Map::Direction Map::getDirectionByPos(std::pair<size_t, size_t> pos1, std::pair<size_t, size_t> pos2) const {
    if (pos1.first > pos2.first && pos1.second == pos2.second) return Direction::UP;
    if (pos1.first < pos2.first && pos1.second == pos2.second) return Direction::DOWN;
    if (pos1.first == pos2.first && pos1.second > pos2.second) return Direction::LEFT;
    if (pos1.first == pos2.first && pos1.second < pos2.second) return Direction::RIGHT;
    return Direction::NONE;
}

void Map::printOccupied(std::map<size_t, size_t> *occupied) {
    for (auto item : *occupied) {
        std::cerr << "[" << item.first << "," << item.second << ") ";
    }
}

std::string Map::printOccupiedMap() const {
    std::ostringstream oss;
    for (auto it = occupiedMap.begin(); it != occupiedMap.end(); ++it) {
        oss << it->first.pos.first << " " << it->first.pos.second << " " << (int) it->first.direction << ": ";
        oss << it->second->rangeConstraints << std::endl;
    }
    return oss.str();
}

size_t Map::getDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end) {
    size_t distance = 0;
    if (start.first > end.first) distance += start.first - end.first;
    else distance += end.first - start.first;
    if (start.second > end.second) distance += start.second - end.second;
    else distance += end.second - start.second;
    return distance;
}

size_t Map::getGraphDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end) {
    size_t a = start.first * width + start.second;
    size_t b = end.first * width + end.second;
    return distances[a][b];
}

size_t Map::getGraphDistanceEndpoint(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end) {
    size_t a = start.first * width + start.second;
    size_t b = end.first * width + end.second;
    return distancesEndpoint[a][b];
}
