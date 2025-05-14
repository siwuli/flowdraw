#pragma once
#include "Shape.hpp"

// 扇形类
class Sector : public Shape {
public:
    // 实现虚函数
    void paint(QPainter& p, bool selected) const override;
    bool hitTest(const QPointF& pt) const override;
    QPointF getConnectionPoint(const QPointF& ref) const override;
    QJsonObject toJson() const override;
    void fromJson(const QJsonObject& o) override;
    
    // 扇形的角度范围
    qreal startAngle = 0;  // 开始角度（度）
    qreal spanAngle = 90;  // 扇形跨度（度）
}; 