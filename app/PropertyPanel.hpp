#pragma once
#include <QWidget>
#include <QColor>

class QSpinBox;
class QPushButton;
class QLineEdit;
class QFontComboBox;

class PropertyPanel : public QWidget
{
    Q_OBJECT
public:
    explicit PropertyPanel(QWidget* parent = nullptr);

    // 加载当前选中图形的属性到面板
    void load(const QColor& fill, const QColor& stroke, qreal width);
    
    // 加载文本属性
    void loadText(const QString& text, const QColor& textColor, int fontSize);

signals:
    void fillChanged(const QColor&);
    void strokeChanged(const QColor&);
    void widthChanged(qreal);
    void textChanged(const QString&);
    void textColorChanged(const QColor&);
    void textSizeChanged(int);

private:
    QPushButton* btnFill_;
    QPushButton* btnStroke_;
    QSpinBox* spinWidth_;
    
    QPushButton* btnTextColor_;
    QSpinBox* spinTextSize_;
};
