//
// Created by liu on 2020/3/11.
//

#include <iostream>
#include <QtWidgets/QDialog>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QGraphicsProxyWidget>
#include "GraphWidget.h"
#include "Node.h"
#include "Edge.h"

static const int DIRECTION_X[] = {-1, 0, 1, 0};
static const int DIRECTION_Y[] = {0, 1, 0, -1};

GraphWidget::GraphWidget(QWidget *parent) : QGraphicsView(parent) {
    scene = new QGraphicsScene(this);
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    scene->setSceneRect(-200, -200, 400, 400);
    setScene(scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
//    scale(qreal(0.5), qreal(0.5));
    setMinimumSize(1000, 1000);
    setWindowTitle(tr("MAPF"));

    label = new QLabel();
    labelProxy = scene->addWidget(label);
    label->setAttribute(Qt::WA_TranslucentBackground);
    auto font = label->font();
    font.setPointSize(12);
    label->setFont(font);

    occupiedListWidget = new QListWidget();
    occupiedListWidget->setFixedWidth(150);
    occupiedListWidget->setFixedHeight(450);
    occupiedListWidgetProxy = scene->addWidget(occupiedListWidget);
    occupiedListWidgetProxy->setVisible(false);

    occupiedListLabel = new QLabel();
    occupiedListLabelProxy = scene->addWidget(occupiedListLabel);
    occupiedListLabel->setAttribute(Qt::WA_TranslucentBackground);
    occupiedListLabel->setFixedWidth(150);

    openListWidget = new QListWidget();
    openListWidget->setFixedWidth(300);
    openListWidget->setFixedHeight(200);
    openListWidgetProxy = scene->addWidget(openListWidget);
    openListWidgetProxy->setVisible(false);

    openListLabel = new QLabel();
    openListLabelProxy = scene->addWidget(openListLabel);
    openListLabel->setAttribute(Qt::WA_TranslucentBackground);
    openListLabel->setFixedWidth(300);

    closedListWidget = new QListWidget();
    closedListWidget->setFixedWidth(300);
    closedListWidget->setFixedHeight(200);
    closedListWidgetProxy = scene->addWidget(closedListWidget);
    closedListWidgetProxy->setVisible(false);

    closedListLabel = new QLabel();
    closedListLabelProxy = scene->addWidget(closedListLabel);
    closedListLabel->setAttribute(Qt::WA_TranslucentBackground);
    closedListLabel->setFixedWidth(300);

    stepButton = new QPushButton("Next Step");
    stepButtonProxy = scene->addWidget(stepButton);
    stepButtonProxy->setVisible(false);
    connect(stepButton, SIGNAL (released()), this, SLOT (handleStepButton()));
}

void GraphWidget::setSolver(Solver *solver) {
    reset();

    this->solver = solver;
    auto map = solver->getMap();
    auto height = map->getHeight(), width = map->getWidth();
    auto nodeSize = 100.;
    auto startX = nodeSize * (-(width - 1.) / 2) - 200;
    auto startY = nodeSize * (-(height - 1.) / 2);

    nodes.resize(height);
    for (int i = 0; i < height; i++) {
        nodes[i].resize(width);
        for (int j = 0; j < width; j++) {
            auto node = new Node(this, {i, j}, (*map)[i][j] != '.');
            scene->addItem(node);
            auto x = startX + nodeSize * j;
            auto y = startY + nodeSize * i;
            node->setPos(x, y);
            nodes[i][j] = node;
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            for (int k = 1; k < 3; k++) {
                int x = i + DIRECTION_X[k];
                int y = j + DIRECTION_Y[k];
                if (x >= 0 && x < height && y >= 0 && y < width) {
                    auto edge = new Edge(nodes[i][j], nodes[x][y], k);
                    scene->addItem(edge);
                    edges.push_back(edge);
                }
            }
        }
    }

    startX += nodeSize * width;
    startY += nodeSize * height / 2;
    occupiedListWidgetProxy->setVisible(true);
    occupiedListWidgetProxy->setPos(startX, startY - 200);
    occupiedListLabelProxy->setPos(startX, startY - 230);

    openListWidgetProxy->setVisible(true);
    openListWidgetProxy->setPos(startX + 180, startY - 200);
    openListLabelProxy->setPos(startX + 180, startY - 230);

    closedListWidgetProxy->setVisible(true);
    closedListWidgetProxy->setPos(startX + 180, startY + 50);
    closedListLabelProxy->setPos(startX + 180, startY + 20);

    stepButtonProxy->setVisible(true);
    stepButtonProxy->setPos(startX, startY - 300);

    labelProxy->setPos(startX, startY - 450);
    setTimeStamp(0);
    updateLabel();
    updateLists();
}

void GraphWidget::reset() {
    for (auto &row : nodes) {
        for (auto node : row) {
            if (node) {
                scene->removeItem(node);
                delete node;
            }
        }
    }
    for (auto edge: edges) {
        scene->removeItem(edge);
        delete edge;
    }
    timestamp = -1;
    step = 0;
    selectedNode = nullptr;
    selectedEdge = nullptr;
}

void GraphWidget::clearSelected() {
    if (selectedNode) {
        selectedNode->setChosen(false);
    }
    if (selectedEdge) {
        selectedEdge->setChosen(false);
    }
    selectedNode = nullptr;
    selectedEdge = nullptr;
}

void GraphWidget::updateOccupiedList(std::map<size_t, size_t> *occupied, bool all) {
    occupiedListWidget->clear();
    if (all) {
        for (const auto &it : solver->getMap()->getOccupiedMap()) {
            QString header;
            auto node = nodes[it.first.pos.first][it.first.pos.second];
            if (it.first.direction == Map::Direction::NONE) {
                if (node->isBlocked()) {
                    continue;
                }
                header = node->toString();
            } else {
                auto edge = node->getEdge((int) it.first.direction);
                if (edge == nullptr || edge->sourceNode()->isBlocked() || edge->destNode()->isBlocked()) {
                    continue;
                }
                header = edge->toString();
            }
            for (const auto &p : *it.second) {
                QString text = "[" + QString::number(p.first) + ", " + QString::number(p.first + p.second) + ")";
                auto item = new QListWidgetItem(header + " " + text, occupiedListWidget);
                if (p.first <= timestamp && p.second > timestamp) {
                    item->setForeground(Qt::red);
                }
            }
        }
    } else if (occupied) {
        for (const auto &p : *occupied) {
            QString text = "[" + QString::number(p.first) + ", " + QString::number(p.first + p.second) + ")";
            auto item = new QListWidgetItem(text, occupiedListWidget);
            if (p.first <= timestamp && p.second > timestamp) {
                item->setForeground(Qt::red);
            }
        }
    } else {
        new QListWidgetItem("<null>", occupiedListWidget);
    }
}


void GraphWidget::addToOpenClosedList(const Solver::VirtualNode *vNode) {
    QString text = nodes[vNode->pos.first][vNode->pos.second]->toString();
    if (vNode->parent) {
        text += " -> " + nodes[vNode->parent->pos.first][vNode->parent->pos.second]->toString();
    } else {
        text += " -> <null>";
    }
    text += "   " + QString::number(vNode->leaveTime) + ", " + QString::number(vNode->estimateTime);
    QListWidgetItem *item;
    if (vNode->isOpen) {
        item = new QListWidgetItem(text, openListWidget);
    } else {
        item = new QListWidgetItem(text, closedListWidget);
    }
}

void GraphWidget::updateOpenClosedList(const std::multimap<size_t, Solver::VirtualNode *> &open,
                                       const std::multimap<size_t, Solver::VirtualNode *> &closed) {
    openListWidget->clear();
    closedListWidget->clear();
    for (auto &p : open) {
        addToOpenClosedList(p.second);
    }
    for (auto &p : closed) {
        addToOpenClosedList(p.second);
    }
    if (openListWidget->children().count() == 0) {
        new QListWidgetItem("<null>", openListWidget);
    }
    if (closedListWidget->children().count() == 0) {
        new QListWidgetItem("<null>", closedListWidget);
    }
}

void GraphWidget::updateOpenClosedList(const std::set<Solver::VirtualNode *, Solver::VirtualNodeSameNodeComp> &vNodes) {
    openListWidget->clear();
    closedListWidget->clear();
    for (auto &vNode : vNodes) {
        addToOpenClosedList(vNode);
    }
    if (openListWidget->children().count() == 0) {
        new QListWidgetItem("<null>", openListWidget);
    }
    if (closedListWidget->children().count() == 0) {
        new QListWidgetItem("<null>", closedListWidget);
    }
}

void GraphWidget::updateLists() {
    if (selectedNode) {
        auto pos = selectedNode->getPos();
        auto &node = solver->getNodes()[pos.first][pos.second];
        updateOccupiedList(node.occupied);
        updateOpenClosedList(node.virtualNodes);
        occupiedListLabel->setText("Ov " + selectedNode->toString());
        openListLabel->setText("Node Open List " + selectedNode->toString());
        closedListLabel->setText("Node Closed List " + selectedNode->toString());
    } else if (selectedEdge) {
        auto pos = selectedEdge->sourceNode()->getPos();
        auto &node = solver->getNodes()[pos.first][pos.second];
        updateOccupiedList(node.edges[selectedEdge->getDirection()].occupied);
        updateOpenClosedList(solver->getOpen(), solver->getClosed());
        occupiedListLabel->setText("Oe " + selectedEdge->toString());
        openListLabel->setText("Global Open List");
        closedListLabel->setText("Global Closed List");
    } else {
        updateOccupiedList(nullptr, true);
        updateOpenClosedList(solver->getOpen(), solver->getClosed());
        occupiedListLabel->setText("");
        openListLabel->setText("Global Open List");
        closedListLabel->setText("Global Closed List");
    }
}

void GraphWidget::keyPressEvent(QKeyEvent *event) {
    switch (event->key()) {
        case Qt::Key_Escape:
            clearSelected();
            updateLists();
            scene->update();
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            handleStepButton();
            break;
        default:
            QGraphicsView::keyPressEvent(event);
    }
}

void GraphWidget::mousePressNode(std::pair<size_t, size_t> pos) {
    clearSelected();
    selectedNode = nodes[pos.first][pos.second];
    selectedNode->setChosen(true);
    updateLists();
    scene->update();
    std::cerr << "Node " << selectedNode->toString().toStdString() << " pressed" << std::endl;
}

void GraphWidget::mousePressEdge(std::pair<size_t, size_t> pos, int direction) {
    clearSelected();
    selectedEdge = nodes[pos.first][pos.second]->getEdge(direction);
    selectedEdge->setChosen(true);
    updateLists();
    scene->update();
    std::cerr << "Edge " << selectedEdge->toString().toStdString() << " pressed" << std::endl;
}

void GraphWidget::updateLabel() {
    QString string;
    string += "Timestamp (h_v): " + QString::number(timestamp);
    string += "\nStep: " + QString::number(step);
    if (!savedPath.empty()) {
        string += "\nLast VNode: " + nodes[latestVNode.pos.first][latestVNode.pos.second]->toString()
                  + ")\nh_v + g(v) = " + QString::number(latestVNode.estimateTime);
    } else {
        string += "\n\n";
    }
    label->setText(string);
}

void GraphWidget::setTimeStamp(size_t timestamp) {
    if (this->timestamp == timestamp)
        return;

    this->timestamp = timestamp;
    auto map = solver->getMap();
    auto height = map->getHeight(), width = map->getWidth();
    auto &solverNodes = solver->getNodes();

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            auto &solverNode = solverNodes[i][j];
            nodes[i][j]->setOccupied(solver->isOccupied(solverNode.occupied, timestamp));
            for (int k = 0; k < 4; k++) {
                auto edge = nodes[i][j]->getEdge(k);
                if (edge) {
                    edge->setOccupied(solver->isOccupied(solverNode.edges[k].occupied, timestamp));
                }
            }
        }
    }

}

