#include "FlowView.hpp"
#include <QPainter>
#include <QMouseEvent>
#include <QClipboard>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QMenu>
#include <algorithm> 

FlowView::FlowView(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);       // ��������ƶ��¼�
    setFocusPolicy(Qt::ClickFocus);
}

void FlowView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    /* === ������ɫ === */
    painter.fillRect(rect(), QColor("#fdfdfd"));

    /* === ���� === */
    const int gridStep = 20;                  // ������
    painter.setPen(QColor(220, 220, 220));    // ǳ��ɫ������

    for (int x = 0; x < width(); x += gridStep)
        painter.drawLine(x, 0, x, height());

    for (int y = 0; y < height(); y += gridStep)
        painter.drawLine(0, y, width(), y);

    for (size_t i = 0; i < shapes_.size(); ++i)
        shapes_[i]->paint(painter, static_cast<int>(i) == selectedIndex_);
}

/* ---------- ����¼� ---------- */
void FlowView::mousePressEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
        return;

    if (mode_ == ToolMode::DrawRect) {
        // �½����Σ���ʱ size Ϊ 0
        auto rect = std::make_unique<Rect>();
        rect->bounds.setTopLeft(e->pos());
        rect->bounds.setBottomRight(e->pos());
        shapes_.push_back(std::move(rect));
        selectedIndex_ = static_cast<int>(shapes_.size()) - 1;
        dragStart_ = e->pos();
    }
    else {
        // Hit-test���Ӷ���������
        selectedIndex_ = -1;
        for (int i = static_cast<int>(shapes_.size()) - 1; i >= 0; --i) {
            if (shapes_[i]->hitTest(e->pos())) {
                selectedIndex_ = i;
                dragStart_ = e->pos();
                break;
            }
        }
    }
    update();
}

void FlowView::mouseMoveEvent(QMouseEvent* e)
{
    if (!(e->buttons() & Qt::LeftButton))
        return;

    if (mode_ == ToolMode::DrawRect && selectedIndex_ != -1) {
        // ��̬�����¾��δ�С
        auto* r = dynamic_cast<Rect*>(shapes_[selectedIndex_].get());
        if (r) {
            r->bounds.setBottomRight(e->pos());
            update();
        }
    }
    else if (selectedIndex_ != -1) {
        // �ƶ���ѡ�е� shape
        QPointF delta = e->pos() - dragStart_;
        dragStart_ = e->pos();
        shapes_[selectedIndex_]->bounds.translate(delta);
        update();
    }
}

void FlowView::mouseReleaseEvent(QMouseEvent*)
{
    // ���һ�β����������ڻ��ƾ��Σ����˳�����ģʽ
    if (mode_ == ToolMode::DrawRect)
        mode_ = ToolMode::None;
}

//ʵ��ʾ�������� / ճ����
void FlowView::copySelection()
{
    if (selectedIndex_ == -1) return;
    QJsonObject obj = shapes_[selectedIndex_]->toJson();
    QJsonDocument doc(obj);
    QGuiApplication::clipboard()->setText(doc.toJson());
}

void FlowView::cutSelection()
{
    copySelection();
    deleteSelection();
}

void FlowView::pasteClipboard()
{
    QString txt = QGuiApplication::clipboard()->text();
    QJsonDocument doc = QJsonDocument::fromJson(txt.toUtf8());
    if (!doc.isObject()) return;

    QJsonObject obj = doc.object();
    std::unique_ptr<Shape> s;
    if (obj["type"] == "rect")
        s = std::make_unique<Rect>();
    // �Ժ����չ ellipse ��

    if (!s) return;
    s->fromJson(obj);

    // ��΢ƫ�Ʊ����ص�
    s->bounds.translate(10, 10);

    shapes_.push_back(std::move(s));
    selectedIndex_ = static_cast<int>(shapes_.size()) - 1;
    update();
}

//ɾ�� & Z-order
void FlowView::deleteSelection()
{
    if (selectedIndex_ == -1) return;
    shapes_.erase(shapes_.begin() + selectedIndex_);
    selectedIndex_ = -1;
    update();
}

void FlowView::bringToFront()
{
    if (selectedIndex_ == -1) return;
    auto sel = std::move(shapes_[selectedIndex_]);
    shapes_.erase(shapes_.begin() + selectedIndex_);
    shapes_.push_back(std::move(sel));
    selectedIndex_ = static_cast<int>(shapes_.size()) - 1;
    update();
}

void FlowView::sendToBack()
{
    if (selectedIndex_ == -1) return;
    auto sel = std::move(shapes_[selectedIndex_]);
    shapes_.erase(shapes_.begin() + selectedIndex_);
    shapes_.insert(shapes_.begin(), std::move(sel));
    selectedIndex_ = 0;
    update();
}

void FlowView::moveUp()
{
    if (selectedIndex_ < 0 || selectedIndex_ + 1 >= shapes_.size()) return;
    std::swap(shapes_[selectedIndex_], shapes_[selectedIndex_ + 1]);
    ++selectedIndex_;
    update();
}

void FlowView::moveDown()
{
    if (selectedIndex_ <= 0) return;
    std::swap(shapes_[selectedIndex_], shapes_[selectedIndex_ - 1]);
    --selectedIndex_;
    update();
}

//�Ҽ������Ĳ˵�
void FlowView::contextMenuEvent(QContextMenuEvent* e)
{
    QMenu menu(this);
    menu.addAction(tr("Copy"), this, &FlowView::copySelection);
    menu.addAction(tr("Cut"), this, &FlowView::cutSelection);
    menu.addAction(tr("Paste"), this, &FlowView::pasteClipboard);
    menu.addSeparator();
    menu.addAction(tr("Delete"), this, &FlowView::deleteSelection);
    menu.addSeparator();
    menu.addAction(tr("Bring to Front"), this, &FlowView::bringToFront);
    menu.addAction(tr("Send to Back"), this, &FlowView::sendToBack);
    menu.addAction(tr("Move Up"), this, &FlowView::moveUp);
    menu.addAction(tr("Move Down"), this, &FlowView::moveDown);

    menu.exec(e->globalPos());
}
