//
// Created by liu on 2020/3/7.
//

#ifndef MAPF_SCENARIO_H
#define MAPF_SCENARIO_H

#include "Map.h"

#include <utility>

class Scenario {
private:
    size_t bucket;
    Map *map;
    std::pair<size_t, size_t> start, end;
    double optimal;
public:
    Scenario(size_t bucket, Map *map, std::pair<size_t, size_t> start, std::pair<size_t, size_t> end, double optimal);

    auto getBucket() const { return this->bucket; };

    auto getMap() const { return this->map; };

    auto getStart() const { return this->start; };

    auto getEnd() const { return this->end; };

    auto getOptimal() const { return this->optimal; };
};


#endif //MAPF_SCENARIO_H
