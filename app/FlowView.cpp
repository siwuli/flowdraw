#include "FlowView.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDrag>
#include <QJsonDocument>
#include <QApplication>
#include <QClipboard>
#include <algorithm>
#include <cmath>
#include <QMenu>

/* ===== 构造 ===== */
FlowView::FlowView(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    setAcceptDrops(true);
}

/* ======= 绘制 ======= */
void FlowView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    /* 背景 */
    p.fillRect(rect(), QColor("#fdfdfd"));

    /* 网格 */
    const int step = 20;
    p.setPen(QColor(220, 220, 220));
    for (int x = 0; x < width(); x += step) p.drawLine(x, 0, x, height());
    for (int y = 0; y < height(); y += step) p.drawLine(0, y, width(), y);

    /* 连接线（先画，在线下层） */
    for (const auto& c : connectors_) c.paint(p);
    if (currentConn_.src) currentConn_.paint(p);

    /* 图形 */
    for (int i = 0; i < shapes_.size(); ++i)
        shapes_[i]->paint(p, i == selectedIndex_);
}

/* ======= 鼠标事件 ======= */
void FlowView::mousePressEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton) return;

    /* --- 1. 新建矩形 / 椭圆 --- */
    if (mode_ == ToolMode::DrawRect) {
        auto r = std::make_unique<Rect>();
        r->bounds.setTopLeft(e->pos());
        r->bounds.setBottomRight(e->pos());
        shapes_.push_back(std::move(r));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = e->pos();
        return;
    }
    if (mode_ == ToolMode::DrawEllipse) {
        auto el = std::make_unique<Ellipse>();
        el->bounds.setTopLeft(e->pos());
        el->bounds.setBottomRight(e->pos());
        shapes_.push_back(std::move(el));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = e->pos();
        return;
    }

    /* --- 2. 新建连接线 (起点) --- */
    if (mode_ == ToolMode::DrawConnector && e->button() == Qt::LeftButton) {
        for (int i = shapes_.size() - 1; i >= 0; --i) {
            if (shapes_[i]->hitTest(e->pos())) {
                currentConn_.src = shapes_[i].get();
                currentConn_.tempEnd = e->pos();   // 先临时指向鼠标
                update();
                return;
            }
        }
    }

    /* --- 3. 普通选中 --- */
    selectedIndex_ = -1;
    for (int i = shapes_.size() - 1; i >= 0; --i) {
        if (shapes_[i]->hitTest(e->pos())) {
            selectedIndex_ = i;
            dragStart_ = e->pos();
            break;
        }
    }

    if (selectedIndex_ != -1) {
        auto* s = shapes_[selectedIndex_].get();
        emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
    }
    else {
        emit shapeAttr({}, {}, -1);   // 无选中
    }

    update();
}

void FlowView::mouseMoveEvent(QMouseEvent* e)
{
    /* --- 动态调整新矩形/椭圆大小 --- */
    if ((mode_ == ToolMode::DrawRect || mode_ == ToolMode::DrawEllipse) &&
        selectedIndex_ != -1)
    {
        shapes_[selectedIndex_]->bounds.setBottomRight(e->pos());
        update();
        return;
    }

    /* --- 连接线实时更新 --- */
    if (mode_ == ToolMode::DrawConnector && currentConn_.src) {
        currentConn_.tempEnd = e->pos();
        update();
        return;
    }

    /* --- 移动选中图形 --- */
    if (mode_ == ToolMode::None &&
        selectedIndex_ != -1 &&
        (e->buttons() & Qt::LeftButton))
    {
        QPointF d = e->pos() - dragStart_;
        dragStart_ = e->pos();

        //在移动选中图形的同时，重新计算所有连接线两个端点的锚点坐标。
        shapes_[selectedIndex_]->bounds.translate(d);
        updateConnectorsFor(shapes_[selectedIndex_].get());
        update();
    }
}

void FlowView::mouseReleaseEvent(QMouseEvent* e)
{
    Q_UNUSED(e)

    /* 完成 DrawRect / DrawEllipse */
    if (mode_ == ToolMode::DrawRect || mode_ == ToolMode::DrawEllipse) {
        mode_ = ToolMode::None;
        return;
    }

    /* 完成连接线 (寻找终点) */
    if (mode_ == ToolMode::DrawConnector && currentConn_.src) {
        for (int i = shapes_.size() - 1; i >= 0; --i) {
            if (shapes_[i]->hitTest(e->pos()) &&
                shapes_[i].get() != currentConn_.src)
            {
                currentConn_.dst = shapes_[i].get();
                break;
            }
        }
        if (currentConn_.dst)
            connectors_.push_back(currentConn_);
        currentConn_ = Connector{};
        mode_ = ToolMode::None;
        update();
        return;
    }
}

/* ======= 拖放 ======= */
void FlowView::dragEnterEvent(QDragEnterEvent* e)
{
    if (e->mimeData()->hasFormat("application/x-flow-shape"))
        e->acceptProposedAction();
}

