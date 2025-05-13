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
    
    // 加载对象尺寸属性
    void loadSize(int width, int height);

public slots:
    // 加载文本颜色
    void updateTextColor(const QColor& color);
    // 加载文本大小
    void updateTextSize(int size);

signals:
    void fillChanged(const QColor&);
    void strokeChanged(const QColor&);
    void widthChanged(qreal);
    void textChanged(const QString&);
    void textColorChanged(const QColor&);
    void textSizeChanged(int);
    // 对象尺寸变化信号
    void objectWidthChanged(int);
    void objectHeightChanged(int);

private:
    QPushButton* btnFill_;
    QPushButton* btnStroke_;
    QSpinBox* spinWidth_;
    
    QPushButton* btnTextColor_;
    QSpinBox* spinTextSize_;
    
    // 对象尺寸微调控件
    QSpinBox* spinObjectWidth_;
    QSpinBox* spinObjectHeight_;
};
