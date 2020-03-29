//
// Created by liu on 2020/3/7.
//

#ifndef MAPF_MANAGER_H
#define MAPF_MANAGER_H

#include "Map.h"
#include "Scenario.h"

#include <unordered_map>
#include <memory>

class Manager {
private:
    std::string dataPath;
    std::unordered_map<std::string, std::unique_ptr<Map> > maps;
    std::vector<std::unique_ptr<Scenario> > scenarios;

    Map *loadMapFile(const std::string &filename);

public:
    explicit Manager(std::string dataPath);

    Map *getMap(const std::string &mapName);

    void loadScenarioFile(const std::string &filename);

    Scenario* getScenario(size_t index = 0);
};


#endif //MAPF_MANAGER_H
