#include "FlowView.hpp"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDrag>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QApplication>
#include <QClipboard>
#include <algorithm>
#include <cmath>
#include <QMenu>
#include <QPainterPath>
#include <QFile>
#include <QSvgGenerator>
#include "model/TextEditDialog.hpp"

/* =====  ===== */
FlowView::FlowView(QWidget* parent)
    : QWidget(parent)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::ClickFocus);
    setAcceptDrops(true);
}

/* ======= ���� ======= */
void FlowView::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    /* 背景 */
    p.fillRect(rect(), backgroundColor_);

    /* 网格 */
    if (showGrid_) {
        const int step = 20;
        p.setPen(QColor(220, 220, 220));
        for (int x = 0; x < width(); x += step) p.drawLine(x, 0, x, height());
        for (int y = 0; y < height(); y += step) p.drawLine(0, y, width(), y);
    }

    /* 连接线（先画连接线再画图形） */
    for (const auto& c : connectors_) c.paint(p);
    if (currentConn_.src) currentConn_.paint(p);

    /* 图形 */
    for (int i = 0; i < shapes_.size(); ++i)
        shapes_[i]->paint(p, i == selectedIndex_);
}

/* ======= ����¼� ======= */
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
                currentConn_.tempEnd = e->pos();   // temporary pointer position
                update();
                return;
            }
        }
    }

    /* --- 3. 普通选择 --- */
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
        emit shapeAttr({}, {}, -1);   // no selection
    }

    update();
}

void FlowView::mouseMoveEvent(QMouseEvent* e)
{
    /* --- ��̬�����¾���/��Բ��С --- */
    if ((mode_ == ToolMode::DrawRect || mode_ == ToolMode::DrawEllipse) &&
        selectedIndex_ != -1)
    {
        shapes_[selectedIndex_]->bounds.setBottomRight(e->pos());
        update();
        return;
    }

    /* --- ������ʵʱ���� --- */
    if (mode_ == ToolMode::DrawConnector && currentConn_.src) {
        currentConn_.tempEnd = e->pos();
        update();
        return;
    }

    /* --- �ƶ�ѡ��ͼ�� --- */
    if (mode_ == ToolMode::None &&
        selectedIndex_ != -1 &&
        (e->buttons() & Qt::LeftButton))
    {
        QPointF d = e->pos() - dragStart_;
        dragStart_ = e->pos();

        //���ƶ�ѡ��ͼ�ε�ͬʱ�����¼������������������˵��ê�����ꡣ
        shapes_[selectedIndex_]->bounds.translate(d);
        updateConnectorsFor(shapes_[selectedIndex_].get());
        update();
    }
}

void FlowView::mouseReleaseEvent(QMouseEvent* e)
{
    Q_UNUSED(e)

    /* ��� DrawRect / DrawEllipse */
    if (mode_ == ToolMode::DrawRect || mode_ == ToolMode::DrawEllipse) {
        mode_ = ToolMode::None;
        return;
    }

    /* ��������� (Ѱ���յ�) */
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

/* ======= �Ϸ� ======= */
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

/* ======= �Ҽ��˵� ======= */
void FlowView::contextMenuEvent(QContextMenuEvent* e)
{
    // �Ƚ��� hit-test�����Ҽ��������Ϊ��ǰѡ��
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

    // �����Ƿ���ѡ��������/����
    bool hasSel = selectedIndex_ != -1;
    actCopy->setEnabled(hasSel);
    actCut->setEnabled(hasSel);
    actDel->setEnabled(hasSel);

    menu.exec(e->globalPos());
}


/* ======= �����庯�� ======= */
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
    s->bounds.translate(10, 10);       // ��΢ƫ��
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


//ʵ������ setter slot
void FlowView::setFill(const QColor& c)
{
    if (selectedIndex_ == -1 || !c.isValid()) return;
    shapes_[selectedIndex_]->fillColor = c;
    update();
    
    // �� �������������Թ㲥�����
    auto* s = shapes_[selectedIndex_].get();
    emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
}
void FlowView::setStroke(const QColor& c)
{
    if (selectedIndex_ == -1 || !c.isValid()) return;
    shapes_[selectedIndex_]->strokeColor = c;
    update();

    // �� �������������Թ㲥�����
    auto* s = shapes_[selectedIndex_].get();
    emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
}
void FlowView::setWidth(qreal w)
{
    if (selectedIndex_ == -1) return;
    shapes_[selectedIndex_]->strokeWidth = w;
    update();

    // �� �������������Թ㲥�����
    auto* s = shapes_[selectedIndex_].get();
    emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
}


void FlowView::updateConnectorsFor(Shape* movedShape)
{
    for (auto& c : connectors_) {
        if (c.src == movedShape || c.dst == movedShape) {
            // ֻ�败���ػ漴�ɣ����������� paint() ʱ���¼���
        }
    }
}

void FlowView::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton) {
        // 检查是否点击了某个形状
        for (int i = shapes_.size() - 1; i >= 0; --i) {
            if (shapes_[i]->hitTest(e->pos())) {
                selectedIndex_ = i;
                // 打开文本编辑对话框
                TextEditDialog dialog(this, shapes_[i]->text);
                if (dialog.exec() == QDialog::Accepted) {
                    shapes_[i]->text = dialog.getText();
                    update();
                }
                break;
            }
        }
    }
}

