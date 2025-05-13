#include "Connector.hpp"
#include <QtMath>

QPointF Connector::anchorPoint(const Shape* s, const QPointF& ref) const
{
    return s->getConnectionPoint(ref);
}

void Connector::drawArrow(QPainter& p, const QPointF& from, const QPointF& to) const
{
    QLineF line(from, to);
    constexpr double arrowSize = 12;
    double angle = std::atan2(-line.dy(), line.dx());

    QPointF p1 = to + QPointF(std::sin(angle + M_PI / 2.5) * arrowSize,
        std::cos(angle + M_PI / 2.5) * arrowSize);
    QPointF p2 = to + QPointF(std::sin(angle - M_PI / 2.5) * arrowSize,
        std::cos(angle - M_PI / 2.5) * arrowSize);

    QPolygonF head;
    head << to << p1 << p2;
    
    QPen originalPen = p.pen();
    p.setBrush(color);
    
    QPen arrowPen(color.darker(120), width + 0.5);
    p.setPen(arrowPen);
    
    p.drawPolygon(head);
    
    p.setPen(originalPen);
}

void Connector::paint(QPainter& p) const
{
    if (!src) return;

    QPointF p1 = anchorPoint(src, tempEnd);
    QPointF p2 = dst ? anchorPoint(dst, p1) : tempEnd;

    p.setPen(QPen(color, width));
    p.drawLine(p1, p2);
    drawArrow(p, p1, p2);
}
