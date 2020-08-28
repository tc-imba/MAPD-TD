#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <exception>
#include <algorithm>

#include "../utils/ezOptionParser.hpp"

#include "Manager.h"
#include "Solver.h"

std::string generateOutputFileName(const std::string &scheduler, int algorithmId,
                                   bool boundFlag, bool sortFlag, bool multiLabelFlag, bool deadlineBoundFlag) {
    std::ostringstream oss;
    oss << scheduler << "-algo-" << algorithmId;
    if (boundFlag) {
        oss << "-bound";
        if (sortFlag) {
            oss << "-sort";
        }
    }
    if (multiLabelFlag) {
        oss << "-mlabel";
    }
    if (deadlineBoundFlag) {
        oss << "-db";
    }
    oss << ".txt";
    return oss.str();
}

int main(int argc, const char *argv[]) {
    ez::ezOptionParser optionParser;

    optionParser.overview = "Multi Agent Path Finding";
    optionParser.syntax = "./MAPF [OPTIONS]";
    optionParser.example = "./MAPF";
    optionParser.footer = "";


    optionParser.add("test-benchmark", false, 1, 0, "Data Path", "-d", "--data");
    optionParser.add("task/well-formed-21-35-10-2.task", false, 1, 0, "Task", "-t", "--task");
    optionParser.add("", false, 1, 0, "Output File", "-o", "--output");
    optionParser.add("flex", false, 1, 0, "Scheduler (flex/edf)", "--scheduler");

    auto validPhi = new ez::ezOptionValidator("d", "ge", "0");
    optionParser.add("1", false, 1, 0, "Phi", "--phi", validPhi);

    auto validAlgorithm = new ez::ezOptionValidator("s1", "gele", "0,1");
    optionParser.add("0", false, 1, 0, "Algorithm", "-a", "--algorithm", validAlgorithm);

    auto validMaxStep = new ez::ezOptionValidator("u4");
    optionParser.add("100000", false, 1, 0, "Max Step", "--max-step", validMaxStep);

    optionParser.add("", false, 0, 0, "Use Branch and Bound", "-b", "--bound");
    optionParser.add("", false, 0, 0, "Use Sort", "-s", "--sort");
    optionParser.add("", false, 0, 0, "Use Multi Label", "-m", "--mlabel");
    optionParser.add("", false, 0, 0, "Use Deadline Bound", "-db", "--deadline-bound");
    optionParser.parse(argc, argv);

    std::string dataPath, taskFile, outputFile, scheduler;
    double phi;
    int algorithmId;
    bool boundFlag, sortFlag, multiLabelFlag, deadlineBoundFlag;
    unsigned long long maxStep;

    optionParser.get("--data")->getString(dataPath);
    optionParser.get("--task")->getString(taskFile);
    optionParser.get("--output")->getString(outputFile);
    optionParser.get("--scheduler")->getString(scheduler);
    optionParser.get("--phi")->getDouble(phi);
    optionParser.get("--algorithm")->getInt(algorithmId);
    optionParser.get("--max-step")->getULongLong(maxStep);
    boundFlag = optionParser.isSet("--bound");
    sortFlag = optionParser.isSet("--sort");
    multiLabelFlag = optionParser.isSet("--mlabel");
    deadlineBoundFlag = optionParser.isSet("--deadline-bound");

    auto coutBuf = std::cout.rdbuf();
    std::ofstream fout;
    if (!outputFile.empty()) {
        if (outputFile == "auto") {
            outputFile = generateOutputFileName(scheduler, algorithmId, boundFlag, sortFlag, multiLabelFlag, deadlineBoundFlag);
        }
        fout.open(outputFile);
        std::cout.rdbuf(fout.rdbuf());
    }

    Manager manager(dataPath, maxStep, boundFlag, sortFlag, multiLabelFlag, true, deadlineBoundFlag);
    auto map = manager.loadTaskFile(taskFile);

    if (scheduler == "edf") {
        manager.earliestDeadlineFirstAssign(map, algorithmId, phi);
    } else if (scheduler == "flex") {
        manager.leastFlexFirstAssign(map, algorithmId, phi);
    } else {
        assert(0);
    }

    if (!outputFile.empty()) {
        std::cout.rdbuf(coutBuf);
        fout.close();
    }

    return 0;
}
