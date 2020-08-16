//
// Created by liu on 2020/3/7.
//

#ifndef MAPF_SCENARIO_H
#define MAPF_SCENARIO_H

#include "Map.h"

#include <utility>
#include <vector>
#include <cassert>

class Scenario {
private:
    size_t bucket;
    Map *map;
    std::vector<std::pair<size_t, size_t> > positions;
    std::vector<size_t> distances;
    double optimal;
    size_t startTime;

    void initDistances();

public:
    Scenario(size_t bucket, Map *map, std::pair<size_t, size_t> start, std::pair<size_t, size_t> end, double optimal, size_t startTime);

    Scenario(size_t bucket, Map *map, std::vector<std::pair<size_t, size_t> > positions, double optimal, size_t startTime);

    auto getBucket() const { return this->bucket; };

    auto getMap() const { return this->map; };

    auto size() const { return positions.size() - 1; }

    auto getStart(size_t i = 0) const {
        assert(i < size());
        return this->positions[i];
    };

    auto getEnd(size_t i) const {
        assert(i < size());
        return this->positions[i + 1];
    };

    auto getEnd() const { return getEnd(size() - 1); }

    auto getDistance(size_t i = 0) const {
        assert(i < size());
        return this->distances[i + 1];
    };

    auto getOptimal() const { return this->optimal; };

    auto getStartTime() const { return this->startTime; };
};


#endif //MAPF_SCENARIO_H
