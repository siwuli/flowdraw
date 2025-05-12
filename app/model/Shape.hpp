#pragma once
#include <QPainter>
#include <QRectF>
#include <QJsonObject>

/* 抽象基类：所有可绘制元素的公共接口 */
class Shape
{
public:

    virtual ~Shape() = default;

    // 绘制自身
    virtual void paint(QPainter& p, bool selected) const = 0;
    // 命中测试：判断 pt 是否落在形状内
    virtual bool hitTest(const QPointF& pt) const = 0;
   
    // 增加虚函数
    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject&) = 0;

    QRectF bounds;   // 包围盒，用于移动与序列化
    QColor  fillColor = Qt::white;   
    QColor  strokeColor = Qt::black;   
    qreal   strokeWidth = 1.5;
};
