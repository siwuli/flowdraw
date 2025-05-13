#include "Pentagon.hpp"
#include <QPainterPath>
#include <QtMath>
#include <limits>

void Pentagon::paint(QPainter& p, bool selected) const
{
    // 创建五边形路径
    QPainterPath path;
    QRectF rect = bounds;
    QPointF center = rect.center();
    
    // 计算五边形的五个顶点
    const int numPoints = 5;
    const qreal radius = qMin(rect.width(), rect.height()) / 2.0;
    QPointF points[numPoints];
    
    // 从顶部开始，顺时针创建五个点
    for (int i = 0; i < numPoints; ++i) {
        // 计算角度: 从-90度开始（顶部），顺时针旋转
        qreal angle = -M_PI_2 + i * 2 * M_PI / numPoints;
        points[i] = center + QPointF(radius * std::cos(angle), radius * std::sin(angle));
    }
    
    // 构建五边形路径
    path.moveTo(points[0]);
    for (int i = 1; i < numPoints; ++i) {
        path.lineTo(points[i]);
    }
    path.closeSubpath();
    
    // 绘制五边形
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

bool Pentagon::hitTest(const QPointF& pt) const
{
    // 创建五边形路径用于命中测试
    QPainterPath path;
    QRectF rect = bounds;
    QPointF center = rect.center();
    
    const int numPoints = 5;
    const qreal radius = qMin(rect.width(), rect.height()) / 2.0;
    QPointF points[numPoints];
    
    for (int i = 0; i < numPoints; ++i) {
        qreal angle = -M_PI_2 + i * 2 * M_PI / numPoints;
        points[i] = center + QPointF(radius * std::cos(angle), radius * std::sin(angle));
    }
    
    path.moveTo(points[0]);
    for (int i = 1; i < numPoints; ++i) {
        path.lineTo(points[i]);
    }
    path.closeSubpath();
    
    return path.contains(pt);
}

QPointF Pentagon::getConnectionPoint(const QPointF& ref) const
{
    QRectF rect = bounds;
    QPointF center = rect.center();
    
    // 计算五边形的顶点
    const int numPoints = 5;
    const qreal radius = qMin(rect.width(), rect.height()) / 2.0;
    QPointF points[numPoints];
    
    for (int i = 0; i < numPoints; ++i) {
        qreal angle = -M_PI_2 + i * 2 * M_PI / numPoints;
        points[i] = center + QPointF(radius * std::cos(angle), radius * std::sin(angle));
    }
    
    // 计算参考点到五边形中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 创建五边形的五条边
    QLineF edges[numPoints];
    for (int i = 0; i < numPoints; ++i) {
        edges[i] = QLineF(points[i], points[(i + 1) % numPoints]);
    }
    
    // 从中心点向外射线
    QLineF ray(center, center + direction * 1000.0); // 足够长以确保与边相交
    
    // 检查与哪条边相交，选择最近的交点
    QPointF intersection;
    QPointF bestIntersection;
    qreal bestDistance = std::numeric_limits<qreal>::max();
    
    // 检查与所有边的交点
    for (int i = 0; i < numPoints; ++i) {
        if (ray.intersect(edges[i], &intersection) == QLineF::BoundedIntersection) {
            qreal distance = QLineF(center, intersection).length();
            if (distance < bestDistance) {
                bestDistance = distance;
                bestIntersection = intersection;
            }
        }
    }
    
    // 如果找到交点，返回它；否则使用默认实现
    if (bestDistance < std::numeric_limits<qreal>::max()) {
        return bestIntersection;
    }
    
    // 如果没有找到交点（这是一个安全措施），使用五个顶点中最近的点
    QPointF bestVertex = points[0];
    bestDistance = QLineF(ref, bestVertex).length();
    
    for (int i = 1; i < numPoints; ++i) {
        qreal distance = QLineF(ref, points[i]).length();
        if (distance < bestDistance) {
            bestDistance = distance;
            bestVertex = points[i];
        }
    }
    
    return bestVertex;
}

QJsonObject Pentagon::toJson() const
{
    return QJsonObject{
        {"type", "pentagon"},
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

void Pentagon::fromJson(const QJsonObject& o)
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