// 添加文本颜色设置
void FlowView::setTextColor(const QColor& c)
{
    if (selectedIndex_ == -1 || !c.isValid()) return;
    shapes_[selectedIndex_]->textColor = c;
    update();
}

// 添加文本大小设置
void FlowView::setTextSize(int size)
{
    if (selectedIndex_ == -1 || size <= 0) return;
    shapes_[selectedIndex_]->textSize = size;
    update();
}

// 添加文本内容设置
void FlowView::setText(const QString& text)
{
    if (selectedIndex_ == -1) return;
    shapes_[selectedIndex_]->text = text;
    update();
}

/* ---------- 文件操作 ---------- */

bool FlowView::saveToFile(const QString& filename)
{
    QJsonObject root;
    
    // 保存页面属性
    QJsonObject pageObj;
    pageObj["backgroundColor"] = backgroundColor_.name(QColor::HexArgb);
    pageObj["width"] = pageSize_.width();
    pageObj["height"] = pageSize_.height();
    pageObj["showGrid"] = showGrid_;
    root["page"] = pageObj;
    
    // 保存所有图形
    QJsonArray shapesArray;
    for (const auto& shape : shapes_) {
        shapesArray.append(shape->toJson());
    }
    root["shapes"] = shapesArray;
    
    // 保存所有连接线
    QJsonArray connArray;
    for (const auto& conn : connectors_) {
        // 需要找到连接线的源和目标在shapes中的索引
        int srcIdx = -1, dstIdx = -1;
        for (size_t i = 0; i < shapes_.size(); ++i) {
            if (shapes_[i].get() == conn.src) srcIdx = i;
            if (shapes_[i].get() == conn.dst) dstIdx = i;
        }
        
        if (srcIdx >= 0 && dstIdx >= 0) {
            QJsonObject connObj;
            connObj["src"] = srcIdx;
            connObj["dst"] = dstIdx;
            connObj["color"] = conn.color.name(QColor::HexArgb);
            connObj["width"] = conn.width;
            connArray.append(connObj);
        }
    }
    root["connectors"] = connArray;
    
    // 写入文件
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson());
    return true;
}

