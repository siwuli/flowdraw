#include "Ellipse.hpp"
#include <QtMath>

void Ellipse::paint(QPainter& p, bool selected) const
{
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    p.drawEllipse(bounds);
    
    // 绘制文本
    drawText(p);

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

QPointF Ellipse::getConnectionPoint(const QPointF& ref) const
{
    // 获取椭圆中心点
    QPointF center = bounds.center();
    
    // 计算参考点到椭圆中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 椭圆的半径
    double a = bounds.width() / 2.0;  // 水平半径
    double b = bounds.height() / 2.0; // 垂直半径
    
    // 计算方向向量的角度
    double angle = std::atan2(direction.y(), direction.x());
    
    // 根据椭圆方程计算交点
    // x = a * cos(t), y = b * sin(t)
    // 连接点坐标
    double x = center.x() + a * std::cos(angle);
    double y = center.y() + b * std::sin(angle);
    
    return QPointF(x, y);
}

QJsonObject Ellipse::toJson() const
{
    return QJsonObject{
        {"type","ellipse"},  // 修正这里的类型
        {"x",bounds.x()}, {"y",bounds.y()},
        {"w",bounds.width()}, {"h",bounds.height()},
        {"fill",   fillColor.name(QColor::HexArgb)},
        {"stroke", strokeColor.name(QColor::HexArgb)},
        {"width",  strokeWidth},
        {"text", text},
        {"textColor", textColor.name(QColor::HexArgb)},
        {"textSize", textSize}
    };
}

void Ellipse::fromJson(const QJsonObject& o)
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
