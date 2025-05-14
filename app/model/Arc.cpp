#include "Arc.hpp"
#include <QPainterPath>
#include <cmath>

// 在Windows平台上定义M_PI（如果尚未定义）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Arc::paint(QPainter& p, bool selected) const
{
    // 绘制圆环
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    
    // 计算圆环路径
    QPainterPath path;
    
    // 圆环的中心点
    QPointF center = bounds.center();
    
    // 圆环的外半径（取宽高中较小值的一半）
    qreal outerRadius = qMin(bounds.width(), bounds.height()) / 2.0;
    
    // 圆环的内半径
    qreal innerRadius = qMax(outerRadius - thickness, 5.0); // 确保内半径至少5个像素
    
    // 创建外弧形和内弧形的矩形区域
    QRectF outerRect(
        center.x() - outerRadius,
        center.y() - outerRadius,
        outerRadius * 2,
        outerRadius * 2
    );
    
    QRectF innerRect(
        center.x() - innerRadius,
        center.y() - innerRadius,
        innerRadius * 2,
        innerRadius * 2
    );
    
    // 开始构建路径
    // 添加外弧形
    path.arcTo(outerRect, startAngle, spanAngle);
    
    // 添加内弧形（反方向）
    path.arcTo(innerRect, startAngle + spanAngle, -spanAngle);
    
    // 闭合路径
    path.closeSubpath();
    
    // 绘制圆环
    p.drawPath(path);
    
    // 绘制文本
    drawText(p);
    
    // 如果被选中，绘制虚线框，适应圆环形状
    if (selected) {
        QPen dashPen(Qt::DashLine);
        dashPen.setColor(Qt::blue);
        p.setPen(dashPen);
        p.setBrush(Qt::NoBrush);
        
        // 使用稍微放大的路径绘制选中框
        QPainterPath selectionPath = path;
        QTransform transform;
        transform.translate(center.x(), center.y());
        transform.scale(1.04, 1.04); // 比实际形状稍大
        transform.translate(-center.x(), -center.y());
        selectionPath = transform.map(selectionPath);
        
        p.drawPath(selectionPath);
    }
}

bool Arc::hitTest(const QPointF& pt) const
{
    // 检查点是否在圆环内
    
    // 先检查点是否在外围矩形内
    if (!bounds.contains(pt)) {
        return false;
    }
    
    // 圆环的中心点
    QPointF center = bounds.center();
    
    // 计算点到中心的距离
    QPointF diff = pt - center;
    qreal distance = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
    
    // 圆环的外半径和内半径
    qreal outerRadius = qMin(bounds.width(), bounds.height()) / 2.0;
    qreal innerRadius = qMax(outerRadius - thickness, 5.0);
    
    // 如果距离不在圆环的半径范围内，则不在圆环内
    if (distance < innerRadius || distance > outerRadius) {
        return false;
    }
    
    // 计算点的角度（弧度）
    qreal angle = std::atan2(diff.y(), diff.x()) * 180.0 / M_PI;
    
    // 将角度调整到0-360范围
    while (angle < 0)
        angle += 360.0;
    
    // 计算圆环的结束角度
    qreal endAngle = startAngle + spanAngle;
    
    // 检查点的角度是否在圆环的角度范围内
    return angle >= startAngle && angle <= endAngle;
}

QPointF Arc::getConnectionPoint(const QPointF& ref) const
{
    // 获取圆环的中心点
    QPointF center = bounds.center();
    
    // 计算参考点到圆环中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 计算方向的角度（度）
    qreal angle = std::atan2(direction.y(), direction.x()) * 180.0 / M_PI;
    
    // 调整角度到0-360范围
    while (angle < 0)
        angle += 360.0;
    
    // 检查角度是否在圆环范围内
    qreal endAngle = startAngle + spanAngle;
    qreal clampedAngle = angle;
    
    // 如果角度不在圆环范围内，则使用最近的边界角度
    if (angle < startAngle || angle > endAngle) {
        qreal distToStart = std::abs(angle - startAngle);
        qreal distToEnd = std::abs(angle - endAngle);
        
        clampedAngle = (distToStart <= distToEnd) ? startAngle : endAngle;
    }
    
    // 转换成弧度
    qreal radians = clampedAngle * M_PI / 180.0;
    
    // 圆环的外半径
    qreal radius = qMin(bounds.width(), bounds.height()) / 2.0;
    
    // 计算圆环中间点的连接点
    qreal connectRadius = radius - thickness / 2.0;
    QPointF connectionPoint(
        center.x() + connectRadius * std::cos(radians),
        center.y() + connectRadius * std::sin(radians)
    );
    
    return connectionPoint;
}

QJsonObject Arc::toJson() const
{
    return QJsonObject{
        {"type", "arc"},
        {"x", bounds.x()}, {"y", bounds.y()},
        {"w", bounds.width()}, {"h", bounds.height()},
        {"fill", fillColor.name(QColor::HexArgb)},
        {"stroke", strokeColor.name(QColor::HexArgb)},
        {"width", strokeWidth},
        {"text", text},
        {"textColor", textColor.name(QColor::HexArgb)},
        {"textSize", textSize},
        {"startAngle", startAngle},
        {"spanAngle", spanAngle},
        {"thickness", thickness}
    };
}

void Arc::fromJson(const QJsonObject& o)
{
    bounds = { o["x"].toDouble(), o["y"].toDouble(),
               o["w"].toDouble(), o["h"].toDouble() };
    fillColor = QColor(o["fill"].toString("#ffffffff"));
    strokeColor = QColor(o["stroke"].toString("#ff000000"));
    strokeWidth = o["width"].toDouble(1.5);
    text = o["text"].toString();
    textColor = QColor(o["textColor"].toString("#ff000000"));
    textSize = o["textSize"].toInt(10);
    startAngle = o["startAngle"].toDouble(0);
    spanAngle = o["spanAngle"].toDouble(90);
    thickness = o["thickness"].toDouble(20);
} 