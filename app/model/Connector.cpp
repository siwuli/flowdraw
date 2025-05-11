#include "Connector.hpp"
#include <QtMath>

QPointF Connector::anchorPoint(const Shape* s, const QPointF& ref) const
{
    const QRectF& b = s->bounds;
    QPointF mids[4] = {
        {(b.left() + b.right()) / 2, b.top()},
        {b.right(), (b.top() + b.bottom()) / 2},
        {(b.left() + b.right()) / 2, b.bottom()},
        {b.left(), (b.top() + b.bottom()) / 2}
    };
    QPointF best = mids[0];
    qreal bestDist = QLineF(ref, best).length();
    for (int i = 1; i < 4; ++i) {
        qreal d = QLineF(ref, mids[i]).length();
        if (d < bestDist) { best = mids[i]; bestDist = d; }
    }
    return best;
}

void Connector::drawArrow(QPainter& p, const QPointF& from, const QPointF& to) const
{
    QLineF line(from, to);
    constexpr double arrowSize = 8;
    double angle = std::atan2(-line.dy(), line.dx());

    QPointF p1 = to + QPointF(std::sin(angle + M_PI / 3) * arrowSize,
        std::cos(angle + M_PI / 3) * arrowSize);
    QPointF p2 = to + QPointF(std::sin(angle - M_PI / 3) * arrowSize,
        std::cos(angle - M_PI / 3) * arrowSize);

    QPolygonF head;
    head << to << p1 << p2;        // ÒÀ´ÎÑ¹Èë¶¥µã
    p.setBrush(Qt::black);
    p.drawPolygon(head);
}

void Connector::paint(QPainter& p) const
{
    if (!src) return;

    QPointF p1 = anchorPoint(src, tempEnd);
    QPointF p2 = dst ? anchorPoint(dst, p1) : tempEnd;

    p.setPen(QPen(Qt::black, 1.5));
    p.drawLine(p1, p2);
    drawArrow(p, p1, p2);
}
