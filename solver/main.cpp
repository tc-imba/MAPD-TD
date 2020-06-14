#include <iostream>
#include <fstream>
#include <limits>
#include <exception>
#include <algorithm>

#include "../utils/ezOptionParser.hpp"

#include "Manager.h"
#include "Solver.h"

int main(int argc, const char *argv[]) {
    ez::ezOptionParser optionParser;

    optionParser.overview = "Multi Agent Path Finding";
    optionParser.syntax = "./MAPF [OPTIONS]";
    optionParser.example = "./MAPF";
    optionParser.footer = "";


    optionParser.add("test-benchmark", false, 1, 0, "Data Path", "-d", "--data");
    optionParser.add("task/well-formed-21-35-10-2.task", false, 1, 0, "Task", "-t", "--task");
    optionParser.add("", false, 1, 0, "Output File", "-o", "--output");

    ez::ezOptionValidator validPhi("d", "ge", "0");
    optionParser.add("1", false, 1, 0, "Phi", "--phi", &validPhi);

    ez::ezOptionValidator validAlgorithm("s1", "gele", "0,1");
    optionParser.add("0", false, 1, 0, "Algorithm", "-a", "--algorithm", &validAlgorithm);

    ez::ezOptionValidator validMaxStep("u4");
    optionParser.add("100000", false, 1, 0, "Max Step", "--max-step", &validMaxStep);

    optionParser.add("", false, 0, 0, "Use Branch and Bound", "-b", "--bound");
    optionParser.add("", false, 0, 0, "Use Sort", "-s", "--sort");
    optionParser.parse(argc, argv);

    std::string dataPath, taskFile, outputFile;
    double phi;
    int algorithmId;
    bool boundFlag, sortFlag;
    size_t maxStep;

    optionParser.get("--data")->getString(dataPath);
    optionParser.get("--task")->getString(taskFile);
    optionParser.get("--output")->getString(outputFile);
    optionParser.get("--phi")->getDouble(phi);
    optionParser.get("--algorithm")->getInt(algorithmId);
    optionParser.get("--max-step")->getULongLong(maxStep);
    boundFlag = optionParser.isSet("--bound");
    sortFlag = optionParser.isSet("--sort");

    auto coutBuf = std::cout.rdbuf();
    std::ofstream fout;
    if (!outputFile.empty()) {
        fout.open(outputFile);
        std::cout.rdbuf(fout.rdbuf());
    }

    Manager manager(dataPath, maxStep, boundFlag, sortFlag, true);
    auto map = manager.loadTaskFile(taskFile);
    manager.leastFlexFirstAssign(map, algorithmId, phi);

    if (!outputFile.empty()) {
        std::cout.rdbuf(coutBuf);
        fout.close();
    }

    return 0;
}
