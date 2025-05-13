#pragma once
#include "Shape.hpp"

class Triangle final : public Shape
{
public:
    void paint(QPainter& p, bool selected) const override;
    bool hitTest(const QPointF& pt) const override;
    QPointF getConnectionPoint(const QPointF& ref) const override;

    QJsonObject toJson() const override;
    void fromJson(const QJsonObject&) override;
}; 