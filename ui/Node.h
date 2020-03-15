//
// Created by liu on 2020/3/11.
//

#ifndef MAPF_NODE_H
#define MAPF_NODE_H

#include "GraphWidget.h"
#include "../solver/Solver.h"

#include <QGraphicsItem>

class Node : public QGraphicsItem {
private:
    GraphWidget *graph;
    bool blocked = false;
    int radius;
    std::pair<size_t, size_t> pos;
    QVector<Edge *> edges;

    bool onPath = false;
    bool latest = false;
    bool occupied = false;
    bool chosen = false;
    

public:
    Node(GraphWidget *graphWidget, std::pair<size_t, size_t> pos, bool blocked = false, int radius = 15);

    void addEdge(Edge *edge, int direction);

    Edge *getEdge(int direction) { return edges[direction]; };

    auto getPos() { return this->pos; };

    auto isBlocked() { return blocked; };

    void setOnPath(bool flag);

    void setLatest(bool flag);

    void setOccupied(bool flag);

    void setChosen(bool flag);

    void mousePressEdgeEvent(int direction);

    QString toString();

protected:
    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

};


#endif //MAPF_NODE_H
