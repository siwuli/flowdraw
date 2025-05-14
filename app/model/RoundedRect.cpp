#include "RoundedRect.hpp"
#include <QPainterPath>

void RoundedRect::paint(QPainter& p, bool selected) const
{
    // 绘制圆角矩形
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    p.drawRoundedRect(bounds, cornerRadius_, cornerRadius_);
    
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

bool RoundedRect::hitTest(const QPointF& pt) const
{
    // 判断点是否在矩形内
    // 对于简单的情况，可以直接使用矩形包含测试
    // 对于更精确的圆角测试，可以使用QPainterPath
    
    if (!bounds.contains(pt)) {
        return false;  // 如果不在外围矩形内，直接返回false
    }
    
    // 检查点是否在圆角区域外
    // 只需检查四个角
    double r = cornerRadius_;
    
    // 左上角
    if (pt.x() < bounds.left() + r && pt.y() < bounds.top() + r) {
        QPointF center(bounds.left() + r, bounds.top() + r);
        if (QLineF(center, pt).length() > r) {
            return false;
        }
    }
    
    // 右上角
    if (pt.x() > bounds.right() - r && pt.y() < bounds.top() + r) {
        QPointF center(bounds.right() - r, bounds.top() + r);
        if (QLineF(center, pt).length() > r) {
            return false;
        }
    }
    
    // 左下角
    if (pt.x() < bounds.left() + r && pt.y() > bounds.bottom() - r) {
        QPointF center(bounds.left() + r, bounds.bottom() - r);
        if (QLineF(center, pt).length() > r) {
            return false;
        }
    }
    
    // 右下角
    if (pt.x() > bounds.right() - r && pt.y() > bounds.bottom() - r) {
        QPointF center(bounds.right() - r, bounds.bottom() - r);
        if (QLineF(center, pt).length() > r) {
            return false;
        }
    }
    
    return true;
}

QPointF RoundedRect::getConnectionPoint(const QPointF& ref) const
{
    // 获取矩形的中心点
    QPointF center = bounds.center();
    
    // 计算参考点到矩形中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 创建一个QPainterPath来表示圆角矩形
    QPainterPath path;
    path.addRoundedRect(bounds, cornerRadius_, cornerRadius_);
    
    // 从中心点向参考点方向延伸一条射线
    // 需要确保射线足够长，以确保与矩形边界相交
    double diagonal = QLineF(bounds.topLeft(), bounds.bottomRight()).length();
    QPointF farPoint = center + direction * (diagonal / direction.manhattanLength());
    
    // 在路径上寻找交点
    QLineF ray(center, farPoint);
    
    // 矩形的四条边
    double left = bounds.left();
    double right = bounds.right();
    double top = bounds.top();
    double bottom = bounds.bottom();
    double r = cornerRadius_;
    
    // 考虑圆角，我们需要检查线段与直线和圆弧的交点
    double dx = direction.x();
    double dy = direction.y();
    
    // 初始化四个可能的交点位置参数
    double tx1 = dx != 0 ? (left - center.x()) / dx : -1;
    double tx2 = dx != 0 ? (right - center.x()) / dx : -1;
    double ty1 = dy != 0 ? (top - center.y()) / dy : -1;
    double ty2 = dy != 0 ? (bottom - center.y()) / dy : -1;
    
    // 计算可能的交点
    QList<QPointF> intersections;
    
    // 检查与四条直边的交点
    if (tx1 > 0) {
        double y = center.y() + tx1 * dy;
        if (y >= top + r && y <= bottom - r) {
            intersections.append(QPointF(left, y));
        }
    }
    
    if (tx2 > 0) {
        double y = center.y() + tx2 * dy;
        if (y >= top + r && y <= bottom - r) {
            intersections.append(QPointF(right, y));
        }
    }
    
    if (ty1 > 0) {
        double x = center.x() + ty1 * dx;
        if (x >= left + r && x <= right - r) {
            intersections.append(QPointF(x, top));
        }
    }
    
    if (ty2 > 0) {
        double x = center.x() + ty2 * dx;
        if (x >= left + r && x <= right - r) {
            intersections.append(QPointF(x, bottom));
        }
    }
    
    // 检查与四个圆角的交点
    // 这里简化处理，使用常规的矩形连接点
    if (intersections.isEmpty()) {
        // 如果没有与直边相交，使用默认的连接点实现
        return Shape::getConnectionPoint(ref);
    }
    
    // 找到最近的交点
    QPointF nearest = intersections[0];
    double minDist = QLineF(center, nearest).length();
    
    for (int i = 1; i < intersections.size(); ++i) {
        double dist = QLineF(center, intersections[i]).length();
        if (dist < minDist) {
            minDist = dist;
            nearest = intersections[i];
        }
    }
    
    return nearest;
}

QJsonObject RoundedRect::toJson() const
{
    return QJsonObject{
        {"type", "roundedrect"},
        {"x", bounds.x()}, {"y", bounds.y()},
        {"w", bounds.width()}, {"h", bounds.height()},
        {"fill", fillColor.name(QColor::HexArgb)},
        {"stroke", strokeColor.name(QColor::HexArgb)},
        {"width", strokeWidth},
        {"text", text},
        {"textColor", textColor.name(QColor::HexArgb)},
        {"textSize", textSize},
        {"cornerRadius", cornerRadius_}
    };
}

void RoundedRect::fromJson(const QJsonObject& o)
{
    bounds = { o["x"].toDouble(), o["y"].toDouble(),
               o["w"].toDouble(), o["h"].toDouble() };
    fillColor = QColor(o["fill"].toString("#ffffffff"));
    strokeColor = QColor(o["stroke"].toString("#ff000000"));
    strokeWidth = o["width"].toDouble(1.5);
    text = o["text"].toString();
    textColor = QColor(o["textColor"].toString("#ff000000"));
    textSize = o["textSize"].toInt(10);
    cornerRadius_ = o["cornerRadius"].toDouble(10);
} 