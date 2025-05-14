#include "RectTriangle.hpp"
#include <QPainterPath>
#include <cmath>

// 在Windows平台上定义M_PI（如果尚未定义）
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void RectTriangle::paint(QPainter& p, bool selected) const
{
    // 绘制直角三角形（左下角为直角）
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    
    // 创建路径
    QPainterPath path;
    
    // 三角形三个点：左下角、右下角、左上角
    path.moveTo(bounds.bottomLeft());
    path.lineTo(bounds.bottomRight());
    path.lineTo(bounds.topLeft());
    path.closeSubpath();
    
    p.drawPath(path);
    
    // 绘制文本
    drawText(p);
    
    // 如果被选中，绘制虚线框，适应形状
    if (selected) {
        QPen dashPen(Qt::DashLine);
        dashPen.setColor(Qt::blue);
        p.setPen(dashPen);
        p.setBrush(Qt::NoBrush);
        
        // 使用稍微放大的路径绘制选中框
        QPainterPath selectionPath = path;
        QTransform transform;
        transform.translate(bounds.center().x(), bounds.center().y());
        transform.scale(1.04, 1.04); // 比实际形状稍大
        transform.translate(-bounds.center().x(), -bounds.center().y());
        selectionPath = transform.map(selectionPath);
        
        p.drawPath(selectionPath);
    }
}

bool RectTriangle::hitTest(const QPointF& pt) const
{
    // 判断点是否在形状内
    if (!bounds.contains(pt)) {
        return false;  // 如果不在外围矩形内，直接返回false
    }
    
    QPainterPath path;
    
    // 三角形三个点：左下角、右下角、左上角
    path.moveTo(bounds.bottomLeft());
    path.lineTo(bounds.bottomRight());
    path.lineTo(bounds.topLeft());
    path.closeSubpath();
    
    return path.contains(pt);
}

QPointF RectTriangle::getConnectionPoint(const QPointF& ref) const
{
    // 获取形状的中心点
    QPointF center = bounds.center();
    
    // 计算参考点到中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 判断连接方向
    bool fromLeft = ref.x() < bounds.left();
    bool fromTop = ref.y() < bounds.top();
    bool fromRight = ref.x() > bounds.right();
    bool fromBottom = ref.y() > bounds.bottom();
    
    // 如果是从左侧连接，确保连接点在左侧垂直边上
    if (fromLeft && !fromTop && !fromBottom) {
        // 从左侧连接，返回左侧边上的点
        qreal yRatio = (ref.y() - bounds.top()) / bounds.height();
        qreal y = bounds.top() + bounds.height() * yRatio;
        
        // 确保y坐标在有效范围内
        y = qMax(bounds.top(), qMin(y, bounds.bottom()));
        
        return QPointF(bounds.left(), y);
    }
    
    // 归一化方向向量
    qreal length = std::sqrt(direction.x() * direction.x() + direction.y() * direction.y());
    QPointF unitDir = direction / length;
    
    // 计算从中心点到边缘的射线与形状轮廓的交点
    QPointF edgePoint;
    QLineF ray(center, center + unitDir * qMax(bounds.width(), bounds.height()) * 2.0);
    
    // 定义三角形的边
    QLineF edges[3] = {
        QLineF(bounds.bottomLeft(), bounds.bottomRight()),  // 底边
        QLineF(bounds.bottomRight(), bounds.topLeft()),     // 斜边
        QLineF(bounds.topLeft(), bounds.bottomLeft())       // 左边
    };
    
    // 查找射线与哪个边相交
    for (const QLineF& edge : edges) {
        QPointF intersection;
        if (ray.intersect(edge, &intersection) == QLineF::BoundedIntersection) {
            return intersection;
        }
    }
    
    // 如果没有找到交点（几乎不可能），返回中心点
    return center;
}

QJsonObject RectTriangle::toJson() const
{
    return QJsonObject{
        {"type", "recttriangle"},
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

void RectTriangle::fromJson(const QJsonObject& o)
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