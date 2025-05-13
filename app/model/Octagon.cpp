#include "Octagon.hpp"
#include <QPainterPath>
#include <QtMath>
#include <limits>

void Octagon::paint(QPainter& p, bool selected) const
{
    // 创建八边形路径
    QPainterPath path;
    QRectF rect = bounds;
    QPointF center = rect.center();
    
    // 计算八边形的八个顶点，基于矩形边界
    // 按照水平布局的、可压缩的八边形
    qreal wStep = rect.width() / 4.0;
    qreal hStep = rect.height() / 4.0;
    
    QPointF leftTop(rect.left(), rect.top() + hStep);
    QPointF topLeft(rect.left() + wStep, rect.top());
    QPointF topRight(rect.right() - wStep, rect.top());
    QPointF rightTop(rect.right(), rect.top() + hStep);
    QPointF rightBottom(rect.right(), rect.bottom() - hStep);
    QPointF bottomRight(rect.right() - wStep, rect.bottom());
    QPointF bottomLeft(rect.left() + wStep, rect.bottom());
    QPointF leftBottom(rect.left(), rect.bottom() - hStep);
    
    // 构建八边形路径
    path.moveTo(leftTop);
    path.lineTo(topLeft);
    path.lineTo(topRight);
    path.lineTo(rightTop);
    path.lineTo(rightBottom);
    path.lineTo(bottomRight);
    path.lineTo(bottomLeft);
    path.lineTo(leftBottom);
    path.closeSubpath();
    
    // 绘制八边形
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

bool Octagon::hitTest(const QPointF& pt) const
{
    // 创建八边形路径用于命中测试
    QPainterPath path;
    QRectF rect = bounds;
    
    // 计算八边形的八个顶点
    qreal wStep = rect.width() / 4.0;
    qreal hStep = rect.height() / 4.0;
    
    QPointF leftTop(rect.left(), rect.top() + hStep);
    QPointF topLeft(rect.left() + wStep, rect.top());
    QPointF topRight(rect.right() - wStep, rect.top());
    QPointF rightTop(rect.right(), rect.top() + hStep);
    QPointF rightBottom(rect.right(), rect.bottom() - hStep);
    QPointF bottomRight(rect.right() - wStep, rect.bottom());
    QPointF bottomLeft(rect.left() + wStep, rect.bottom());
    QPointF leftBottom(rect.left(), rect.bottom() - hStep);
    
    path.moveTo(leftTop);
    path.lineTo(topLeft);
    path.lineTo(topRight);
    path.lineTo(rightTop);
    path.lineTo(rightBottom);
    path.lineTo(bottomRight);
    path.lineTo(bottomLeft);
    path.lineTo(leftBottom);
    path.closeSubpath();
    
    return path.contains(pt);
}

QPointF Octagon::getConnectionPoint(const QPointF& ref) const
{
    QRectF rect = bounds;
    QPointF center = rect.center();
    
    // 计算八边形的八个顶点
    qreal wStep = rect.width() / 4.0;
    qreal hStep = rect.height() / 4.0;
    
    QPointF leftTop(rect.left(), rect.top() + hStep);
    QPointF topLeft(rect.left() + wStep, rect.top());
    QPointF topRight(rect.right() - wStep, rect.top());
    QPointF rightTop(rect.right(), rect.top() + hStep);
    QPointF rightBottom(rect.right(), rect.bottom() - hStep);
    QPointF bottomRight(rect.right() - wStep, rect.bottom());
    QPointF bottomLeft(rect.left() + wStep, rect.bottom());
    QPointF leftBottom(rect.left(), rect.bottom() - hStep);
    
    // 八边形顶点数组
    const int numPoints = 8;
    QPointF points[numPoints] = {
        leftTop, topLeft, topRight, rightTop, 
        rightBottom, bottomRight, bottomLeft, leftBottom
    };
    
    // 计算参考点到八边形中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 创建八边形的八条边
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
    
    // 如果没有找到交点（这是一个安全措施），使用八个顶点中最近的点
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

QJsonObject Octagon::toJson() const
{
    return QJsonObject{
        {"type", "octagon"},
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

void Octagon::fromJson(const QJsonObject& o)
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