#pragma once
#include <QPainter>
#include "Shape.hpp"

/* 简单直线连接器（起点、终点分别指向图形） */
class Connector
{
public:
    Shape* src = nullptr;   // 起点图形
    Shape* dst = nullptr;   // 终点图形
    QPointF tempEnd;        // 绘制过程中的临时终点

    void paint(QPainter& p) const;

private:
    QPointF anchorPoint(const Shape* s, const QPointF& ref) const;
    void    drawArrow(QPainter& p, const QPointF& from, const QPointF& to) const;
};