void FlowView::dropEvent(QDropEvent* e)
{
    QByteArray ba = e->mimeData()->data("application/x-flow-shape");
    QString type = QString::fromUtf8(ba);

    std::unique_ptr<Shape> s;
    if (type == "rect")    s = std::make_unique<Rect>();
    else if (type == "ellipse") s = std::make_unique<Ellipse>();
    if (!s) return;

    QPointF pos = e->pos();
    s->bounds = { pos.x() - 50,pos.y() - 30,100,60 };
    shapes_.push_back(std::move(s));
    update();
    e->acceptProposedAction();
}

/* ======= 右键菜单 ======= */
void FlowView::contextMenuEvent(QContextMenuEvent* e)
{
    // 先进行 hit-test，把右键点击处设为当前选中
    selectedIndex_ = -1;
    for (int i = shapes_.size() - 1; i >= 0; --i) {
        if (shapes_[i]->hitTest(e->pos())) {
            selectedIndex_ = i;
            break;
        }
    }
    update();

    QMenu menu(this);

    auto actCopy = menu.addAction(tr("Copy\tCtrl+C"), this, &FlowView::copySelection);
    auto actCut = menu.addAction(tr("Cut\tCtrl+X"), this, &FlowView::cutSelection);
    auto actPaste = menu.addAction(tr("Paste\tCtrl+V"), this, &FlowView::pasteClipboard);
    menu.addSeparator();
    auto actDel = menu.addAction(tr("Delete\tDel"), this, &FlowView::deleteSelection);
    menu.addSeparator();
    menu.addAction(tr("Bring to Front\tCtrl+]"), this, &FlowView::bringToFront);
    menu.addAction(tr("Send to Back\tCtrl+["), this, &FlowView::sendToBack);
    menu.addAction(tr("Move Up\tCtrl+up"), this, &FlowView::moveUp);
    menu.addAction(tr("Move Down\tCtrl+down"), this, &FlowView::moveDown);

    // 根据是否有选中来启用/禁用
    bool hasSel = selectedIndex_ != -1;
    actCopy->setEnabled(hasSel);
    actCut->setEnabled(hasSel);
    actDel->setEnabled(hasSel);

    menu.exec(e->globalPos());
}


/* ======= 剪贴板函数 ======= */
void FlowView::copySelection()
{
    if (selectedIndex_ == -1) return;
    QJsonDocument doc(shapes_[selectedIndex_]->toJson());
    QApplication::clipboard()->setText(doc.toJson());
}

void FlowView::cutSelection()
{
    copySelection();
    deleteSelection();
}

void FlowView::pasteClipboard()
{
    QJsonDocument doc = QJsonDocument::fromJson(
        QApplication::clipboard()->text().toUtf8());
    if (!doc.isObject()) return;
    auto obj = doc.object();

    std::unique_ptr<Shape> s;
    QString type = obj["type"].toString();
    if (type == "rect")    s = std::make_unique<Rect>();
    else if (type == "ellipse") s = std::make_unique<Ellipse>();
    if (!s) return;
    s->fromJson(obj);
    s->bounds.translate(10, 10);       // 轻微偏移
    shapes_.push_back(std::move(s));
    update();
}

void FlowView::deleteSelection()
{
    if (selectedIndex_ == -1) return;
    shapes_.erase(shapes_.begin() + selectedIndex_);
    selectedIndex_ = -1;
    emit shapeAttr({}, {}, -1);
    update();
}

/* ======= Z-Order ======= */
void FlowView::bringToFront()
{
    if (selectedIndex_ == -1) return;
    auto tmp = std::move(shapes_[selectedIndex_]);
    shapes_.erase(shapes_.begin() + selectedIndex_);
    shapes_.push_back(std::move(tmp));
    selectedIndex_ = shapes_.size() - 1;
    update();
}

void FlowView::sendToBack()
{
    if (selectedIndex_ == -1) return;
    auto tmp = std::move(shapes_[selectedIndex_]);
    shapes_.erase(shapes_.begin() + selectedIndex_);
    shapes_.insert(shapes_.begin(), std::move(tmp));
    selectedIndex_ = 0;
    update();
}

void FlowView::moveUp()
{
    if (selectedIndex_ == -1 || selectedIndex_ + 1 >= shapes_.size()) return;
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


//实现三个 setter slot
void FlowView::setFill(const QColor& c)
{
    if (selectedIndex_ == -1 || !c.isValid()) return;
    shapes_[selectedIndex_]->fillColor = c;
    update();
    
    // ★ 立即把最新属性广播给面板
    auto* s = shapes_[selectedIndex_].get();
    emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
}
void FlowView::setStroke(const QColor& c)
{
    if (selectedIndex_ == -1 || !c.isValid()) return;
    shapes_[selectedIndex_]->strokeColor = c;
    update();

    // ★ 立即把最新属性广播给面板
    auto* s = shapes_[selectedIndex_].get();
    emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
}
void FlowView::setWidth(qreal w)
{
    if (selectedIndex_ == -1) return;
    shapes_[selectedIndex_]->strokeWidth = w;
    update();

    // ★ 立即把最新属性广播给面板
    auto* s = shapes_[selectedIndex_].get();
    emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
}


void FlowView::updateConnectorsFor(Shape* movedShape)
{
    for (auto& c : connectors_) {
        if (c.src == movedShape || c.dst == movedShape) {
            // 只需触发重绘即可，真正坐标在 paint() 时重新计算
        }
    }
}
