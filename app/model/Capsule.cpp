#include "Capsule.hpp"
#include <QPainterPath>
#include <cmath>

void Capsule::paint(QPainter& p, bool selected) const
{
    // 绘制胶囊形状（两端为半圆形，中间为矩形）
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    
    // 创建胶囊路径
    QPainterPath path;
    qreal radius;
    
    // 确定半圆半径，取宽高中较小值的一半
    if (bounds.width() < bounds.height()) {
        // 水平胶囊，左右两端为半圆
        radius = bounds.width() / 2.0;
        
        // 添加左半圆
        path.moveTo(bounds.left() + radius, bounds.top());
        path.arcTo(bounds.left(), bounds.top(), 
                  bounds.width(), bounds.width(), 
                  90, 180);
                  
        // 添加右半圆
        path.arcTo(bounds.left(), bounds.bottom() - bounds.width(), 
                  bounds.width(), bounds.width(), 
                  270, 180);
        
        // 闭合路径
        path.closeSubpath();
    } else {
        // 垂直胶囊，上下两端为半圆
        radius = bounds.height() / 2.0;
        
        // 添加上半圆
        path.moveTo(bounds.left(), bounds.top() + radius);
        path.arcTo(bounds.left(), bounds.top(), 
                  bounds.height(), bounds.height(), 
                  180, 180);
                  
        // 添加下半圆
        path.arcTo(bounds.right() - bounds.height(), bounds.top(), 
                  bounds.height(), bounds.height(), 
                  0, 180);
                  
        // 闭合路径
        path.closeSubpath();
    }
    
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

bool Capsule::hitTest(const QPointF& pt) const
{
    // 判断点是否在胶囊内
    if (!bounds.contains(pt)) {
        return false;  // 如果不在外围矩形内，直接返回false
    }
    
    QPainterPath path;
    qreal radius;
    
    // 根据宽高创建适当的胶囊路径
    if (bounds.width() < bounds.height()) {
        // 水平胶囊
        radius = bounds.width() / 2.0;
        
        // 左半圆
        path.moveTo(bounds.left() + radius, bounds.top());
        path.arcTo(bounds.left(), bounds.top(), 
                  bounds.width(), bounds.width(), 
                  90, 180);
                  
        // 右半圆
        path.arcTo(bounds.left(), bounds.bottom() - bounds.width(), 
                  bounds.width(), bounds.width(), 
                  270, 180);
        
        // 闭合路径
        path.closeSubpath();
    } else {
        // 垂直胶囊
        radius = bounds.height() / 2.0;
        
        // 上半圆
        path.moveTo(bounds.left(), bounds.top() + radius);
        path.arcTo(bounds.left(), bounds.top(), 
                  bounds.height(), bounds.height(), 
                  180, 180);
                  
        // 下半圆
        path.arcTo(bounds.right() - bounds.height(), bounds.top(), 
                  bounds.height(), bounds.height(), 
                  0, 180);
                  
        // 闭合路径
        path.closeSubpath();
    }
    
    return path.contains(pt);
}

QPointF Capsule::getConnectionPoint(const QPointF& ref) const
{
    // 获取胶囊的中心点
    QPointF center = bounds.center();
    
    // 计算参考点到胶囊中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 归一化方向向量
    qreal length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    QPointF unitDir = direction / length;
    
    // 胶囊形状的外边界点计算方法
    qreal a = bounds.width() / 2.0;  // 水平半轴长
    qreal b = bounds.height() / 2.0; // 垂直半轴长
    
    qreal r;
    if (bounds.width() < bounds.height()) {
        // 水平胶囊
        qreal dx = std::abs(unitDir.x());
        qreal dy = std::abs(unitDir.y());
        
        if (dy < 1e-6) {
            // 水平方向
            r = a;
        } else {
            // 计算与胶囊边界的交点
            r = (a * b) / std::sqrt(b * b * dx * dx + a * a * dy * dy);
        }
    } else {
        // 垂直胶囊
        qreal dx = std::abs(unitDir.x());
        qreal dy = std::abs(unitDir.y());
        
        if (dx < 1e-6) {
            // 垂直方向
            r = b;
        } else {
            // 计算与胶囊边界的交点
            r = (a * b) / std::sqrt(b * b * dx * dx + a * a * dy * dy);
        }
    }
    
    // 计算边界点
    return center + unitDir * r;
}

QJsonObject Capsule::toJson() const
{
    return QJsonObject{
        {"type", "capsule"},
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

void Capsule::fromJson(const QJsonObject& o)
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