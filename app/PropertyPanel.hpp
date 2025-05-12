#pragma once
#include <QWidget>
#include <QColor>

class QSpinBox;
class QPushButton;

class PropertyPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyPanel(QWidget* parent = nullptr);

    // FlowView 调用：把当前选中图形属性灌入面板
    void load(const QColor& fill, const QColor& stroke, qreal width);

signals:
    void fillChanged(const QColor&);
    void strokeChanged(const QColor&);
    void widthChanged(qreal);

private:
    QPushButton* btnFill_;
    QPushButton* btnStroke_;
    QSpinBox* spinWidth_;
};
