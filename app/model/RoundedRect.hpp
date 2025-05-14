#pragma once
#include "Shape.hpp"

class RoundedRect final : public Shape
{
public:
    RoundedRect() : cornerRadius_(10) {}  // 默认圆角半径为10
    
    void paint(QPainter& p, bool selected) const override;
    bool hitTest(const QPointF& pt) const override;
    QPointF getConnectionPoint(const QPointF& ref) const override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;
    
    // 圆角半径
    qreal cornerRadius() const { return cornerRadius_; }
    void setCornerRadius(qreal radius) { cornerRadius_ = qMax(0.0, radius); }
    
private:
    qreal cornerRadius_;  // 圆角半径
}; 