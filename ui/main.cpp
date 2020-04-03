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


    Manager manager("test-benchmark");
    manager.loadScenarioFile("test/test.scen");

//    Manager manager("MAPF-benchmark");
//    manager.loadScenarioFile("scen-even/room-32-32-4-even-1.scen");

    auto widget = new GraphWidget(&manager);

    auto scenario = manager.getScenario(0);
    auto map = scenario->getMap();
    Solver solver(map);

//   map->addNodeOccupied({0, 1}, 0, 3);
//   map->addNodeOccupied({0, 2}, 4, 5);
//   map->addEdgeOccupied({0, 0}, Map::Direction::RIGHT, 0, 3);

//    solver.initScenario(scenario);

    widget->setSolver(&solver);

    QMainWindow mainWindow;
    mainWindow.setCentralWidget(widget);

    mainWindow.show();
    return app.exec();
}
