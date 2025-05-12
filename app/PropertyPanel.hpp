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

    // FlowView ���ã��ѵ�ǰѡ��ͼ�����Թ������
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
