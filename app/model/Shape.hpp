#pragma once
#include <QPainter>
#include <QRectF>
#include <QJsonObject>
#include <QString>

/* 基类：所有可绘制元素的公共接口 */
class Shape
{
public:

    virtual ~Shape() = default;

    // 绘制函数
    virtual void paint(QPainter& p, bool selected) const = 0;
    // 碰撞测试，判断 pt 是否在形状内
    virtual bool hitTest(const QPointF& pt) const = 0;
   
    // 序列化函数
    virtual QJsonObject toJson() const = 0;
    virtual void fromJson(const QJsonObject&) = 0;
    
    // 获取连接点方法
    virtual QPointF getConnectionPoint(const QPointF& ref) const {
        // 默认实现：使用矩形边框的中点
        const QRectF& b = bounds;
        QPointF mids[4] = {
            {(b.left() + b.right()) / 2, b.top()},
            {b.right(), (b.top() + b.bottom()) / 2},
            {(b.left() + b.right()) / 2, b.bottom()},
            {b.left(), (b.top() + b.bottom()) / 2}
        };
        QPointF best = mids[0];
        qreal bestDist = QLineF(ref, best).length();
        for (int i = 1; i < 4; ++i) {
            qreal d = QLineF(ref, mids[i]).length();
            if (d < bestDist) { best = mids[i]; bestDist = d; }
        }
        return best;
    }

    QRectF bounds;   // 外围框，用于移动和选中
    QColor  fillColor = Qt::white;   
    QColor  strokeColor = Qt::black;   
    qreal   strokeWidth = 1.5;
    
    // 添加文本相关属性
    QString text;
    QColor textColor = Qt::black;
    int textSize = 10;
    
    // 绘制文本的辅助函数
    void drawText(QPainter& p) const {
        if (!text.isEmpty()) {
            QFont font;
            font.setPointSize(textSize);
            p.setFont(font);
            p.setPen(textColor);
            p.drawText(bounds, Qt::AlignCenter, text);
        }
    }
};
