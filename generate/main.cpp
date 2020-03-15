//
// Created by liu on 2020/3/15.
//

#include <iostream>
#include <fstream>
#include <random>
#include <vector>
#include <ctime>

using namespace std;

int main() {
    size_t height = 7;
    size_t width = 7;

    auto mid = height / 2;

    vector<vector<bool> > grid(height, vector<bool>(width, true));

//    random_device r;
    mt19937 gen(time(nullptr));
    uniform_real_distribution<> realDistribution(0, 1);


    for (size_t i = 0; i < height; i++) {
        if (i == mid) {
            for (size_t j = 1; j < width - 1; j++) {
                if (realDistribution(gen) <= 0.5) {
                    grid[mid][j] = false;
                }
            }
        } else {
            for (size_t j = 0; j < width; j++) {
                if (realDistribution(gen) <= 0.3) {
                    grid[i][j] = false;
                }
            }
        }
    }

    string filename = "test-benchmark/map/test-" + to_string(height) + "-" + to_string(width) + ".map";
    ofstream fout(filename);

    fout << "type octile" << endl;
    fout << "height " << height << endl;
    fout << "width " << width << endl;
    fout << "map" << endl;

    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            if (grid[i][j]) {
                fout << ".";
            } else {
                fout << "@";
            }
        }
        fout << endl;
    }
    fout.close();

    filename = "test-benchmark/constraints/test-" + to_string(height) + "-" + to_string(width) + ".map";
    fout.open(filename);

    uniform_int_distribution<> xDistribution(1, height - 2);
    uniform_int_distribution<> yDistribution(1, width - 2);
    uniform_int_distribution<> lengthDistribution(1, 10);
    uniform_int_distribution<> directionDistribution(0, 3);
    for (size_t t = 0; t < 10; t++) {
        for (size_t k = 0; k < 5; k++) {
            auto x = xDistribution(gen);
            auto y = yDistribution(gen);
            auto length = lengthDistribution(gen);
            size_t direction = 0;
            if (realDistribution(gen) < 0.3) {
                direction = directionDistribution(gen);
            }
            fout << x << "\t" << y << "\t" << direction << "\t" << t << "\t" << length << endl;
        }
    }
    fout.close();

    return 0;
}