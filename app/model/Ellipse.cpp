#include "Ellipse.hpp"
#include <QtMath>

void Ellipse::paint(QPainter& p, bool selected) const
{
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    p.drawEllipse(bounds);

    if (selected) {
        QPen pen(Qt::DashLine); pen.setColor(Qt::blue);
        p.setPen(pen); p.setBrush(Qt::NoBrush);
        p.drawEllipse(bounds.adjusted(-2, -2, 2, 2));
    }
}

bool Ellipse::hitTest(const QPointF& pt) const
{
    QPointF c = bounds.center();
    double rx = bounds.width() / 2.0;
    double ry = bounds.height() / 2.0;
    double dx = (pt.x() - c.x()) / rx;
    double dy = (pt.y() - c.y()) / ry;
    return dx * dx + dy * dy <= 1.0;
}

QJsonObject Ellipse::toJson() const
{
    return QJsonObject{
        {"type","rect"},
        {"x",bounds.x()}, {"y",bounds.y()},
        {"w",bounds.width()}, {"h",bounds.height()},
        {"fill",   fillColor.name(QColor::HexArgb)},      // ÑÕÉ«´ø alpha
        {"stroke", strokeColor.name(QColor::HexArgb)},
        {"width",  strokeWidth}
    };
}

void Ellipse::fromJson(const QJsonObject& o)
{
    bounds = { o["x"].toDouble(), o["y"].toDouble(),
               o["w"].toDouble(), o["h"].toDouble() };
    fillColor = QColor(o["fill"].toString("#ffffffff"));
    strokeColor = QColor(o["stroke"].toString("#ff000000"));
    strokeWidth = o["width"].toDouble(1.5);
}
