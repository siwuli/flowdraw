#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

// 文本编辑对话框，用于为图形元素添加/编辑文本
class TextEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TextEditDialog(QWidget* parent = nullptr, const QString& initialText = "");
    
    // 获取编辑后的文本
    QString getText() const;

private:
    QTextEdit* textEdit_;
    QPushButton* okButton_;
    QPushButton* cancelButton_;
};
