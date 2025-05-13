#include "Triangle.hpp"
#include <QPainterPath>
#include <QtMath>
#include <limits>

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

QPointF Triangle::getConnectionPoint(const QPointF& ref) const 
{
    QRectF rect = bounds;
    
    // 三角形的三个点
    QPointF top(rect.center().x(), rect.top());
    QPointF bottomLeft(rect.left(), rect.bottom());
    QPointF bottomRight(rect.right(), rect.bottom());
    
    // 计算参考点到三角形中心的方向向量
    QPointF center = rect.center();
    QPointF direction = ref - center;
    
    // 三角形的三条边
    QLineF topToRight(top, bottomRight);
    QLineF rightToLeft(bottomRight, bottomLeft);
    QLineF leftToTop(bottomLeft, top);
    
    // 从中心点向外射线
    QLineF ray(center, center + direction * 1000.0); // 足够长以确保与边相交
    
    // 检查与哪条边相交，选择最近的交点
    QPointF intersection;
    QPointF bestIntersection;
    qreal bestDistance = std::numeric_limits<qreal>::max();
    
    // 检查与顶部到右下角边的交点
    if (ray.intersect(topToRight, &intersection) == QLineF::BoundedIntersection) {
        qreal distance = QLineF(center, intersection).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIntersection = intersection;
        }
    }
    
    // 检查与右下角到左下角边的交点
    if (ray.intersect(rightToLeft, &intersection) == QLineF::BoundedIntersection) {
        qreal distance = QLineF(center, intersection).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIntersection = intersection;
        }
    }
    
    // 检查与左下角到顶部边的交点
    if (ray.intersect(leftToTop, &intersection) == QLineF::BoundedIntersection) {
        qreal distance = QLineF(center, intersection).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIntersection = intersection;
        }
    }
    
    // 如果找到交点，返回它；否则使用默认实现
    if (bestDistance < std::numeric_limits<qreal>::max()) {
        return bestIntersection;
    }
    
    // 如果没有找到交点（这是一个安全措施），使用三角形的顶点
    QPointF vertices[3] = { top, bottomLeft, bottomRight };
    QPointF bestVertex = vertices[0];
    bestDistance = QLineF(ref, bestVertex).length();
    
    for (int i = 1; i < 3; ++i) {
        qreal distance = QLineF(ref, vertices[i]).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestVertex = vertices[i];
        }
    }
    
    return bestVertex;
} 