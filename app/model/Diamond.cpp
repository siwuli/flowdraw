#include "Diamond.hpp"
#include <QPainterPath>
#include <limits>

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

QPointF Diamond::getConnectionPoint(const QPointF& ref) const
{
    QRectF rect = bounds;
    QPointF center = rect.center();
    
    // 菱形的四个点：上、右、下、左
    QPointF top(center.x(), rect.top());
    QPointF right(rect.right(), center.y());
    QPointF bottom(center.x(), rect.bottom());
    QPointF left(rect.left(), center.y());
    
    // 计算参考点到菱形中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 菱形的四条边
    QLineF topToRight(top, right);
    QLineF rightToBottom(right, bottom);
    QLineF bottomToLeft(bottom, left);
    QLineF leftToTop(left, top);
    
    // 从中心点向外射线
    QLineF ray(center, center + direction * 1000.0); // 足够长以确保与边相交
    
    // 检查与哪条边相交，选择最近的交点
    QPointF intersection;
    QPointF bestIntersection;
    qreal bestDistance = std::numeric_limits<qreal>::max();
    
    // 检查与所有边的交点
    if (ray.intersect(topToRight, &intersection) == QLineF::BoundedIntersection) {
        qreal distance = QLineF(center, intersection).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIntersection = intersection;
        }
    }
    
    if (ray.intersect(rightToBottom, &intersection) == QLineF::BoundedIntersection) {
        qreal distance = QLineF(center, intersection).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIntersection = intersection;
        }
    }
    
    if (ray.intersect(bottomToLeft, &intersection) == QLineF::BoundedIntersection) {
        qreal distance = QLineF(center, intersection).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestIntersection = intersection;
        }
    }
    
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
    
    // 如果没有找到交点（这是一个安全措施），使用四个顶点中最近的点
    QPointF vertices[4] = { top, right, bottom, left };
    QPointF bestVertex = vertices[0];
    bestDistance = QLineF(ref, bestVertex).length();
    
    for (int i = 1; i < 4; ++i) {
        qreal distance = QLineF(ref, vertices[i]).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestVertex = vertices[i];
        }
    }
    
    return bestVertex;
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