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
    
    // 获取边界的宽度和高度
    qreal w = bounds.width();
    qreal h = bounds.height();
    
    if (w >= h) {
        // 水平胶囊 - 左右两端为半圆
        qreal diameter = h; // 半圆的直径等于高度
        qreal radius = diameter / 2.0;
        
        // 左右半圆的矩形区域
        QRectF leftCircle(bounds.left(), bounds.top(), diameter, diameter);
        QRectF rightCircle(bounds.right() - diameter, bounds.top(), diameter, diameter);
        
        // 绘制路径：顺时针方向
        path.moveTo(bounds.left() + radius, bounds.top());                   // 左上角起点
        path.lineTo(bounds.right() - radius, bounds.top());                  // 向右到右上角
        path.arcTo(rightCircle, 90, -180);                                   // 右半圆：从90度开始，逆时针旋转180度
        path.lineTo(bounds.left() + radius, bounds.bottom());                // 向左到左下角
        path.arcTo(leftCircle, 270, -180);                                   // 左半圆：从270度开始，逆时针旋转180度
        path.closeSubpath();
    } else {
        // 垂直胶囊 - 上下两端为半圆
        qreal diameter = w; // 半圆的直径等于宽度
        qreal radius = diameter / 2.0;
        
        // 上下半圆的矩形区域
        QRectF topCircle(bounds.left(), bounds.top(), diameter, diameter);
        QRectF bottomCircle(bounds.left(), bounds.bottom() - diameter, diameter, diameter);
        
        // 绘制路径：顺时针方向
        path.moveTo(bounds.left(), bounds.top() + radius);                   // 左上角起点
        path.arcTo(topCircle, 180, -180);                                    // 上半圆：从180度开始，逆时针旋转180度
        path.lineTo(bounds.right(), bounds.bottom() - radius);               // 向下到右下角
        path.arcTo(bottomCircle, 0, -180);                                   // 下半圆：从0度开始，逆时针旋转180度
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
    
    // 获取边界的宽度和高度
    qreal w = bounds.width();
    qreal h = bounds.height();
    
    if (w >= h) {
        // 水平胶囊 - 左右两端为半圆
        qreal diameter = h; // 半圆的直径等于高度
        qreal radius = diameter / 2.0;
        
        // 左右半圆的矩形区域
        QRectF leftCircle(bounds.left(), bounds.top(), diameter, diameter);
        QRectF rightCircle(bounds.right() - diameter, bounds.top(), diameter, diameter);
        
        // 绘制路径：顺时针方向
        path.moveTo(bounds.left() + radius, bounds.top());                   // 左上角起点
        path.lineTo(bounds.right() - radius, bounds.top());                  // 向右到右上角
        path.arcTo(rightCircle, 90, -180);                                   // 右半圆：从90度开始，逆时针旋转180度
        path.lineTo(bounds.left() + radius, bounds.bottom());                // 向左到左下角
        path.arcTo(leftCircle, 270, -180);                                   // 左半圆：从270度开始，逆时针旋转180度
        path.closeSubpath();
    } else {
        // 垂直胶囊 - 上下两端为半圆
        qreal diameter = w; // 半圆的直径等于宽度
        qreal radius = diameter / 2.0;
        
        // 上下半圆的矩形区域
        QRectF topCircle(bounds.left(), bounds.top(), diameter, diameter);
        QRectF bottomCircle(bounds.left(), bounds.bottom() - diameter, diameter, diameter);
        
        // 绘制路径：顺时针方向
        path.moveTo(bounds.left(), bounds.top() + radius);                   // 左上角起点
        path.arcTo(topCircle, 180, -180);                                    // 上半圆：从180度开始，逆时针旋转180度
        path.lineTo(bounds.right(), bounds.bottom() - radius);               // 向下到右下角
        path.arcTo(bottomCircle, 0, -180);                                   // 下半圆：从0度开始，逆时针旋转180度
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
    
    // 获取边界的宽度和高度
    qreal w = bounds.width();
    qreal h = bounds.height();
    
    if (w >= h) {
        // 水平胶囊 - 左右两端为半圆
        qreal diameter = h; // 半圆的直径等于高度
        qreal radius = diameter / 2.0;
        
        // 计算矩形部分的边界位置
        qreal rectLeft = bounds.left() + radius;
        qreal rectRight = bounds.right() - radius;
        qreal centerY = bounds.center().y();
        
        // 检查是否与矩形部分相交
        if (unitDir.y() != 0) {
            // 计算与水平边界的交点
            qreal t;
            QPointF intersection;
            qreal y;
            
            if (unitDir.y() > 0) {
                y = bounds.bottom();
            } else {
                y = bounds.top();
            }
            
            t = (y - center.y()) / unitDir.y();
            intersection.setX(center.x() + t * unitDir.x());
            intersection.setY(y);
            
            // 检查交点是否在矩形部分
            if (intersection.x() >= rectLeft && intersection.x() <= rectRight) {
                return intersection;
            }
        }
        
        // 检查是否与半圆部分相交
        QPointF circleCenter;
        if (unitDir.x() >= 0) {
            circleCenter = QPointF(rectRight, centerY);
        } else {
            circleCenter = QPointF(rectLeft, centerY);
        }
        
        // 计算从圆心到交点的向量
        QPointF toCircle = circleCenter - center;
        qreal distToCircle = QLineF(center, circleCenter).length();
        qreal dotProduct = toCircle.x() * unitDir.x() + toCircle.y() * unitDir.y();
        
        // 计算圆上的交点
        qreal projLength;
        
        if (distToCircle < 1e-6) {
            // 极少见的情况：圆心与胶囊中心重合
            projLength = radius;
        } else {
            projLength = dotProduct / distToCircle;
        }
        
        QPointF closestPoint = circleCenter - projLength * (toCircle / distToCircle);
        QPointF pointOnCircle = circleCenter + radius * ((closestPoint - circleCenter) / QLineF(closestPoint, circleCenter).length());
        
        return pointOnCircle;
    } else {
        // 垂直胶囊 - 上下两端为半圆
        qreal diameter = w; // 半圆的直径等于宽度
        qreal radius = diameter / 2.0;
        
        // 计算矩形部分的边界位置
        qreal rectTop = bounds.top() + radius;
        qreal rectBottom = bounds.bottom() - radius;
        qreal centerX = bounds.center().x();
        
        // 检查是否与矩形部分相交
        if (unitDir.x() != 0) {
            // 计算与垂直边界的交点
            qreal t;
            QPointF intersection;
            qreal x;
            
            if (unitDir.x() > 0) {
                x = bounds.right();
            } else {
                x = bounds.left();
            }
            
            t = (x - center.x()) / unitDir.x();
            intersection.setX(x);
            intersection.setY(center.y() + t * unitDir.y());
            
            // 检查交点是否在矩形部分
            if (intersection.y() >= rectTop && intersection.y() <= rectBottom) {
                return intersection;
            }
        }
        
        // 检查是否与半圆部分相交
        QPointF circleCenter;
        if (unitDir.y() >= 0) {
            circleCenter = QPointF(centerX, rectBottom);
        } else {
            circleCenter = QPointF(centerX, rectTop);
        }
        
        // 计算从圆心到交点的向量
        QPointF toCircle = circleCenter - center;
        qreal distToCircle = QLineF(center, circleCenter).length();
        qreal dotProduct = toCircle.x() * unitDir.x() + toCircle.y() * unitDir.y();
        
        // 计算圆上的交点
        qreal projLength;
        
        if (distToCircle < 1e-6) {
            // 极少见的情况：圆心与胶囊中心重合
            projLength = radius;
        } else {
            projLength = dotProduct / distToCircle;
        }
        
        QPointF closestPoint = circleCenter - projLength * (toCircle / distToCircle);
        QPointF pointOnCircle = circleCenter + radius * ((closestPoint - circleCenter) / QLineF(closestPoint, circleCenter).length());
        
        return pointOnCircle;
    }
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