//
// Created by liu on 2020/3/7.
//

#include "Manager.h"

#include <fstream>
#include <sstream>
#include <iostream>

Manager::Manager(std::string dataPath) : dataPath(std::move(dataPath)) {

}

Map *Manager::loadMapFile(const std::string &mapName) {
    auto filePath = dataPath + "/map/" + mapName;
    auto map = std::make_unique<Map>(filePath);
    auto mapPtr = map.get();
    this->maps.emplace(mapName, std::move(map));
    filePath = dataPath + "/constraints/" + mapName;
    if (mapPtr->loadConstraints(filePath)) {
        std::cerr << "constraints loaded" << std::endl;
    } else {
        std::cerr << "no constraints" << std::endl;
    }
    return mapPtr;
}

void Manager::loadScenarioFile(const std::string &filename) {
    auto filePath = dataPath + "/" + filename;
    std::ifstream fin(filePath);
    if (!fin.is_open()) {
        throw std::runtime_error("scenario file not found");
    }
    std::string line, temp, version;
    std::istringstream iss;

    std::getline(fin, line);
    iss.str(line);
    iss >> temp >> version;
    if (temp != "version") {
        throw std::runtime_error("scenario version error");
    }

    while (std::getline(fin, line)) {
        if (line.length() == 0) continue;
        iss.clear();
        iss.str(line);
        std::string mapName;
        size_t bucket, width, height;
        std::pair<size_t, size_t> start, end;
        double optimal;
        iss >> bucket >> mapName >> width >> height
            >> start.first >> start.second >> end.first >> end.second >> optimal;
        auto map = this->getMap(mapName);
        if (map->getHeight() != height || map->getWidth() != width) {
            throw std::runtime_error("scenario map size error");
        }
        auto scenario = std::make_unique<Scenario>(bucket, map, start, end, optimal);
        this->scenarios.emplace_back(std::move(scenario));
    }

    std::cerr << "Scenario " << filename << " imported" << std::endl;
}

Map *Manager::getMap(const std::string &mapName) {
    auto it = this->maps.find(mapName);
    if (it != this->maps.end()) return it->second.get();
    return this->loadMapFile(mapName);
}

Scenario *Manager::getScenario(size_t index) {
    if (index >= 0 && index < this->scenarios.size()) {
        return this->scenarios[index].get();
    }
    return nullptr;
}


