//
// Created by liu on 18/10/2020.
//

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <limits>
#include <cassert>
#include "../utils/ezOptionParser.hpp"

enum class Direction {
    UP, RIGHT, DOWN, LEFT, NONE
};

const Direction directions[4] = {Direction::UP, Direction::RIGHT,
                                 Direction::DOWN, Direction::LEFT};

const int DIRECTION_X[4] = {-1, 0, 1, 0};
const int DIRECTION_Y[4] = {0, 1, 0, -1};

std::pair<bool, std::pair<int, int>>
getPosByDirection(std::pair<int, int> pos, Direction direction, int height, int width) {
    bool flag = true;
    pos.first += DIRECTION_X[(int) direction];
    pos.second += DIRECTION_Y[(int) direction];
    if (pos.first >= height || pos.second >= width || pos.first < 0 || pos.second < 0) flag = false;
    return std::make_pair(flag, pos);
}

int main(int argc, const char *argv[]) {
    ez::ezOptionParser optionParser;

//    optionParser.overview = "Multi Agent Path Finding Well Formed Task Generation";
//    optionParser.syntax = "./MAPF-generate-well-formed [OPTIONS]";
//    optionParser.example = "./MAPF-generate-well-formed";
//    optionParser.footer = "";

    optionParser.add("21", false, 1, 0, "Height", "-h", "--height");
    optionParser.add("35", false, 1, 0, "Width", "-w", "--width");
    optionParser.add("", false, 0, 0, "Generate d* with endpoints", "-e", "--endpoint");

    optionParser.parse(argc, argv);

    int height, width;
    bool endpoint;

    optionParser.get("--height")->getInt(height);
    optionParser.get("--width")->getInt(width);
    endpoint = optionParser.isSet("--endpoint");


//    int height = 33, width = 46;
//    int height = 21, width = 35;
    std::string dataPath = "test-benchmark-2";
    std::string mapName = "well-formed-" + std::to_string(height) + "-" + std::to_string(width) + ".map";

    std::ifstream fin(dataPath + "/map/" + mapName);
    std::ofstream fout(dataPath + "/map/" + mapName + (endpoint ? ".endpoint" : "") + ".distance");

    assert(fin.is_open());

    std::vector<std::vector<char> > map(height, std::vector<char>(width));
    std::string temp;
    std::getline(fin, temp);
    std::getline(fin, temp);
    std::getline(fin, temp);
    std::getline(fin, temp);
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            fin >> map[i][j] >> std::ws;
//            std::cout << map[i][j];
        }
//        std::cout << std::endl;
    }
    fin.close();

    std::vector<std::vector<int> > distances;

    int size = width * height;
    distances.resize(size, std::vector<int>(size, std::numeric_limits<int>::max() / 3));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            int a = i * width + j;
            distances[a][a] = 0;
            if (map[i][j] != '@' && (!endpoint || map[i][j] != 't')) {
                for (auto direction : directions) {
                    auto p = getPosByDirection({i, j}, direction, height, width);
                    if (p.first && map[p.second.first][p.second.second] != '@' &&
                        (!endpoint || map[p.second.first][p.second.second] != 't')) {
                        int b = p.second.first * width + p.second.second;
                        distances[a][b] = 1;
//                        std::cout << i << " " << j << " " << p.second.first << " " << p.second.second << std::endl;
                    }
                }
            }
        }
    }
    for (int k = 0; k < size; k++) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (distances[i][j] > distances[i][k] + distances[k][j]) {
                    distances[i][j] = distances[i][k] + distances[k][j];
                }
            }
        }
    }
    if (endpoint) {
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                int xi = i / width, yi = i % width;
                int xj = j / width, yj = j % width;
                if (map[xi][yi] == 't' && i != j) {
//                    assert(distances[i][j] >= std::numeric_limits<int>::max() / 3);
                    for (auto di : directions) {
                        auto pi = getPosByDirection({xi, yi}, di, height, width);
                        if (pi.first && map[pi.second.first][pi.second.second] == '.') {
                            int a = pi.second.first * width + pi.second.second;
                            if (map[xj][yj] == 't') {
                                for (auto dj : directions) {
                                    auto pj = getPosByDirection({xj, yj}, dj, height, width);
                                    if (pj.first && map[pj.second.first][pj.second.second] == '.') {
                                        int b = pj.second.first * width + pj.second.second;
                                        distances[j][i] = distances[i][j] = std::min(distances[i][j], distances[a][b] + 2);
                                    }
                                }
                            } else if (map[xj][yj] != '@') {
                                distances[j][i] = distances[i][j] = std::min(distances[i][j], distances[a][j] + 1);
                            }
                        }

                    }
                    if (map[xj][yj] != '@') {
                        assert(distances[i][j] < std::numeric_limits<int>::max() / 3);
                    }
//                    std::cout << xi << " " << yi << " " << xj << " " << yj << " " << distances[i][j] << std::endl;
                }
            }
        }
    }
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (distances[i][j] < std::numeric_limits<int>::max() / 3) {
//                fout << i << " " << j << " " << distances[i][j] << std::endl;
                int x1 = i / width, y1 = i % width;
                int x2 = j / width, y2 = j % width;
                fout << x1 << " " << y1 << " " << x2 << " " << y2 << " " << distances[i][j] << std::endl;
            }
        }
    }
    fout.close();

    return 0;
}