//
// Created by liu on 2020/3/15.
//

#include "Constraints.h"

Constraints::Constraints(const Map *map) : map(map) {

}

void Constraints::addNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime) {
    addEdgeOccupied(pos, Map::Direction::NONE, startTime, endTime);
}

void Constraints::addEdgeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime) {
    if (direction == Map::Direction::LEFT) {
        pos = map->getPosByDirection(pos, direction).second;
        direction = Map::Direction::RIGHT;
    } else if (direction == Map::Direction::UP) {
        pos = map->getPosByDirection(pos, direction).second;
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
