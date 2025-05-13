#include "Triangle.hpp"
#include <QPainterPath>

void Triangle::paint(QPainter& p, bool selected) const
{
    // 创建三角形路径
    QPainterPath path;
    QRectF rect = bounds;
    
    // 三角形的三个点：顶部中心点、左下角、右下角
    QPointF top(rect.center().x(), rect.top());
    QPointF bottomLeft(rect.left(), rect.bottom());
    QPointF bottomRight(rect.right(), rect.bottom());
    
    path.moveTo(top);
    path.lineTo(bottomRight);
    path.lineTo(bottomLeft);
    path.closeSubpath();
    
    // 绘制三角形
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    p.drawPath(path);
    
    // 绘制文本
    drawText(p);

    // 如果被选中，绘制虚线框
    if (selected) {
        QPen dashPen(Qt::DashLine);
        dashPen.setColor(Qt::blue);
        p.setPen(dashPen);
        p.setBrush(Qt::NoBrush);
        p.drawRect(bounds.adjusted(-2, -2, 2, 2));
    }
}

bool Triangle::hitTest(const QPointF& pt) const
{
    // 创建三角形路径用于命中测试
    QPainterPath path;
    QRectF rect = bounds;
    
    QPointF top(rect.center().x(), rect.top());
    QPointF bottomLeft(rect.left(), rect.bottom());
    QPointF bottomRight(rect.right(), rect.bottom());
    
    path.moveTo(top);
    path.lineTo(bottomRight);
    path.lineTo(bottomLeft);
    path.closeSubpath();
    
    return path.contains(pt);
}

QJsonObject Triangle::toJson() const
{
    return QJsonObject{
        {"type", "triangle"},
        {"x", bounds.x()}, {"y", bounds.y()},
        {"w", bounds.width()}, {"h", bounds.height()},
        {"fill", fillColor.name(QColor::HexArgb)},
        {"stroke", strokeColor.name(QColor::HexArgb)},
        {"width", strokeWidth},
        {"text", text},
        {"textColor", textColor.name(QColor::HexArgb)},
        {"textSize", textSize}
    };
}

void Triangle::fromJson(const QJsonObject& o)
{
    bounds = { o["x"].toDouble(), o["y"].toDouble(),
               o["w"].toDouble(), o["h"].toDouble() };
    fillColor = QColor(o["fill"].toString("#ffffffff"));
    strokeColor = QColor(o["stroke"].toString("#ff000000"));
    strokeWidth = o["width"].toDouble(1.5);
    text = o["text"].toString();
    textColor = QColor(o["textColor"].toString("#ff000000"));
    textSize = o["textSize"].toInt(10);
} 