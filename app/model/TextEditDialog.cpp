#include "TextEditDialog.hpp"
#include <QHBoxLayout>

TextEditDialog::TextEditDialog(QWidget* parent, const QString& initialText)
    : QDialog(parent)
{
    setWindowTitle(tr("Edit Text"));
    setMinimumSize(300, 200);

    auto* mainLayout = new QVBoxLayout(this);
    
    // 创建文本编辑框
    textEdit_ = new QTextEdit(this);
    textEdit_->setText(initialText);
    textEdit_->setPlaceholderText(tr("Enter text here..."));
    
    // 创建按钮
    auto* buttonLayout = new QHBoxLayout();
    okButton_ = new QPushButton(tr("OK"), this);
    cancelButton_ = new QPushButton(tr("Cancel"), this);
    
    // 连接信号和槽
    connect(okButton_, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton_, &QPushButton::clicked, this, &QDialog::reject);
    
    // 设置布局
    buttonLayout->addStretch();
    buttonLayout->addWidget(okButton_);
    buttonLayout->addWidget(cancelButton_);
    
    mainLayout->addWidget(new QLabel(tr("Text content:"), this));
    mainLayout->addWidget(textEdit_);
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

QString TextEditDialog::getText() const
{
    return textEdit_->toPlainText();
}
