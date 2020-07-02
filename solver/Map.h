//
// Created by liu on 2020/3/7.
//

#ifndef MAPF_MAP_H
#define MAPF_MAP_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>

class Map {
public:
    enum class Direction {
        UP, RIGHT, DOWN, LEFT, NONE
    };

    static const Direction directions[4];
    static const int DIRECTION_X[4];
    static const int DIRECTION_Y[4];

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
    size_t height = 0, width = 0;
    std::string type;
    std::vector<std::vector<char> > map;

    std::unordered_map<OccupiedKey, std::unique_ptr<std::map<size_t, size_t> >, OccupiedKeyHash, OccupiedKeyEqual> occupiedMap;


    template<typename T>
    static void parseHeader(const std::string &line, const std::string &key, T &value);

public:
    explicit Map(const std::string &filename);

    auto getHeight() const { return this->height; };

    auto getWidth() const { return this->width; };

    const std::vector<char> &operator[](size_t index) const;

    std::pair<bool, std::pair<size_t, size_t>> getPosByDirection(std::pair<size_t, size_t> pos, Direction direction) const;

    Direction getDirectionByPos(std::pair<size_t, size_t> pos1, std::pair<size_t, size_t> pos2) const;

    void addNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime);

    void removeNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime);

    void addEdgeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime);

    bool loadConstraints(const std::string &filename);

    auto &getOccupiedMap() const { return this->occupiedMap; };

    void printOccupiedMap() const;

    static size_t getDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end);
};


#endif //MAPF_MAP_H
