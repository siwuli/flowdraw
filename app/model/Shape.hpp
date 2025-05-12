#pragma once
#include <QPainter>
#include <QRectF>
#include <QJsonObject>

/* ������ࣺ���пɻ���Ԫ�صĹ����ӿ� */
class Shape
{
public:

    virtual ~Shape() = default;

    // ��������
    virtual void paint(QPainter& p, bool selected) const = 0;
    // ���в��ԣ��ж� pt �Ƿ�������״��
    virtual bool hitTest(const QPointF& pt) const = 0;
   
    // �����麯��
    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject&) = 0;

    QRectF bounds;   // ��Χ�У������ƶ������л�
    QColor  fillColor = Qt::white;   
    QColor  strokeColor = Qt::black;   
    qreal   strokeWidth = 1.5;
};
