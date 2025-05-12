#include "Rect.hpp"

void Rect::paint(QPainter& p, bool selected) const
{
    // 填充
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    p.drawRect(bounds);

    // 如果被选中，加虚线高亮
    if (selected) {
        QPen dashPen(Qt::DashLine);
        dashPen.setColor(Qt::blue);
        p.setPen(dashPen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(bounds.adjusted(-2, -2, 2, 2));
    }
}

bool Rect::hitTest(const QPointF& pt) const
{
    return bounds.contains(pt);
}

QJsonObject Rect::toJson() const
{
    QJsonObject obj;
    obj["type"] = "rect";
    obj["x"] = bounds.x();
    obj["y"] = bounds.y();
    obj["w"] = bounds.width();
    obj["h"] = bounds.height();
    return obj;
}

void Rect::fromJson(const QJsonObject& obj)
{
    bounds = { obj["x"].toDouble(),
              obj["y"].toDouble(),
              obj["w"].toDouble(),
              obj["h"].toDouble() };
}
