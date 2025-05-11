#pragma once
#include "Shape.hpp"

class Rect final : public Shape
{
public:
    void paint(QPainter& p, bool selected) const override;
    bool hitTest(const QPointF& pt) const override;

    QJsonObject toJson() const override;        // ¡û ÉùÃ÷
    void fromJson(const QJsonObject&) override;
};
