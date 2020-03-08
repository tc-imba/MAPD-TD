//
// Created by liu on 2020/3/7.
//

#include "Map.h"

#include <fstream>
#include <sstream>
#include <exception>
#include <iostream>

template<typename T>
void Map::parseHeader(const std::string &line, const std::string &key, T &value) {
    std::string temp;
    std::istringstream iss;
    iss.str(line);
    iss >> temp >> value;
    if (temp != key) {
        throw std::exception("map key error");
    }
}

Map::Map(const std::string &filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        throw std::exception("map file not found");
    }
    std::string line;

    // Get map type
    std::getline(fin, line);
    parseHeader(line, "type", this->type);
    if (this->type != "octile") {
        throw std::exception("map type error");
    }

    // Get map height
    std::getline(fin, line);
    parseHeader(line, "height", this->height);
    if (this->height == 0) {
        throw std::exception("map height error");
    }
    std::getline(fin, line);
    parseHeader(line, "width", this->width);
    if (this->width == 0) {
        throw std::exception("map width error");
    }

    // Get map
    std::getline(fin, line);
    if (line != "map") {
        throw std::exception("map format error");
    }
    this->map.resize(this->height);
    for (size_t i = 0; i < this->height; i++) {
        this->map[i].resize(this->width, '.');
        if (!std::getline(fin, line) || line.length() < this->width) {
            throw std::exception("map format error");
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

