//
// Created by liu on 2020/3/11.
//

#ifndef MAPF_EDGE_H
#define MAPF_EDGE_H

#include "GraphWidget.h"

#include <QGraphicsItem>

class Edge : public QGraphicsItem {
private:
    Node *source, *dest;
    int direction, nodeRadius;
    QPointF sourcePoint, destPoint;

    bool onPath = false;
    bool occupied = false;
    bool chosen = false;

    void adjust();

public:
    Edge(Node *sourceNode, Node *destNode, int direction, int nodeRadius = 15);

    auto getDirection() { return direction; };

    Node *sourceNode() const { return source; };

    Node *destNode() const { return dest; };

    void setOnPath(bool flag);

    void setOccupied(bool flag);

    void setChosen(bool flag);

    QString toString();
protected:
    QRectF boundingRect() const override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;

    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};


#endif //MAPF_EDGE_H
