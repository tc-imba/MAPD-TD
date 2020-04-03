//
// Created by liu on 2020/3/11.
//

#ifndef MAPF_GRAPHWIDGET_H
#define MAPF_GRAPHWIDGET_H

#include "../solver/Solver.h"
#include "../solver/Manager.h"

#include <QGraphicsView>
#include <QVector>
#include <QGraphicsProxyWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>

class Edge;

class Node;

class GraphWidget : public QGraphicsView {
Q_OBJECT
private:
    QGraphicsScene *scene;
    QVector<QVector<Node *> > nodes;
    QVector<Edge *> edges;

    QListWidget *occupiedListWidget;
    QGraphicsProxyWidget *occupiedListWidgetProxy;
    QLabel *occupiedListLabel;
    QGraphicsProxyWidget *occupiedListLabelProxy;

    QListWidget *openListWidget;
    QGraphicsProxyWidget *openListWidgetProxy;
    QLabel *openListLabel;
    QGraphicsProxyWidget *openListLabelProxy;

    QListWidget *closedListWidget;
    QGraphicsProxyWidget *closedListWidgetProxy;
    QLabel *closedListLabel;
    QGraphicsProxyWidget *closedListLabelProxy;

    QPushButton *stepButton;
    QGraphicsProxyWidget *stepButtonProxy;
    QPushButton *agentButton;
    QGraphicsProxyWidget *agentButtonProxy;

    QLabel *label;
    QGraphicsProxyWidget *labelProxy;

    Solver *solver;
    Manager *manager;
    QVector<std::pair<std::pair<size_t, size_t>, int>> savedPath;
    Solver::VirtualNode latestVNode;
    Node *selectedNode = nullptr;
    Edge *selectedEdge = nullptr;

    size_t timestamp = 0;
    size_t step = 0;
    size_t agentNum = 0;

    void reset();

    void clearSelected();

    void updateOccupiedList(std::map<size_t, size_t> *occupied, bool all = false);

    void addToOpenClosedList(const Solver::VirtualNode *vNode);

    void updateOpenClosedList(const std::multimap<size_t, Solver::VirtualNode *> &open,
                              const std::multimap<size_t, Solver::VirtualNode *> &closed);

    void updateOpenClosedList(const std::set<Solver::VirtualNode *, Solver::VirtualNodeSameNodeComp> &vNodes);

    void updateLists();

    void keyPressEvent(QKeyEvent *event) override;

public:
    GraphWidget(Manager *manager, QWidget *parent = nullptr);

    void setSolver(Solver *solver);

    void mousePressNode(std::pair<size_t, size_t> pos);

    void mousePressEdge(std::pair<size_t, size_t> pos, int direction);

    void updateLabel();

    void setTimeStamp(size_t timestamp);

    void initScenario(Scenario *scenario);

private slots:

    void handleStepButton();

    void handleAgentButton();

};


#endif //MAPF_GRAPHWIDGET_H
