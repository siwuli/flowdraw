#pragma once
#include <QPainter>
#include "Shape.hpp"

/* 用直线连接两个端点、起点直指向图形： */
class Connector
{
public:
    Shape* src = nullptr;   // 起点图形
    Shape* dst = nullptr;   // 终点图形
    QPointF tempEnd;        // 绘制过程中的临时终点
    
    // 添加颜色和宽度属性
    QColor color = Qt::black;   // 连接线颜色
    qreal width = 1.5;          // 连接线宽度

    void paint(QPainter& p) const;

private:
    QPointF anchorPoint(const Shape* s, const QPointF& ref) const;
    void    drawArrow(QPainter& p, const QPointF& from, const QPointF& to) const;
};
