//
// Created by liu on 18/10/2020.
//

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <limits>

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

int main() {
//    int height = 33, width = 46;
    int height = 21, width = 35;
    std::string dataPath = "test-benchmark";
    std::string mapName = "well-formed-" + std::to_string(height) + "-" + std::to_string(width);

    std::ifstream fin(dataPath + "/map/" + mapName + ".map");
    std::ofstream fout(dataPath + "/map/" + mapName + ".distance");

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
            if (map[i][j] == '.') {
                for (auto direction : directions) {
                    auto p = getPosByDirection({i, j}, direction, height, width);
                    if (p.first && map[p.second.first][p.second.second] == '.') {
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