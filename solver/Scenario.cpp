//
// Created by liu on 2020/3/7.
//

#include "Scenario.h"

#include <iostream>

void Scenario::initDistances() {
    distances.resize(size() + 1, 0);
    size_t distance = 0;
    for (size_t i = size(); i != 0; i--) {
        distance += Map::getDistance(positions[i - 1], positions[i]);
        distances[i - 1] = distance;
    }
}

Scenario::Scenario(
        size_t bucket, Map *map, std::pair<size_t, size_t> start, std::pair<size_t, size_t> end, double optimal
) : bucket(bucket), map(map), optimal(optimal), positions({start, end}) {
    initDistances();
}

Scenario::Scenario(size_t bucket, Map *map, std::vector<std::pair<size_t, size_t>> positions, double optimal)
        : bucket(bucket), map(map), optimal(optimal), positions(std::move(positions)) {
    assert(this->positions.size() >= 2);
    initDistances();
}
