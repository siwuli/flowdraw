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
    setMouseTracking(true);       // 启用鼠标移动事件
    setFocusPolicy(Qt::ClickFocus);
}

void FlowView::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    /* === 背景底色 === */
    painter.fillRect(rect(), QColor("#fdfdfd"));

    /* === 网格 === */
    const int gridStep = 20;                  // 网格间距
    painter.setPen(QColor(220, 220, 220));    // 浅灰色网格线

    for (int x = 0; x < width(); x += gridStep)
        painter.drawLine(x, 0, x, height());

    for (int y = 0; y < height(); y += gridStep)
        painter.drawLine(0, y, width(), y);

    for (size_t i = 0; i < shapes_.size(); ++i)
        shapes_[i]->paint(painter, static_cast<int>(i) == selectedIndex_);
}

/* ---------- 鼠标事件 ---------- */
void FlowView::mousePressEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
        return;

    if (mode_ == ToolMode::DrawRect) {
        // 新建矩形，暂时 size 为 0
        auto rect = std::make_unique<Rect>();
        rect->bounds.setTopLeft(e->pos());
        rect->bounds.setBottomRight(e->pos());
        shapes_.push_back(std::move(rect));
        selectedIndex_ = static_cast<int>(shapes_.size()) - 1;
        dragStart_ = e->pos();
    }
    else {
        // Hit-test：从顶层往下找
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
        // 动态调整新矩形大小
        auto* r = dynamic_cast<Rect*>(shapes_[selectedIndex_].get());
        if (r) {
            r->bounds.setBottomRight(e->pos());
            update();
        }
    }
    else if (selectedIndex_ != -1) {
        // 移动已选中的 shape
        QPointF delta = e->pos() - dragStart_;
        dragStart_ = e->pos();
        shapes_[selectedIndex_]->bounds.translate(delta);
        update();
    }
}

void FlowView::mouseReleaseEvent(QMouseEvent*)
{
    // 完成一次操作，若是在绘制矩形，则退出绘制模式
    if (mode_ == ToolMode::DrawRect)
        mode_ = ToolMode::None;
}

//实现示例（复制 / 粘贴）
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
    // 以后可扩展 ellipse 等

    if (!s) return;
    s->fromJson(obj);

    // 轻微偏移避免重叠
    s->bounds.translate(10, 10);

    shapes_.push_back(std::move(s));
    selectedIndex_ = static_cast<int>(shapes_.size()) - 1;
    update();
}

//删除 & Z-order
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

//右键上下文菜单
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
