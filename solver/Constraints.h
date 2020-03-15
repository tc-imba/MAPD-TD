//
// Created by liu on 2020/3/15.
//

#ifndef MAPF_CONSTRAINTS_H
#define MAPF_CONSTRAINTS_H

#include "Map.h"

#include <memory>
#include <map>
#include <unordered_map>

class Constraints {
public:
    struct OccupiedKey {
        std::pair<size_t, size_t> pos;
        Map::Direction direction;
    };

    struct OccupiedKeyHash {
        size_t operator()(OccupiedKey const &occupiedKey) const noexcept {
            auto hash = std::hash<std::size_t>{};
            return hash(occupiedKey.pos.first) ^ (hash(occupiedKey.pos.second) << 1) ^
                   (hash(size_t(occupiedKey.direction)) << 2);
        }
    };

    struct OccupiedKeyEqual {
        constexpr bool operator()(const OccupiedKey &a, const OccupiedKey &b) const {
            return a.pos == b.pos && a.direction == b.direction;
        }
    };

private:
    const Map *map;
    std::unordered_map<OccupiedKey, std::unique_ptr<std::map<size_t, size_t> >, OccupiedKeyHash, OccupiedKeyEqual> occupiedMap;

public:
    explicit Constraints(const Map *map);

    void addNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime);

    void addEdgeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime);

    auto &getOccupiedMap() { return occupiedMap; };
};


#endif //MAPF_CONSTRAINTS_H
