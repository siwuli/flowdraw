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
    return QJsonObject{
        {"type","rect"},
        {"x",bounds.x()}, {"y",bounds.y()},
        {"w",bounds.width()}, {"h",bounds.height()},
        {"fill",   fillColor.name(QColor::HexArgb)},      // 颜色带 alpha
        {"stroke", strokeColor.name(QColor::HexArgb)},
        {"width",  strokeWidth}
    };
}

void Rect::fromJson(const QJsonObject& o)
{
    bounds = { o["x"].toDouble(), o["y"].toDouble(),
               o["w"].toDouble(), o["h"].toDouble() };
    fillColor = QColor(o["fill"].toString("#ffffffff"));
    strokeColor = QColor(o["stroke"].toString("#ff000000"));
    strokeWidth = o["width"].toDouble(1.5);
}

