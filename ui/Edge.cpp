//
// Created by liu on 2020/3/11.
//

#include <iostream>
#include "Edge.h"
#include "Node.h"

void Edge::adjust() {
    QLineF line(mapFromItem(source, 0, 0), mapFromItem(dest, 0, 0));
    int connectPath = (onPath || blocked) ? 0 : 1;
    auto offset = connectPath * (nodeRadius + 1);
    if (direction % 2 == 1) {
        sourcePoint = line.p1() + QPointF(offset, 0);
        destPoint = line.p2() - QPointF(offset, 0);
    } else {
        sourcePoint = line.p1() + QPointF(0, offset);
        destPoint = line.p2() - QPointF(0, offset);
    }
}

Edge::Edge(Node *sourceNode, Node *destNode, int direction, int nodeRadius)
        : source(sourceNode), dest(destNode), direction(direction), nodeRadius(nodeRadius) {
//    setAcceptedMouseButtons(Qt::NoButton);
    if (!source || !dest)
        return;

    blocked = source->isBlocked() || dest->isBlocked();
    source->addEdge(this, direction);
    dest->addEdge(this, (direction + 2) % 4);
    adjust();
}

QRectF Edge::boundingRect() const {
    qreal adjust = 10;
    if (direction % 2 == 1) {
        return QRectF(sourcePoint - QPointF(0, adjust), destPoint + QPointF(0, adjust));
    } else {
        return QRectF(sourcePoint - QPointF(adjust, 0), destPoint + QPointF(adjust, 0));
    }
}

void Edge::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
    if (!source || !dest)
        return;

    if (source->isBlocked() && dest->isBlocked()) {
        QLineF line(sourcePoint, destPoint);
        painter->setPen(QPen(Qt::lightGray, nodeRadius + 1));
        painter->drawLine(line);
        return;
    }
    if (blocked) {
        return;
    }

    QLineF line(sourcePoint, destPoint);
    if (chosen) {
        painter->setPen(QPen(Qt::green, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (onPath) {
        painter->setPen(QPen(Qt::blue, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else if (occupied) {
        painter->setPen(QPen(Qt::red, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    } else {
        painter->setPen(QPen(Qt::black, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    }
    painter->drawLine(line);
/*    QRectF rect;
    if (direction % 2 == 1) {
        rect = QRectF(sourcePoint - QPointF(0, 5), destPoint + QPointF(0, 5));
    } else {
        rect = QRectF(sourcePoint - QPointF(5, 0), destPoint + QPointF(5, 0));
    }
    painter->drawRect(rect);*/
}

void Edge::setOnPath(bool flag) {
    if (onPath == flag)
        return;
    onPath = flag;
    adjust();
}

void Edge::setOccupied(bool flag) {
    if (occupied == flag)
        return;
    occupied = flag;
}

void Edge::setChosen(bool flag) {
    if (chosen == flag)
        return;
    chosen = flag;
}

void Edge::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    source->mousePressEdgeEvent(direction);
    QGraphicsItem::mousePressEvent(event);
}

QString Edge::toString() {
    if (!source || !dest)
        return "";
    return "(" + QString::number(source->getPos().first) + ", " + QString::number(source->getPos().second) +
           ", " + QString::number(direction) + ")";
}