bool FlowView::loadFromFile(const QString& filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return false;
    }
    
    // 清空当前数据
    clearAll();
    
    QJsonObject root = doc.object();
    
    // 加载页面属性
    if (root.contains("page") && root["page"].isObject()) {
        QJsonObject pageObj = root["page"].toObject();
        backgroundColor_ = QColor(pageObj["backgroundColor"].toString("#fdfdfd"));
        pageSize_ = QSize(
            pageObj["width"].toInt(2000),
            pageObj["height"].toInt(2000)
        );
        showGrid_ = pageObj["showGrid"].toBool(true);
    }
    
    // 加载图形
    if (root.contains("shapes") && root["shapes"].isArray()) {
        QJsonArray shapesArray = root["shapes"].toArray();
        for (const QJsonValue& val : shapesArray) {
            if (!val.isObject()) continue;
            
            QJsonObject shapeObj = val.toObject();
            QString type = shapeObj["type"].toString();
            
            std::unique_ptr<Shape> shape;
            if (type == "rect") {
                shape = std::make_unique<Rect>();
            } else if (type == "ellipse") {
                shape = std::make_unique<Ellipse>();
            }
            
            if (shape) {
                shape->fromJson(shapeObj);
                shapes_.push_back(std::move(shape));
            }
        }
    }
    
    // 加载连接线
    if (root.contains("connectors") && root["connectors"].isArray()) {
        QJsonArray connArray = root["connectors"].toArray();
        for (const QJsonValue& val : connArray) {
            if (!val.isObject()) continue;
            
            QJsonObject connObj = val.toObject();
            int srcIdx = connObj["src"].toInt(-1);
            int dstIdx = connObj["dst"].toInt(-1);
            
            if (srcIdx >= 0 && srcIdx < shapes_.size() &&
                dstIdx >= 0 && dstIdx < shapes_.size()) {
                Connector conn;
                conn.src = shapes_[srcIdx].get();
                conn.dst = shapes_[dstIdx].get();
                conn.color = QColor(connObj["color"].toString("#ff000000"));
                conn.width = connObj["width"].toDouble(1.0);
                connectors_.push_back(conn);
            }
        }
    }
    
    update();
    return true;
}

bool FlowView::exportToPng(const QString& filename)
{
    QImage image(pageSize_, QImage::Format_ARGB32);
    image.fill(backgroundColor_);
    
    QPainter painter(&image);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制网格
    if (showGrid_) {
        const int step = 20;
        painter.setPen(QColor(220, 220, 220));
        for (int x = 0; x < pageSize_.width(); x += step) 
            painter.drawLine(x, 0, x, pageSize_.height());
        for (int y = 0; y < pageSize_.height(); y += step) 
            painter.drawLine(0, y, pageSize_.width(), y);
    }
    
    // 绘制连接线
    for (const auto& c : connectors_) 
        c.paint(painter);
    
    // 绘制图形
    for (const auto& shape : shapes_) 
        shape->paint(painter, false);
    
    return image.save(filename, "PNG");
}

bool FlowView::exportToSvg(const QString& filename)
{
    QSvgGenerator generator;
    generator.setFileName(filename);
    generator.setSize(pageSize_);
    generator.setViewBox(QRect(0, 0, pageSize_.width(), pageSize_.height()));
    generator.setTitle("FlowDraw Diagram");
    generator.setDescription("Created with FlowDraw");
    
    QPainter painter;
    painter.begin(&generator);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 背景
    painter.fillRect(QRect(0, 0, pageSize_.width(), pageSize_.height()), backgroundColor_);
    
    // 绘制网格
    if (showGrid_) {
        const int step = 20;
        painter.setPen(QColor(220, 220, 220));
        for (int x = 0; x < pageSize_.width(); x += step) 
            painter.drawLine(x, 0, x, pageSize_.height());
        for (int y = 0; y < pageSize_.height(); y += step) 
            painter.drawLine(0, y, pageSize_.width(), y);
    }
    
    // 绘制连接线
    for (const auto& c : connectors_) 
        c.paint(painter);
    
    // 绘制图形
    for (const auto& shape : shapes_) 
        shape->paint(painter, false);
    
    painter.end();
    return true;
}

void FlowView::clearAll()
{
    shapes_.clear();
    connectors_.clear();
    selectedIndex_ = -1;
    currentConn_ = Connector{};
    update();
}

/* ---------- 页面设置 ---------- */

void FlowView::setBackgroundColor(const QColor& color)
{
    if (color.isValid()) {
        backgroundColor_ = color;
        update();
    }
}

void FlowView::setPageSize(int width, int height)
{
    if (width > 0 && height > 0) {
        pageSize_ = QSize(width, height);
        update();
    }
}

void FlowView::setGridVisible(bool visible)
{
    showGrid_ = visible;
    update();
}
