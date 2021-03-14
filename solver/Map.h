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

#include <boost/icl/discrete_interval.hpp>
#include <boost/icl/interval_set.hpp>

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

    struct OccupiedValue {
        boost::icl::interval_set<size_t> rangeConstraints;
        size_t infiniteWaiting = 0;
        std::map<size_t, size_t> waitingAgents;
    };

private:
    size_t height = 0, width = 0;
    std::string type;
    std::vector<std::vector<char> > map;
    std::vector<std::vector<size_t> > distances;
    std::vector<std::vector<size_t> > distancesEndpoint;
    std::vector<std::pair<size_t, size_t>> parkingLocations;
    std::vector<std::vector<size_t> > extraCost;

    std::unordered_map<OccupiedKey, std::unique_ptr<OccupiedValue>, OccupiedKeyHash, OccupiedKeyEqual> occupiedMap;


    template<typename T>
    static void parseHeader(const std::string &line, const std::string &key, T &value);

    void calculateDistances();

    void readDistances(const std::string &filename, std::vector<std::vector<size_t> > &distances);

public:
    explicit Map(const std::string &filename);

    auto getHeight() const { return this->height; };

    auto getWidth() const { return this->width; };

    const std::vector<char> &operator[](size_t index) const;

    std::pair<bool, std::pair<size_t, size_t>> getPosByDirection(std::pair<size_t, size_t> pos, Direction direction) const;

    Direction getDirectionByPos(std::pair<size_t, size_t> pos1, std::pair<size_t, size_t> pos2) const;

    void addNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime);

    void removeNodeOccupied(std::pair<size_t, size_t> pos, size_t startTime, size_t endTime);

    size_t addInfiniteWaiting(std::pair<size_t, size_t> pos, size_t startTime = 0);

    size_t removeInfiniteWaiting(std::pair<size_t, size_t> pos);

    size_t getExtraCostTime(std::pair<size_t, size_t> pos);

    void addEdgeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime);

    void removeEdgeOccupied(std::pair<size_t, size_t> pos, Map::Direction direction, size_t startTime, size_t endTime);

    void addWaitingAgent(std::pair<size_t, size_t> pos, size_t startTime, size_t agent);

    void removeWaitingAgent(std::pair<size_t, size_t> pos, size_t startTime, size_t agent);

    size_t getLastWaitingAgent(std::pair<size_t, size_t> pos);

    bool loadConstraints(const std::string &filename);

    auto &getOccupiedMap() const { return this->occupiedMap; };

    static void printOccupied(std::map<size_t, size_t> *occupied);

    std::string printOccupiedMap() const;

    static size_t getDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end);

    size_t getGraphDistance(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end);

    size_t getGraphDistanceEndpoint(std::pair<size_t, size_t> start, std::pair<size_t, size_t> end);

    auto &getParkingLocations() const { return this->parkingLocations; };
};


#endif //MAPF_MAP_H
