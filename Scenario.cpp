//
// Created by liu on 2020/3/7.
//

#include "Scenario.h"

#include <utility>

Scenario::Scenario(
        size_t bucket, Map *map, std::pair<size_t, size_t> start, std::pair<size_t, size_t> end, double optimal
) : bucket(bucket), map(map), start(std::move(start)), end(std::move(end)), optimal(optimal) {

}
