#include "Sector.hpp"
#include <QPainterPath>
#include <cmath>

// 在Windows平台上定义M_PI（如果尚未定义）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Sector::paint(QPainter& p, bool selected) const
{
    // 绘制扇形
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    
    // 计算扇形路径
    QPainterPath path;
    
    // 扇形的中心点
    QPointF center = bounds.center();
    
    // 扇形的半径（取宽高中较小值的一半）
    qreal radius = qMin(bounds.width(), bounds.height()) / 2.0;
    
    // 添加扇形路径
    path.moveTo(center);
    path.arcTo(
        center.x() - radius, 
        center.y() - radius, 
        radius * 2, 
        radius * 2, 
        startAngle, 
        spanAngle
    );
    path.lineTo(center);
    path.closeSubpath();
    
    // 绘制扇形
    p.drawPath(path);
    
    // 绘制文本
    drawText(p);
    
    // 如果被选中，绘制虚线框，适应扇形形状
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

bool Sector::hitTest(const QPointF& pt) const
{
    // 检查点是否在扇形内
    
    // 先检查点是否在外围矩形内
    if (!bounds.contains(pt)) {
        return false;
    }
    
    // 扇形的中心点
    QPointF center = bounds.center();
    
    // 扇形的半径
    qreal radius = qMin(bounds.width(), bounds.height()) / 2.0;
    
    // 计算点到中心的距离
    QPointF diff = pt - center;
    qreal distance = std::sqrt(diff.x() * diff.x() + diff.y() * diff.y());
    
    // 如果距离大于半径，则不在扇形内
    if (distance > radius) {
        return false;
    }
    
    // 计算点的角度（弧度）
    qreal angle = std::atan2(diff.y(), diff.x()) * 180.0 / M_PI;
    
    // 将角度转换为与startAngle兼容的范围
    while (angle < startAngle)
        angle += 360.0;
    
    // 检查点的角度是否在扇形的角度范围内
    qreal endAngle = startAngle + spanAngle;
    return angle >= startAngle && angle <= endAngle;
}

QPointF Sector::getConnectionPoint(const QPointF& ref) const
{
    // 获取扇形的中心点
    QPointF center = bounds.center();
    
    // 计算参考点到扇形中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 计算方向的角度（弧度）
    qreal angle = std::atan2(direction.y(), direction.x()) * 180.0 / M_PI;
    
    // 调整角度到0-360范围
    while (angle < 0)
        angle += 360.0;
    
    // 检查角度是否在扇形范围内
    qreal endAngle = startAngle + spanAngle;
    qreal clampedAngle = angle;
    
    // 如果角度不在扇形范围内，则使用最近的边界角度
    if (angle < startAngle || angle > endAngle) {
        qreal distToStart = std::abs(angle - startAngle);
        qreal distToEnd = std::abs(angle - endAngle);
        
        clampedAngle = (distToStart <= distToEnd) ? startAngle : endAngle;
    }
    
    // 转换成弧度
    qreal radians = clampedAngle * M_PI / 180.0;
    
    // 扇形的半径
    qreal radius = qMin(bounds.width(), bounds.height()) / 2.0;
    
    // 计算连接点
    QPointF connectionPoint(
        center.x() + radius * std::cos(radians),
        center.y() + radius * std::sin(radians)
    );
    
    return connectionPoint;
}

QJsonObject Sector::toJson() const
{
    return QJsonObject{
        {"type", "sector"},
        {"x", bounds.x()}, {"y", bounds.y()},
        {"w", bounds.width()}, {"h", bounds.height()},
        {"fill", fillColor.name(QColor::HexArgb)},
        {"stroke", strokeColor.name(QColor::HexArgb)},
        {"width", strokeWidth},
        {"text", text},
        {"textColor", textColor.name(QColor::HexArgb)},
        {"textSize", textSize},
        {"startAngle", startAngle},
        {"spanAngle", spanAngle}
    };
}

void Sector::fromJson(const QJsonObject& o)
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
} 