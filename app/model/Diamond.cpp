#include "Diamond.hpp"
#include <QPainterPath>

void Diamond::paint(QPainter& p, bool selected) const
{
    // 创建菱形路径
    QPainterPath path;
    QRectF rect = bounds;
    QPointF center = rect.center();
    
    // 菱形的四个点：上、右、下、左
    QPointF top(center.x(), rect.top());
    QPointF right(rect.right(), center.y());
    QPointF bottom(center.x(), rect.bottom());
    QPointF left(rect.left(), center.y());
    
    path.moveTo(top);
    path.lineTo(right);
    path.lineTo(bottom);
    path.lineTo(left);
    path.closeSubpath();
    
    // 绘制菱形
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

bool Diamond::hitTest(const QPointF& pt) const
{
    // 创建菱形路径用于命中测试
    QPainterPath path;
    QRectF rect = bounds;
    QPointF center = rect.center();
    
    QPointF top(center.x(), rect.top());
    QPointF right(rect.right(), center.y());
    QPointF bottom(center.x(), rect.bottom());
    QPointF left(rect.left(), center.y());
    
    path.moveTo(top);
    path.lineTo(right);
    path.lineTo(bottom);
    path.lineTo(left);
    path.closeSubpath();
    
    return path.contains(pt);
}

QJsonObject Diamond::toJson() const
{
    return QJsonObject{
        {"type", "diamond"},
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

void Diamond::fromJson(const QJsonObject& o)
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