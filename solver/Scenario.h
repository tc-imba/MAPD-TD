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

    void initDistances();

public:
    Scenario(size_t bucket, Map *map, std::pair<size_t, size_t> start, std::pair<size_t, size_t> end, double optimal);

    Scenario(size_t bucket, Map *map, std::vector<std::pair<size_t, size_t> > positions, double optimal);

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
};


#endif //MAPF_SCENARIO_H
