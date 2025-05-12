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
    
    // 文本属性组
    auto* textGroup = new QGroupBox(tr("Text Properties"), this);
    auto* textLay = new QFormLayout(textGroup);
    
    btnTextColor_ = new QPushButton;  btnTextColor_->setAutoFillBackground(true);
    spinTextSize_ = new QSpinBox;     spinTextSize_->setRange(8, 36);
    
    textLay->addRow(tr("Text Color"), btnTextColor_);
    textLay->addRow(tr("Font Size"), spinTextSize_);
    textLay->addRow(new QLabel(tr("Tip: Double-click shape to edit text"), this));
    
    // 添加到主布局
    mainLayout->addWidget(shapeGroup);
    mainLayout->addWidget(textGroup);
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
        
    // 文本属性信号连接
    connect(btnTextColor_, &QPushButton::clicked, this, [this] {
        QColor c = QColorDialog::getColor(btnTextColor_->palette().button().color(), this);
        if (c.isValid()) emit textColorChanged(c);
    });
    connect(spinTextSize_, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int v) { emit textSizeChanged(v); });
}

void PropertyPanel::load(const QColor& fill, const QColor& stroke, qreal width)
{
    bool hasSel = width >= 0;           // width<0 表示当前无选中

    btnFill_->setEnabled(hasSel);
    btnStroke_->setEnabled(hasSel);
    spinWidth_->setEnabled(hasSel);
    btnTextColor_->setEnabled(hasSel);
    spinTextSize_->setEnabled(hasSel);

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
