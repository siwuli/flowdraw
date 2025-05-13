#include "PropertyPanel.hpp"
#include <QFormLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QSpinBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QFontComboBox>
#include <QLabel>

static void setColorButton(QPushButton* btn, const QColor& c)
{
    QString css = c.isValid()
        ? QString("background:%1").arg(c.name())
        : "background:lightgray";
    btn->setStyleSheet(css);
}


PropertyPanel::PropertyPanel(QWidget* parent) : QWidget(parent)
{
    auto* mainLayout = new QVBoxLayout(this);
    
    // 形状属性组
    auto* shapeGroup = new QGroupBox(tr("Shape Properties"), this);
    auto* shapeLay = new QFormLayout(shapeGroup);

    btnFill_ = new QPushButton;  btnFill_->setAutoFillBackground(true);
    btnStroke_ = new QPushButton;  btnStroke_->setAutoFillBackground(true);
    spinWidth_ = new QSpinBox;     spinWidth_->setRange(1, 10);

    shapeLay->addRow(tr("Fill Color"), btnFill_);
    shapeLay->addRow(tr("Stroke Color"), btnStroke_);
    shapeLay->addRow(tr("Line Width"), spinWidth_);
    
    // 尺寸属性组
    auto* sizeGroup = new QGroupBox(tr("Size Properties"), this);
    auto* sizeLay = new QFormLayout(sizeGroup);
    
    spinObjectWidth_ = new QSpinBox; spinObjectWidth_->setRange(10, 1000);
    spinObjectHeight_ = new QSpinBox; spinObjectHeight_->setRange(10, 1000);
    
    sizeLay->addRow(tr("Width"), spinObjectWidth_);
    sizeLay->addRow(tr("Height"), spinObjectHeight_);
    
    // 文本属性组
    auto* textGroup = new QGroupBox(tr("Text Properties"), this);
    auto* textLay = new QFormLayout(textGroup);
    
    btnTextColor_ = new QPushButton;  btnTextColor_->setAutoFillBackground(true);
    spinTextSize_ = new QSpinBox;     spinTextSize_->setRange(8, 36);
    
    textLay->addRow(tr("Text Color"), btnTextColor_);
    textLay->addRow(tr("Font Size"), spinTextSize_);
    textLay->addRow(new QLabel(tr("Tip: Double-click shape to edit text"), this));
    
    // 连接线属性组
    auto* connectorGroup = new QGroupBox(tr("Connector Properties"), this);
    auto* connectorLay = new QFormLayout(connectorGroup);
    
    btnConnectorColor_ = new QPushButton; btnConnectorColor_->setAutoFillBackground(true);
    connectorLay->addRow(tr("Connector Color"), btnConnectorColor_);
    
    // 添加到主布局
    mainLayout->addWidget(shapeGroup);
    mainLayout->addWidget(sizeGroup);
    mainLayout->addWidget(textGroup);
    mainLayout->addWidget(connectorGroup);
    mainLayout->addStretch();
    
    // 形状属性信号连接
    connect(btnFill_, &QPushButton::clicked, this, [this] {
        QColor c = QColorDialog::getColor(btnFill_->palette().button().color(), this);
        if (c.isValid()) emit fillChanged(c);
    });
    connect(btnStroke_, &QPushButton::clicked, this, [this] {
        QColor c = QColorDialog::getColor(btnStroke_->palette().button().color(), this);
        if (c.isValid()) emit strokeChanged(c);
    });
    connect(spinWidth_, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int v) { emit widthChanged(v); });
    
    // 尺寸属性信号连接
    connect(spinObjectWidth_, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int v) { emit objectWidthChanged(v); });
    connect(spinObjectHeight_, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int v) { emit objectHeightChanged(v); });
        
    // 文本属性信号连接
    connect(btnTextColor_, &QPushButton::clicked, this, [this] {
        QColor c = QColorDialog::getColor(btnTextColor_->palette().button().color(), this);
        if (c.isValid()) emit textColorChanged(c);
    });
    connect(spinTextSize_, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int v) { emit textSizeChanged(v); });
        
    // 连接线属性信号连接
    connect(btnConnectorColor_, &QPushButton::clicked, this, [this] {
        QColor c = QColorDialog::getColor(btnConnectorColor_->palette().button().color(), this);
        if (c.isValid()) emit connectorColorChanged(c);
    });
}

void PropertyPanel::load(const QColor& fill, const QColor& stroke, qreal width)
{
    bool hasSel = width >= 0;           // width<0 表示当前无选中

    btnFill_->setEnabled(hasSel);
    btnStroke_->setEnabled(hasSel);
    spinWidth_->setEnabled(hasSel);
    btnTextColor_->setEnabled(hasSel);
    spinTextSize_->setEnabled(hasSel);
    spinObjectWidth_->setEnabled(hasSel);
    spinObjectHeight_->setEnabled(hasSel);
    btnConnectorColor_->setEnabled(false);  // 默认禁用连接线颜色按钮

    if (!hasSel) {                       // 灰度显示
        setColorButton(btnFill_, {});
        setColorButton(btnStroke_, {});
        spinWidth_->blockSignals(true);
        spinWidth_->setValue(1);
        spinWidth_->blockSignals(false);
        
        setColorButton(btnTextColor_, {});
        spinTextSize_->blockSignals(true);
        spinTextSize_->setValue(10);
        spinTextSize_->blockSignals(false);
        
        spinObjectWidth_->blockSignals(true);
        spinObjectWidth_->setValue(100);
        spinObjectWidth_->blockSignals(false);
        
        spinObjectHeight_->blockSignals(true);
        spinObjectHeight_->setValue(100);
        spinObjectHeight_->blockSignals(false);
        
        setColorButton(btnConnectorColor_, {});
        return;
    }

    setColorButton(btnFill_, fill);
    setColorButton(btnStroke_, stroke);
    spinWidth_->blockSignals(true);
    spinWidth_->setValue(int(width));
    spinWidth_->blockSignals(false);
}

void PropertyPanel::loadText(const QString& text, const QColor& textColor, int fontSize)
{
    bool hasText = !text.isEmpty();
    
    setColorButton(btnTextColor_, textColor.isValid() ? textColor : Qt::black);
    spinTextSize_->blockSignals(true);
    spinTextSize_->setValue(fontSize > 0 ? fontSize : 10);
    spinTextSize_->blockSignals(false);
}

void PropertyPanel::loadSize(int width, int height)
{
    spinObjectWidth_->blockSignals(true);
    spinObjectWidth_->setValue(width);
    spinObjectWidth_->blockSignals(false);
    
    spinObjectHeight_->blockSignals(true);
    spinObjectHeight_->setValue(height);
    spinObjectHeight_->blockSignals(false);
}

// 文本颜色改变槽函数
void PropertyPanel::updateTextColor(const QColor& color)
{
    if (color.isValid()) {
        setColorButton(btnTextColor_, color);
    }
}

// 文本大小改变槽函数
void PropertyPanel::updateTextSize(int size)
{
    if (size > 0) {
        spinTextSize_->blockSignals(true);
        spinTextSize_->setValue(size);
        spinTextSize_->blockSignals(false);
    }
}

// 连接线颜色改变槽函数
void PropertyPanel::updateConnectorColor(const QColor& color)
{
    if (color.isValid()) {
        setColorButton(btnConnectorColor_, color);
        btnConnectorColor_->setEnabled(true);
    } else {
        setColorButton(btnConnectorColor_, Qt::black);
        btnConnectorColor_->setEnabled(false);
    }
}
