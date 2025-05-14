#pragma once
#include "Shape.hpp"

// 圆环类
class Arc : public Shape {
public:
    // 实现虚函数
    void paint(QPainter& p, bool selected) const override;
    bool hitTest(const QPointF& pt) const override;
    QPointF getConnectionPoint(const QPointF& ref) const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& o) override;
    
    // 圆环的参数
    qreal startAngle = 0;     // 开始角度（度）
    qreal spanAngle = 90;     // 圆环跨度（度）
    qreal thickness = 20;     // 圆环厚度（像素）
}; 