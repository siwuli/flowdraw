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
    
    // 创建文本样式控制区域
    auto* styleLayout = new QHBoxLayout();
    
    // 颜色选择按钮
    auto* colorLabel = new QLabel(tr("Color:"), this);
    colorButton_ = new QPushButton(this);
    colorButton_->setFixedSize(30, 20);
    updateColorButton();
    connect(colorButton_, &QPushButton::clicked, this, &TextEditDialog::selectColor);
    
    // 大小选择
    auto* sizeLabel = new QLabel(tr("Size:"), this);
    sizeSpinBox_ = new QSpinBox(this);
    sizeSpinBox_->setRange(6, 72);
    sizeSpinBox_->setValue(textSize_);
    
    styleLayout->addWidget(colorLabel);
    styleLayout->addWidget(colorButton_);
    styleLayout->addSpacing(10);
    styleLayout->addWidget(sizeLabel);
    styleLayout->addWidget(sizeSpinBox_);
    styleLayout->addStretch();
    
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
    mainLayout->addLayout(styleLayout);
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

QString TextEditDialog::getText() const
{
    return textEdit_->toPlainText();
}

void TextEditDialog::setText(const QString& text)
{
    textEdit_->setText(text);
}

void TextEditDialog::setTextColor(const QColor& color)
{
    textColor_ = color;
    updateColorButton();
}

void TextEditDialog::setTextSize(int size)
{
    textSize_ = size;
    sizeSpinBox_->setValue(size);
}

void TextEditDialog::selectColor()
{
    QColor newColor = QColorDialog::getColor(textColor_, this, tr("Select Text Color"));
    if (newColor.isValid()) {
        textColor_ = newColor;
        updateColorButton();
    }
}

void TextEditDialog::updateColorButton()
{
    // 设置颜色按钮的背景色
    QString styleSheet = QString("background-color: %1;").arg(textColor_.name());
    colorButton_->setStyleSheet(styleSheet);
}
