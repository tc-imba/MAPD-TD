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
                                   bool boundFlag, bool sortFlag, bool multiLabelFlag, bool deadlineBoundFlag,
                                   bool recalculateFlag, bool reserveAllFlag) {
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
    if (recalculateFlag) {
        oss << "-recalc";
    }
    if (reserveAllFlag) {
        oss << "-reserve";
    }
    oss << ".txt";
    return oss.str();
}

int main(int argc, const char *argv[]) {
    ez::ezOptionParser optionParser;

    optionParser.overview = "Multi Agent Path Finding";
    optionParser.syntax = "./MAPF [OPTIONS]";
    optionParser.example = "./MAPF --flex -a 0 --phi 0 -b -s -m -db -re -d test-benchmark -t task/well-formed-21-35-10-2.task -o auto\n";
    optionParser.footer = "";

    optionParser.add("", false, 0, 0, "Display this Message.", "-h", "--help");

    optionParser.add("test-benchmark", false, 1, 0, "Data Path", "-d", "--data");
    optionParser.add("task/well-formed-21-35-10-2.task", false, 1, 0, "Task File (Relative to Data Path)", "-t", "--task");
    optionParser.add("", false, 1, 0, "Output File", "-o", "--output");
    optionParser.add("flex", false, 1, 0, "Scheduler (flex/edf)", "--scheduler");

    auto validPhi = new ez::ezOptionValidator("d", "ge", "0");
    optionParser.add("0", false, 1, 0, "Phi", "--phi", validPhi);

    auto validAlgorithm = new ez::ezOptionValidator("s1", "gele", "0,1");
    optionParser.add("0", false, 1, 0, "Algorithm (deprecated, only 0 working)", "-a", "--algorithm", validAlgorithm);

    auto validMaxStep = new ez::ezOptionValidator("u4");
    optionParser.add("100000", false, 1, 0, "Max Step", "--max-step", validMaxStep);

    auto validWindowSize = new ez::ezOptionValidator("u4", "ge", "0");
    optionParser.add("0", false, 1, 0, "Window Size (0 means no limit)", "-w", "--window", validWindowSize);

    optionParser.add("", false, 0, 0, "Use Branch and Bound", "-b", "--bound");
    optionParser.add("", false, 0, 0, "Use Sort", "-s", "--sort");
    optionParser.add("", false, 0, 0, "Use Multi Label", "-m", "--mlabel");
    optionParser.add("", false, 0, 0, "Use Deadline Bound", "-db", "--deadline-bound");
    optionParser.add("", false, 0, 0, "Recalculate After Flex", "-re", "--recalculate");
    optionParser.add("", false, 0, 0, "Reserve all", "-ra", "--reserve-all");
    optionParser.parse(argc, argv);

    if (optionParser.isSet("-h")) {
        std::string usage;
        optionParser.getUsage(usage, 80, ez::ezOptionParser::ALIGN);
        std::cout << usage;
        return 1;
    }

    std::string dataPath, taskFile, outputFile, scheduler;
    double phi;
    int algorithmId;
    bool boundFlag, sortFlag, multiLabelFlag, deadlineBoundFlag, recalculateFlag, reserveAllFlag;
    unsigned long long maxStep, windowSize;

    optionParser.get("--data")->getString(dataPath);
    optionParser.get("--task")->getString(taskFile);
    optionParser.get("--output")->getString(outputFile);
    optionParser.get("--scheduler")->getString(scheduler);
    optionParser.get("--phi")->getDouble(phi);
    optionParser.get("--algorithm")->getInt(algorithmId);
    optionParser.get("--max-step")->getULongLong(maxStep);
    optionParser.get("--window")->getULongLong(windowSize);
    boundFlag = optionParser.isSet("--bound");
    sortFlag = optionParser.isSet("--sort");
    multiLabelFlag = optionParser.isSet("--mlabel");
    deadlineBoundFlag = optionParser.isSet("--deadline-bound");
    recalculateFlag = optionParser.isSet("--recalculate");
    reserveAllFlag = optionParser.isSet("--reserve-all");

    auto coutBuf = std::cout.rdbuf();
    std::ofstream fout;
    if (!outputFile.empty()) {
        if (outputFile == "auto") {
            outputFile = generateOutputFileName(scheduler, algorithmId, boundFlag, sortFlag, multiLabelFlag,
                                                deadlineBoundFlag, recalculateFlag, reserveAllFlag);
        }
        fout.open(outputFile);
        std::cout.rdbuf(fout.rdbuf());
    }
    std::cerr << outputFile << std::endl;

    Manager manager(
            dataPath, maxStep, windowSize,
            boundFlag, sortFlag, multiLabelFlag, true,
            deadlineBoundFlag, recalculateFlag, reserveAllFlag
    );
    auto map = manager.loadTaskFile(taskFile);

    if (scheduler == "edf") {
        manager.earliestDeadlineFirstAssign(map, algorithmId, phi);
    } else if (scheduler == "flex") {
        manager.leastFlexFirstAssign(map, algorithmId, phi);
    } else {
        assert(0);
    }

    manager.printPaths();

    if (!outputFile.empty()) {
        std::cout.rdbuf(coutBuf);
        fout.close();
    }

    return 0;
}
