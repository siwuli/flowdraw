#include "Capsule.hpp"
#include <QPainterPath>
#include <cmath>

// 在Windows平台上定义M_PI（如果尚未定义）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void Capsule::paint(QPainter& p, bool selected) const
{
    // 绘制胶囊形状（两端为半圆形，中间为矩形）
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    
    // 创建胶囊路径
    QPainterPath path;
    
    if (bounds.width() >= bounds.height()) {
        // 水平胶囊 - 左右两端为半圆
        qreal radius = bounds.height() / 2.0;
        
        // 圆心坐标
        qreal leftCenterX = bounds.left() + radius;
        qreal rightCenterX = bounds.right() - radius;
        qreal centerY = bounds.center().y();
        
        // 从左半圆的最右上点开始
        path.moveTo(leftCenterX, bounds.top());
        
        // 画右上方的直线
        path.lineTo(rightCenterX, bounds.top());
        
        // 画右半圆
        path.arcTo(rightCenterX - radius, centerY - radius, 2 * radius, 2 * radius, 90, -180);
        
        // 画左下方的直线
        path.lineTo(leftCenterX, bounds.bottom());
        
        // 画左半圆
        path.arcTo(leftCenterX - radius, centerY - radius, 2 * radius, 2 * radius, -90, -180);
    } else {
        // 垂直胶囊 - 上下两端为半圆
        qreal radius = bounds.width() / 2.0;
        
        // 圆心坐标
        qreal topCenterY = bounds.top() + radius;
        qreal bottomCenterY = bounds.bottom() - radius;
        qreal centerX = bounds.center().x();
        
        // 从上半圆的最左下点开始
        path.moveTo(bounds.left(), topCenterY);
        
        // 画左下方的直线
        path.lineTo(bounds.left(), bottomCenterY);
        
        // 画下半圆
        path.arcTo(centerX - radius, bottomCenterY - radius, 2 * radius, 2 * radius, 180, -180);
        
        // 画右上方的直线
        path.lineTo(bounds.right(), topCenterY);
        
        // 画上半圆
        path.arcTo(centerX - radius, topCenterY - radius, 2 * radius, 2 * radius, 0, -180);
    }
    
    // 闭合路径
    path.closeSubpath();
    
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
    
    if (bounds.width() >= bounds.height()) {
        // 水平胶囊 - 左右两端为半圆
        qreal radius = bounds.height() / 2.0;
        
        // 圆心坐标
        qreal leftCenterX = bounds.left() + radius;
        qreal rightCenterX = bounds.right() - radius;
        qreal centerY = bounds.center().y();
        
        // 从左半圆的最右上点开始
        path.moveTo(leftCenterX, bounds.top());
        
        // 画右上方的直线
        path.lineTo(rightCenterX, bounds.top());
        
        // 画右半圆
        path.arcTo(rightCenterX - radius, centerY - radius, 2 * radius, 2 * radius, 90, -180);
        
        // 画左下方的直线
        path.lineTo(leftCenterX, bounds.bottom());
        
        // 画左半圆
        path.arcTo(leftCenterX - radius, centerY - radius, 2 * radius, 2 * radius, -90, -180);
    } else {
        // 垂直胶囊 - 上下两端为半圆
        qreal radius = bounds.width() / 2.0;
        
        // 圆心坐标
        qreal topCenterY = bounds.top() + radius;
        qreal bottomCenterY = bounds.bottom() - radius;
        qreal centerX = bounds.center().x();
        
        // 从上半圆的最左下点开始
        path.moveTo(bounds.left(), topCenterY);
        
        // 画左下方的直线
        path.lineTo(bounds.left(), bottomCenterY);
        
        // 画下半圆
        path.arcTo(centerX - radius, bottomCenterY - radius, 2 * radius, 2 * radius, 180, -180);
        
        // 画右上方的直线
        path.lineTo(bounds.right(), topCenterY);
        
        // 画上半圆
        path.arcTo(centerX - radius, topCenterY - radius, 2 * radius, 2 * radius, 0, -180);
    }
    
    // 闭合路径
    path.closeSubpath();
    
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
    
    // 创建胶囊路径
    QPainterPath path;
    
    if (bounds.width() >= bounds.height()) {
        // 水平胶囊 - 左右两端为半圆
        qreal radius = bounds.height() / 2.0;
        
        // 圆心坐标
        qreal leftCenterX = bounds.left() + radius;
        qreal rightCenterX = bounds.right() - radius;
        qreal centerY = bounds.center().y();
        
        // 从左半圆的最右上点开始
        path.moveTo(leftCenterX, bounds.top());
        
        // 画右上方的直线
        path.lineTo(rightCenterX, bounds.top());
        
        // 画右半圆
        path.arcTo(rightCenterX - radius, centerY - radius, 2 * radius, 2 * radius, 90, -180);
        
        // 画左下方的直线
        path.lineTo(leftCenterX, bounds.bottom());
        
        // 画左半圆
        path.arcTo(leftCenterX - radius, centerY - radius, 2 * radius, 2 * radius, -90, -180);
    } else {
        // 垂直胶囊 - 上下两端为半圆
        qreal radius = bounds.width() / 2.0;
        
        // 圆心坐标
        qreal topCenterY = bounds.top() + radius;
        qreal bottomCenterY = bounds.bottom() - radius;
        qreal centerX = bounds.center().x();
        
        // 从上半圆的最左下点开始
        path.moveTo(bounds.left(), topCenterY);
        
        // 画左下方的直线
        path.lineTo(bounds.left(), bottomCenterY);
        
        // 画下半圆
        path.arcTo(centerX - radius, bottomCenterY - radius, 2 * radius, 2 * radius, 180, -180);
        
        // 画右上方的直线
        path.lineTo(bounds.right(), topCenterY);
        
        // 画上半圆
        path.arcTo(centerX - radius, topCenterY - radius, 2 * radius, 2 * radius, 0, -180);
    }
    
    // 闭合路径
    path.closeSubpath();
    
    // 从中心点向外延伸足够长的线段
    qreal maxDist = bounds.width() + bounds.height();
    QLineF line(center, center + unitDir * maxDist);
    
    // 查找交点
    QPointF result = center;
    QPointF intersectionPoint;
    
    // 定义线段与路径轮廓的交点，我们简化计算
    if (bounds.width() >= bounds.height()) {
        // 水平胶囊
        qreal radius = bounds.height() / 2.0;
        qreal dx = std::abs(unitDir.x());
        qreal dy = std::abs(unitDir.y());
        
        if (dy < 0.1) {
            // 几乎水平
            if (unitDir.x() > 0) {
                result.setX(bounds.right());
            } else {
                result.setX(bounds.left());
            }
            result.setY(center.y());
        } else if (dx < 0.1) {
            // 几乎垂直
            result.setX(center.x());
            if (unitDir.y() > 0) {
                result.setY(bounds.bottom());
            } else {
                result.setY(bounds.top());
            }
        } else {
            // 斜向，需要考虑是否与圆弧相交
            qreal slope = unitDir.y() / unitDir.x();
            qreal halfHeight = bounds.height() / 2.0;
            
            // 检查与水平边的交点
            qreal x;
            if (unitDir.y() < 0) { // 向上
                x = center.x() + (bounds.top() - center.y()) / slope;
            } else { // 向下
                x = center.x() + (bounds.bottom() - center.y()) / slope;
            }
            
            // 检查交点是否在矩形部分
            if (x > bounds.left() + radius && x < bounds.right() - radius) {
                // 与水平边相交
                result.setX(x);
                if (unitDir.y() < 0) {
                    result.setY(bounds.top());
                } else {
                    result.setY(bounds.bottom());
                }
            } else {
                // 与圆弧相交
                qreal circleX;
                if (unitDir.x() < 0) {
                    circleX = bounds.left() + radius; // 左圆弧中心
                } else {
                    circleX = bounds.right() - radius; // 右圆弧中心
                }
                
                // 计算与圆弧的交点
                QPointF circleCenter(circleX, center.y());
                QLineF lineToCircle(center, circleCenter);
                qreal angle = lineToCircle.angle();
                qreal angleOffset = 0;
                
                if (unitDir.y() < 0) { // 上半部分
                    angleOffset = -std::atan(slope) * 180.0 / M_PI;
                } else { // 下半部分
                    angleOffset = std::atan(slope) * 180.0 / M_PI;
                }
                
                qreal finalAngle = angle + angleOffset;
                result.setX(circleX + radius * std::cos(finalAngle * M_PI / 180.0));
                result.setY(center.y() + radius * std::sin(finalAngle * M_PI / 180.0));
            }
        }
    } else {
        // 垂直胶囊
        qreal radius = bounds.width() / 2.0;
        qreal dx = std::abs(unitDir.x());
        qreal dy = std::abs(unitDir.y());
        
        if (dx < 0.1) {
            // 几乎垂直
            result.setX(center.x());
            if (unitDir.y() > 0) {
                result.setY(bounds.bottom());
            } else {
                result.setY(bounds.top());
            }
        } else if (dy < 0.1) {
            // 几乎水平
            if (unitDir.x() > 0) {
                result.setX(bounds.right());
            } else {
                result.setX(bounds.left());
            }
            result.setY(center.y());
        } else {
            // 斜向，需要考虑是否与圆弧相交
            qreal slope = unitDir.y() / unitDir.x();
            qreal halfWidth = bounds.width() / 2.0;
            
            // 检查与垂直边的交点
            qreal y;
            if (unitDir.x() < 0) { // 向左
                y = center.y() + slope * (bounds.left() - center.x());
            } else { // 向右
                y = center.y() + slope * (bounds.right() - center.x());
            }
            
            // 检查交点是否在矩形部分
            if (y > bounds.top() + radius && y < bounds.bottom() - radius) {
                // 与垂直边相交
                if (unitDir.x() < 0) {
                    result.setX(bounds.left());
                } else {
                    result.setX(bounds.right());
                }
                result.setY(y);
            } else {
                // 与圆弧相交
                qreal circleY;
                if (unitDir.y() < 0) {
                    circleY = bounds.top() + radius; // 上圆弧中心
                } else {
                    circleY = bounds.bottom() - radius; // 下圆弧中心
                }
                
                // 计算与圆弧的交点
                QPointF circleCenter(center.x(), circleY);
                QLineF lineToCircle(center, circleCenter);
                qreal angle = lineToCircle.angle();
                qreal angleOffset = 0;
                
                if (unitDir.x() < 0) { // 左半部分
                    angleOffset = -std::atan(1.0/slope) * 180.0 / M_PI;
                } else { // 右半部分
                    angleOffset = std::atan(1.0/slope) * 180.0 / M_PI;
                }
                
                qreal finalAngle = angle + angleOffset;
                result.setX(center.x() + radius * std::cos(finalAngle * M_PI / 180.0));
                result.setY(circleY + radius * std::sin(finalAngle * M_PI / 180.0));
            }
        }
    }
    
    return result;
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