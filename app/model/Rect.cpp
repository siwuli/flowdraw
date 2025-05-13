#include "Rect.hpp"

void Rect::paint(QPainter& p, bool selected) const
{
    // 绘制矩形
    QPen pen(strokeColor, strokeWidth);
    p.setPen(pen);
    p.setBrush(fillColor);
    p.drawRect(bounds);
    
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

bool Rect::hitTest(const QPointF& pt) const
{
    return bounds.contains(pt);
}

QPointF Rect::getConnectionPoint(const QPointF& ref) const
{
    // 获取矩形的中心点
    QPointF center = bounds.center();
    
    // 计算参考点到矩形中心的方向向量
    QPointF direction = ref - center;
    
    // 如果方向向量为零，直接返回中心点
    if (direction.isNull()) {
        return center;
    }
    
    // 矩形的四条边
    double left = bounds.left();
    double right = bounds.right();
    double top = bounds.top();
    double bottom = bounds.bottom();
    
    // 寻找与射线相交的边
    // 计算交点比例
    double dx = direction.x();
    double dy = direction.y();
    
    // 初始化四个可能的交点位置参数
    double tx1 = dx != 0 ? (left - center.x()) / dx : -1;
    double tx2 = dx != 0 ? (right - center.x()) / dx : -1;
    double ty1 = dy != 0 ? (top - center.y()) / dy : -1;
    double ty2 = dy != 0 ? (bottom - center.y()) / dy : -1;
    
    // 筛选出有效的交点位置参数（大于0表示从中心点向外）
    QList<double> validT;
    if (tx1 > 0) validT.append(tx1);
    if (tx2 > 0) validT.append(tx2);
    if (ty1 > 0) validT.append(ty1);
    if (ty2 > 0) validT.append(ty2);
    
    // 取最小的有效交点位置参数，即最近的交点
    if (!validT.isEmpty()) {
        double tMin = *std::min_element(validT.begin(), validT.end());
        return center + direction * tMin;
    }
    
    // 如果没有有效交点（不应该发生），返回最近的边缘点
    QPointF nearestPoint = center;
    if (qAbs(direction.x()) > qAbs(direction.y())) {
        // 水平方向为主
        nearestPoint.setX(direction.x() > 0 ? right : left);
    } else {
        // 垂直方向为主
        nearestPoint.setY(direction.y() > 0 ? bottom : top);
    }
    
    return nearestPoint;
}

QJsonObject Rect::toJson() const
{
    return QJsonObject{
        {"type","rect"},
        {"x",bounds.x()}, {"y",bounds.y()},
        {"w",bounds.width()}, {"h",bounds.height()},
        {"fill",   fillColor.name(QColor::HexArgb)},
        {"stroke", strokeColor.name(QColor::HexArgb)},
        {"width",  strokeWidth},
        {"text", text},
        {"textColor", textColor.name(QColor::HexArgb)},
        {"textSize", textSize}
    };
}

void Rect::fromJson(const QJsonObject& o)
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

