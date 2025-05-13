#include "Connector.hpp"
#include <QtMath>

QPointF Connector::anchorPoint(const Shape* s, const QPointF& ref) const
{
    return s->getConnectionPoint(ref);
}

void Connector::drawArrow(QPainter& p, const QPointF& from, const QPointF& to) const
{
    QLineF line(from, to);
    constexpr double arrowSize = 14;
    double angle = std::atan2(-line.dy(), line.dx());

    QPointF p1 = to + QPointF(std::sin(angle + M_PI / 3.0) * arrowSize,
        std::cos(angle + M_PI / 3.0) * arrowSize);
    QPointF p2 = to + QPointF(std::sin(angle - M_PI / 3.0) * arrowSize,
        std::cos(angle - M_PI / 3.0) * arrowSize);
        
    QPointF tailPoint = to + QPointF(std::sin(angle + M_PI) * arrowSize * 0.5,
        std::cos(angle + M_PI) * arrowSize * 0.5);

    QPolygonF head;
    head << to << p1 << tailPoint << p2;
    
    QPen originalPen = p.pen();
    QBrush originalBrush = p.brush();
    
    p.setBrush(color);
    
    QPen arrowPen(color.darker(120), width + 0.5);
    p.setPen(arrowPen);
    
    p.drawPolygon(head);
    
    p.setPen(originalPen);
    p.setBrush(originalBrush);
}

void Connector::paint(QPainter& p) const
{
    if (!src) return;

    // 获取初始参考点
    QPointF dstPoint = dst ? dst->bounds.center() : tempEnd;
    QPointF srcPoint = src->bounds.center();
    
    // 迭代计算，使起点和终点互相影响
    // 第一次计算：起点基于目标中心点
    QPointF p1 = anchorPoint(src, dstPoint);
    
    // 第二次计算：终点基于新的起点
    QPointF p2 = dst ? anchorPoint(dst, p1) : tempEnd;
    
    // 第三次计算：起点基于新的终点（再次调整）
    p1 = anchorPoint(src, p2);

    p.setPen(QPen(color, width));
    p.drawLine(p1, p2);
    
    // 绘制终点箭头
    drawArrow(p, p1, p2);
    
    // 如果是双向箭头，则在起点也绘制箭头
    if (bidirectional && dst) {
        drawArrow(p, p2, p1);
    }
}
