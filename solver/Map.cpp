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