void GraphWidget::handleStepButton() {
    if (solver->success())
        return;
    auto vNode = solver->step();
    latestVNode = *vNode;

    step += 1;
    setTimeStamp(vNode->leaveTime);

    for (auto &p : savedPath) {
        auto node = nodes[p.first.first][p.first.second];
        node->setOnPath(false);
        node->setLatest(false);
        if (p.second >= 0) {
            node->getEdge(p.second)->setOnPath(false);
        }
//        nodes[p.first][p.second]->update();
    }
    savedPath.clear();

    auto newPath = solver->constructPath(vNode);
    if (!newPath.empty()) {
        for (int i = 0; i < newPath.size(); i++) {
            auto p1 = newPath[i]->pos;
            int direction = -1;
            if (i < newPath.size() - 1) {
                auto p2 = newPath[i + 1]->pos;
                for (int j = 0; j < 4; j++) {
                    if (int(p2.first) - int(p1.first) == DIRECTION_X[j] &&
                        int(p2.second) - int(p1.second) == DIRECTION_Y[j]) {
                        direction = j;
                        break;
                    }
                }
            }
            auto node = nodes[p1.first][p1.second];
            node->setOnPath(true);
            if (i == 0) {
                node->setLatest(true);
            }
            if (direction >= 0) {
                node->getEdge(direction)->setOnPath(true);
            }
            savedPath.push_back(std::make_pair(p1, direction));
        }
    }

    updateLabel();
    updateLists();

    scene->update();
}











