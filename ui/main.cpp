//
// Created by liu on 2020/3/11.
//

#include "GraphWidget.h"
#include "../solver/Manager.h"
#include "../solver/Solver.h"

#include <QApplication>
#include <QTime>
#include <QMainWindow>

int main(int argc, char **argv) {
    QApplication app(argc, argv);

    auto widget = new GraphWidget;

//    Manager manager("test-benchmark");
//    manager.loadScenarioFile("test/test.scen");
//
    Manager manager("MAPF-benchmark");
    manager.loadScenarioFile("scen-even/room-32-32-4-even-1.scen");

    auto scenario = manager.getScenario();
    Solver solver(scenario->getMap());

    solver.addNodeOccupied({0, 1}, 0, 3);
    solver.addNodeOccupied({0, 2}, 4, 5);
    solver.addEdgeOccupied({0, 0}, Solver::Direction::RIGHT, 0, 3);

    solver.initScenario(scenario);

    widget->setSolver(&solver);

    QMainWindow mainWindow;
    mainWindow.setCentralWidget(widget);

    mainWindow.show();
    return app.exec();
}
