#pragma once
#include "Shape.hpp"
#include <QJsonObject>

class Ellipse final : public Shape
{
public:
    void paint(QPainter& p, bool selected) const override;
    bool hitTest(const QPointF& pt) const override;

    QJsonObject toJson()  const override;
    void fromJson(const QJsonObject&) override;
};
