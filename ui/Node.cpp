//
// Created by liu on 2020/3/11.
//

#include <iostream>
#include "Node.h"

Node::Node(GraphWidget *graphWidget, std::pair<size_t, size_t> pos, bool blocked, int radius)
        : graph(graphWidget), pos(std::move(pos)), blocked(blocked), radius(radius) {
    edges.resize(4);
    for (int i = 0; i < 4; i++) edges[i] = nullptr;
}


QRectF Node::boundingRect() const {
    qreal adjust = 2;
    return QRectF(-radius - adjust, -radius - adjust, 2 * radius + adjust, 2 * radius + adjust);
//    return QRectF();
}

void Node::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget) {
//    painter->setPen(Qt::NoPen);
//    painter->setBrush(Qt::darkGray);
//    painter->drawEllipse(-7, -7, 20, 20);

/*    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, QColor(Qt::yellow).lighter(120));
        gradient.setColorAt(0, QColor(Qt::darkYellow).lighter(120));
    } else {
        gradient.setColorAt(0, Qt::yellow);
        gradient.setColorAt(1, Qt::darkYellow);
    }
    painter->setBrush(gradient);*/

//    std::cerr << "Paint (" << pos.first << ", " << pos.second << ")" << std::endl;

//    if (blocked) return;
    if (blocked) {
        painter->setPen(QPen(Qt::lightGray, 0));
        painter->setBrush(QBrush(Qt::lightGray));
        painter->drawRect(-radius / 2, -radius / 2, radius, radius);
        return;
    }

    if (chosen) {
        painter->setPen(QPen(Qt::green, 3));
    } else if (latest) {
        painter->setPen(QPen(Qt::blue, 3));
    } else {
        painter->setPen(QPen(Qt::black, 3));
    }

    if (occupied) {
        painter->setBrush(QBrush(Qt::red));
    }

    painter->drawEllipse(-radius, -radius, radius * 2, radius * 2);


    if (onPath) {
        painter->setPen(QPen(Qt::blue, 3));
        painter->setBrush(QBrush(Qt::blue));
        painter->drawEllipse(-3, -3, 6, 6);
    }
}

void Node::addEdge(Edge *edge, int direction) {
    assert(direction >= 0 && direction < 4);
    edges[direction] = edge;
}

void Node::setOnPath(bool flag) {
    if (onPath == flag)
        return;
    onPath = flag;
//    update();
}

void Node::setLatest(bool flag) {
    if (latest == flag)
        return;
    latest = flag;
//    update();
}

void Node::setOccupied(bool flag) {
    if (occupied == flag)
        return;
    occupied = flag;
//    update();
}

void Node::setChosen(bool flag) {
    if (chosen == flag)
        return;
    chosen = flag;
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent *event) {
    graph->mousePressNode(pos);
    QGraphicsItem::mousePressEvent(event);
}

void Node::mousePressEdgeEvent(int direction) {
    graph->mousePressEdge(pos, direction);
}

QString Node::toString() {
    return "(" + QString::number(pos.first) + ", " + QString::number(pos.second) + ")";
}


