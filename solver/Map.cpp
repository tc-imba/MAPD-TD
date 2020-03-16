//
// Created by liu on 2020/3/7.
//

#include "Map.h"

#include <fstream>
#include <sstream>
#include <exception>
#include <iostream>

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
    for (size_t i = 0; i < this->height; i++) {
        this->map[i].resize(this->width, '.');
        if (!std::getline(fin, line) || line.length() < this->width) {
            throw std::runtime_error("map format error");
        }
        for (size_t j = 0; j < this->width; j++) {
            this->map[i][j] = line[j];
        }
    }

    std::cerr << "Map " << filename << " imported" << std::endl;
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

void Map::addEdgeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime) {
    if (direction == Map::Direction::LEFT) {
        pos = getPosByDirection(pos, direction).second;
        direction = Map::Direction::RIGHT;
    } else if (direction == Map::Direction::UP) {
        pos = getPosByDirection(pos, direction).second;
        direction = Map::Direction::DOWN;
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
