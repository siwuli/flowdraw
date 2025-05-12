#include "PropertyPanel.hpp"
#include <QFormLayout>
#include <QPushButton>
#include <QColorDialog>
#include <QSpinBox>

static void setColorButton(QPushButton* btn, const QColor& c)
{
    QPalette pal = btn->palette();
    pal.setColor(QPalette::Button, c.isValid() ? c : Qt::lightGray);
    btn->setPalette(pal);
    btn->update();
}

PropertyPanel::PropertyPanel(QWidget* parent) : QWidget(parent)
{
    auto* lay = new QFormLayout(this);

    btnFill_ = new QPushButton;  btnFill_->setAutoFillBackground(true);
    btnStroke_ = new QPushButton;  btnStroke_->setAutoFillBackground(true);
    spinWidth_ = new QSpinBox;     spinWidth_->setRange(1, 10);

    lay->addRow(tr("Fill"), btnFill_);
    lay->addRow(tr("Stroke"), btnStroke_);
    lay->addRow(tr("Width"), spinWidth_);

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
}

void PropertyPanel::load(const QColor& fill, const QColor& stroke, qreal width)
{
    bool hasSel = width >= 0;           // width<0 表示当前无选中

    btnFill_->setEnabled(hasSel);
    btnStroke_->setEnabled(hasSel);
    spinWidth_->setEnabled(hasSel);

    if (!hasSel) {                       // 灰掉显示
        setColorButton(btnFill_, {});
        setColorButton(btnStroke_, {});
        spinWidth_->blockSignals(true);
        spinWidth_->setValue(1);
        spinWidth_->blockSignals(false);
        return;
    }

    setColorButton(btnFill_, fill);
    setColorButton(btnStroke_, stroke);
    spinWidth_->blockSignals(true);
    spinWidth_->setValue(int(width));
    spinWidth_->blockSignals(false);
}
