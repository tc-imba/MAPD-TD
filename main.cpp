#include "Manager.h"
#include "Solver.h"

int main() {
    Manager manager("test-benchmark");
    manager.loadScenarioFile("test/test.scen");

    auto scenario = manager.getScenario();
    Solver solver(scenario->getMap());

    solver.addNodeOccupied({0,1},0, 3);
    solver.addNodeOccupied({0,2},4, 5);

    solver.solve(scenario);

    return 0;
}
