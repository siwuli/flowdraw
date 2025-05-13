#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QColorDialog>
#include <QSpinBox>
#include <QHBoxLayout>

// 文本编辑对话框，用于为图形元素添加/编辑文本
class TextEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextEditDialog(QWidget* parent = nullptr, const QString& initialText = "");
    
    // 获取编辑后的文本
    QString getText() const;
    // 获取文本颜色
    QColor getTextColor() const { return textColor_; }
    // 获取文本大小
    int getTextSize() const { return textSize_; }
    
    // 设置初始文本
    void setText(const QString& text);
    // 设置文本颜色
    void setTextColor(const QColor& color);
    // 设置文本大小
    void setTextSize(int size);

private slots:
    void selectColor();

private:
    // 更新颜色按钮的外观
    void updateColorButton();
    
    QTextEdit* textEdit_;
    QPushButton* colorButton_;
    QSpinBox* sizeSpinBox_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
    QColor textColor_ = Qt::black;
    int textSize_ = 12;
};
