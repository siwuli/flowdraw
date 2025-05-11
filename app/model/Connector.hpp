#pragma once
#include <QPainter>
#include "Shape.hpp"

/* ��ֱ������������㡢�յ�ֱ�ָ��ͼ�Σ� */
class Connector
{
public:
    Shape* src = nullptr;   // ���ͼ��
    Shape* dst = nullptr;   // �յ�ͼ��
    QPointF tempEnd;        // ���ƹ����е���ʱ�յ�

    void paint(QPainter& p) const;

private:
    QPointF anchorPoint(const Shape* s, const QPointF& ref) const;
    void    drawArrow(QPainter& p, const QPointF& from, const QPointF& to) const;
};
