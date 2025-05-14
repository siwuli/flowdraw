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
#include "model/Diamond.hpp"
#include "model/Triangle.hpp"
#include "model/Ellipse.hpp"
#include "model/Pentagon.hpp"
#include "model/Hexagon.hpp"
#include "model/Octagon.hpp"
#include "model/RoundedRect.hpp"
#include "model/Capsule.hpp"
#include <QColorDialog>

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
    
    // 填充窗口背景
    p.fillRect(rect(), QColor("#f0f0f0")); // 灰色的窗口背景
    
    // 绘制页面边界和背景
    drawPageBorder(p);
    
    // 创建剪裁区，只在页面内绘制
    QRectF pageRect = QRectF(docToView(QPointF(0, 0)), 
                             docToView(QPointF(pageSize_.width(), pageSize_.height())));
    p.setClipRect(pageRect);
    
    // 应用视图变换
    p.save();
    p.translate(viewOffset_);
    p.scale(scale_, scale_);

    /* 网格 */
    if (showGrid_) {
        const int step = 20;
        p.setPen(QColor(220, 220, 220));
        
        // 计算网格的起始和结束位置
        int startX = 0;
        int startY = 0;
        int endX = pageSize_.width();
        int endY = pageSize_.height();
        
        for (int x = startX; x <= endX; x += step) 
            p.drawLine(x, startY, x, endY);
        for (int y = startY; y <= endY; y += step) 
            p.drawLine(startX, y, endX, y);
    }

    /* 连接线（先画连接线再画图形） */
    for (const auto& c : connectors_) c.paint(p);
    if (currentConn_.src) currentConn_.paint(p);

    /* 图形 */
    for (int i = 0; i < shapes_.size(); ++i)
        shapes_[i]->paint(p, i == selectedIndex_);
    
    /* 如果有选中的元素，绘制调整大小的控制柄 */
    if (selectedIndex_ >= 0 && selectedIndex_ < shapes_.size()) {
        drawResizeHandles(p, shapes_[selectedIndex_]->bounds);
    }
        
    p.restore();
}

/* ======= ¼ ======= */
void FlowView::mousePressEvent(QMouseEvent* event)
{
    setFocus(); // 获取焦点，以便接收键盘事件
    
    if (isPanning_) {
        if (event->button() == Qt::LeftButton) {
            setCursor(Qt::ClosedHandCursor);
            lastPanPoint_ = event->pos();
            event->accept();
            return;
        }
    }

    // 将视图坐标转换为文档坐标
    QPointF docPos = viewToDoc(event->pos());
    
    // 检查点击位置是否在页面内
    if (docPos.x() < 0 || docPos.y() < 0 || 
        docPos.x() > pageSize_.width() || docPos.y() > pageSize_.height()) {
        return;
    }

    if (event->button() != Qt::LeftButton) return;

    /* --- 1. 新建矩形 / 椭圆 --- */
    if (mode_ == ToolMode::DrawRect) {
        auto r = std::make_unique<Rect>();
        r->bounds.setTopLeft(docPos);
        r->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(r));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }
    if (mode_ == ToolMode::DrawEllipse) {
        auto el = std::make_unique<Ellipse>();
        el->bounds.setTopLeft(docPos);
        el->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(el));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }
    if (mode_ == ToolMode::DrawDiamond) {
        auto diamond = std::make_unique<Diamond>();
        diamond->bounds.setTopLeft(docPos);
        diamond->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(diamond));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }
    if (mode_ == ToolMode::DrawTriangle) {
        auto triangle = std::make_unique<Triangle>();
        triangle->bounds.setTopLeft(docPos);
        triangle->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(triangle));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }
    if (mode_ == ToolMode::DrawPentagon) {
        auto pentagon = std::make_unique<Pentagon>();
        pentagon->bounds.setTopLeft(docPos);
        pentagon->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(pentagon));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }
    if (mode_ == ToolMode::DrawHexagon) {
        auto hexagon = std::make_unique<Hexagon>();
        hexagon->bounds.setTopLeft(docPos);
        hexagon->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(hexagon));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }
    if (mode_ == ToolMode::DrawOctagon) {
        auto octagon = std::make_unique<Octagon>();
        octagon->bounds.setTopLeft(docPos);
        octagon->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(octagon));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }
    if (mode_ == ToolMode::DrawRoundedRect) {
        auto roundedRect = std::make_unique<RoundedRect>();
        roundedRect->bounds.setTopLeft(docPos);
        roundedRect->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(roundedRect));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }
    if (mode_ == ToolMode::DrawCapsule) {
        auto capsule = std::make_unique<Capsule>();
        capsule->bounds.setTopLeft(docPos);
        capsule->bounds.setBottomRight(docPos);
        shapes_.push_back(std::move(capsule));
        selectedIndex_ = int(shapes_.size()) - 1;
        dragStart_ = docPos;
        return;
    }

    /* --- 2. 新建连接线 (起点) --- */
    if (mode_ == ToolMode::DrawConnector && event->button() == Qt::LeftButton) {
        // 如果当前已经有一个连接线起点，则尝试完成连接
        if (currentConn_.src) {
            // 检查是否点击到了作为终点的形状
            for (int i = shapes_.size() - 1; i >= 0; --i) {
                if (shapes_[i].get() != currentConn_.src && shapes_[i]->hitTest(docPos)) {
                    // 找到终点形状，创建连接线
                    currentConn_.dst = shapes_[i].get();
                    connectors_.push_back(currentConn_);
                    
                    // 重置当前连接线
                    currentConn_ = Connector{};
                    
                    // 完成连线后，自动退出连接器模式
                    mode_ = ToolMode::None;
                    setCursor(Qt::ArrowCursor);
                    
                    // 更新视图
                    update();
                    return;
                }
            }
            
            // 如果点击空白处或同一个形状，取消当前连接线
            currentConn_ = Connector{};
            update();
            return;
        }
        
        // 检查是否点击了已有图形作为连线起点
        for (int i = shapes_.size() - 1; i >= 0; --i) {
            if (shapes_[i]->hitTest(docPos)) {
                currentConn_.src = shapes_[i].get();
                currentConn_.tempEnd = docPos;   // temporary pointer position
                update();
                return;
            }
        }
        
        // 如果没有点击到任何形状，但处于连接器模式，尝试以选择操作处理
        if (mode_ == ToolMode::DrawConnector) {
            // 尝试选择模式的行为
            int newSelectedIndex = -1;
            for (int i = shapes_.size() - 1; i >= 0; --i) {
                if (shapes_[i]->hitTest(docPos)) {
                    newSelectedIndex = i;
                    break;
                }
            }
            
            if (newSelectedIndex != -1) {
                // 点击到了形状，但没有开始连线，切换回选择模式
                selectedIndex_ = newSelectedIndex;
                auto* s = shapes_[selectedIndex_].get();
                emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
                updatePropertyPanel();
                
                // 切换回选择模式
                mode_ = ToolMode::None;
                setCursor(Qt::ArrowCursor);
                update();
                return;
            }
        }
    }

    /* --- 3. 检查是否点击了调整柄 --- */
    if (selectedIndex_ != -1 && mode_ == ToolMode::None) {
        resizeHandle_ = hitTestResizeHandles(docPos, shapes_[selectedIndex_]->bounds);
        if (resizeHandle_ != ResizeHandle::None) {
            dragStart_ = docPos;
            // 保存调整大小前的状态
            lastShapeState_ = shapes_[selectedIndex_]->toJson();
            event->accept();
            return;
        }
    }

    /* --- 4. 普通选择 --- */
    selectedIndex_ = -1;
    selectedConnectorIndex_ = -1;
    
    // 首先尝试选择连接线
    selectedConnectorIndex_ = hitTestConnector(docPos);
    
    // 如果没有选中连接线，再尝试选择图形
    if (selectedConnectorIndex_ == -1) {
        for (int i = shapes_.size() - 1; i >= 0; --i) {
            if (shapes_[i]->hitTest(docPos)) {
                selectedIndex_ = i;
                dragStart_ = docPos;
                // 保存移动前的状态
                lastShapeState_ = shapes_[i]->toJson();
                break;
            }
        }
    }
    
    if (selectedIndex_ != -1) {
        auto* s = shapes_[selectedIndex_].get();
        emit shapeAttr(s->fillColor, s->strokeColor, s->strokeWidth);
        // 更新属性面板，包括尺寸
        updatePropertyPanel();
    }
    else if (selectedConnectorIndex_ != -1) {
        // 如果选中了连接线，更新属性面板
        updatePropertyPanel();
    }
    else {
        emit shapeAttr({}, {}, -1);   // no selection
        emit shapeSize(0, 0);  // 清空尺寸数据
        
        // 如果点击空白处且处于连接器模式，退回到选择模式
        if (mode_ == ToolMode::DrawConnector) {
            mode_ = ToolMode::None;
            currentConn_ = Connector{};
            setCursor(Qt::ArrowCursor);
        }
    }

    // 如果未选中图形，重置调整柄
    if (selectedIndex_ == -1) {
        resizeHandle_ = ResizeHandle::None;
    }
    
    update();
}

void FlowView::mouseMoveEvent(QMouseEvent* event)
{
    if (isPanning_ && (event->buttons() & Qt::LeftButton)) {
        // 平移视图
        viewOffset_ += event->pos() - lastPanPoint_;
        lastPanPoint_ = event->pos();
        update();
        return;
    }
    
    // 将视图坐标转换为文档坐标
    QPointF docPos = viewToDoc(event->pos());

    /* --- 调整矩形/椭圆大小（通过拖拽角度和边缘） --- */
    if (selectedIndex_ != -1 && resizeHandle_ != ResizeHandle::None &&
        (event->buttons() & Qt::LeftButton))
    {
        QPointF offset = docPos - dragStart_;
        dragStart_ = docPos;
        
        resizeRect(shapes_[selectedIndex_]->bounds, resizeHandle_, offset);
        updateConnectorsFor(shapes_[selectedIndex_].get());
        updatePropertyPanel();  // 更新尺寸属性面板
        update();
        return;
    }

    /* --- 1. 调整矩形大小 --- */
    if ((mode_ == ToolMode::DrawRect || mode_ == ToolMode::DrawEllipse || mode_ == ToolMode::DrawDiamond || 
         mode_ == ToolMode::DrawTriangle || mode_ == ToolMode::DrawPentagon || mode_ == ToolMode::DrawHexagon || 
         mode_ == ToolMode::DrawOctagon || mode_ == ToolMode::DrawRoundedRect || mode_ == ToolMode::DrawCapsule) &&
        selectedIndex_ != -1 && (event->buttons() & Qt::LeftButton))
    {
        auto& r = shapes_[selectedIndex_]->bounds;
        r.setBottomRight(docPos);
        update();
        return;
    }

    /* --- 2. 连接线拖拽 --- */
    if (mode_ == ToolMode::DrawConnector && currentConn_.src)
    {
        currentConn_.tempEnd = docPos;
                
        // 查找终点是否落在任何图形上
        Shape* hitShape = nullptr;
        for (int i = shapes_.size() - 1; i >= 0; --i) {
            if (shapes_[i].get() != currentConn_.src && shapes_[i]->hitTest(docPos)) {
                hitShape = shapes_[i].get();
                break;
            }
        }
        
        // 当鼠标悬停在可连接的目标形状上时，改变光标样式提示用户
        if (hitShape) {
            setCursor(Qt::PointingHandCursor);
        } else {
            setCursor(Qt::CrossCursor);
        }
        
        // 设置临时终点
        currentConn_.dst = hitShape;
        update();
        return;
    }
    
    /* --- 3. 拖拽移动图形 --- */
    if (mode_ == ToolMode::None && selectedIndex_ != -1 && 
        (event->buttons() & Qt::LeftButton) && resizeHandle_ == ResizeHandle::None)
    {
        QPointF delta = docPos - dragStart_;
        dragStart_ = docPos;
        
        shapes_[selectedIndex_]->bounds.translate(delta);
        updateConnectorsFor(shapes_[selectedIndex_].get());
        update();
        return;
    }
    
    /* --- 4. 悬停时显示合适的鼠标指针 --- */
    if (selectedIndex_ != -1 && mode_ == ToolMode::None) {
        ResizeHandle hitHandle = hitTestResizeHandles(docPos, shapes_[selectedIndex_]->bounds);
        
        if (hitHandle != ResizeHandle::None) {
            // 根据调整柄类型设置不同的鼠标指针形状
            switch (hitHandle) {
                case ResizeHandle::TopLeft:
                case ResizeHandle::BottomRight:
                    setCursor(Qt::SizeFDiagCursor);
                    break;
                    
                case ResizeHandle::TopRight:
                case ResizeHandle::BottomLeft:
                    setCursor(Qt::SizeBDiagCursor);
                    break;
                    
                case ResizeHandle::TopCenter:
                case ResizeHandle::BottomCenter:
                    setCursor(Qt::SizeVerCursor);
                    break;
                    
                case ResizeHandle::MiddleLeft:
                case ResizeHandle::MiddleRight:
                    setCursor(Qt::SizeHorCursor);
                    break;
                    
                default:
                    setCursor(Qt::ArrowCursor);
                    break;
            }
            return;
        }
    }
    
    // 检查是否悬停在任何图形上
    bool hitAnyShape = false;
    for (int i = shapes_.size() - 1; i >= 0; --i) {
        if (shapes_[i]->hitTest(docPos)) {
            hitAnyShape = true;
            break;
        }
    }
    
    // 在连接器模式下，如果鼠标悬停在形状上显示相应的光标
    if (mode_ == ToolMode::DrawConnector) {
        setCursor(hitAnyShape ? Qt::PointingHandCursor : Qt::CrossCursor);
    } else {
        setCursor(hitAnyShape ? Qt::OpenHandCursor : Qt::ArrowCursor);
    }
}

void FlowView::mouseReleaseEvent(QMouseEvent* event)
{
    if (isPanning_ && event->button() == Qt::LeftButton) {
        setCursor(Qt::OpenHandCursor);
        update();
        return;
    }
    
    QPointF docPos = viewToDoc(event->pos());

    /* --- 完成连接线绘制 --- */
    if (mode_ == ToolMode::DrawConnector && currentConn_.src && event->button() == Qt::LeftButton)
    {
        // 如果找到了终点，添加这条连接线
        if (currentConn_.dst)
        {
            // 保存当前连接线的源和目标索引
            int srcIndex = -1, dstIndex = -1;
            for (size_t i = 0; i < shapes_.size(); ++i) {
                if (shapes_[i].get() == currentConn_.src) srcIndex = i;
                if (shapes_[i].get() == currentConn_.dst) dstIndex = i;
            }
            
            // 添加连接线
            connectors_.push_back(currentConn_);
            int connIndex = connectors_.size() - 1;
            
            // 记录连接线创建历史
            recordConnectorAction(ActionType::AddConn, connIndex, srcIndex, dstIndex);
            
            // 重置当前连接线
            currentConn_ = Connector{};
            
            // 连线完成后，退出连接器模式
            mode_ = ToolMode::None;
            setCursor(Qt::ArrowCursor);
        }
        else
        {
            // 没有终点，取消当前连接线
            currentConn_ = Connector{};
        }
        
        update();
        return;
    }
    
    /* --- 完成矩形/椭圆绘制 --- */
    if ((mode_ == ToolMode::DrawRect || mode_ == ToolMode::DrawEllipse || mode_ == ToolMode::DrawDiamond || 
         mode_ == ToolMode::DrawTriangle || mode_ == ToolMode::DrawPentagon || mode_ == ToolMode::DrawHexagon ||
         mode_ == ToolMode::DrawOctagon || mode_ == ToolMode::DrawRoundedRect || mode_ == ToolMode::DrawCapsule) && 
        selectedIndex_ != -1 && event->button() == Qt::LeftButton)
    {
        auto& r = shapes_[selectedIndex_]->bounds;
        if (r.width() < 5 || r.height() < 5) {
            // 如果太小则删除
            shapes_.erase(shapes_.begin() + selectedIndex_);
            selectedIndex_ = -1;
        } else {
            // 确保矩形尺寸正常
            if (r.width() < 0) {
                r.setLeft(r.right());
                r.setRight(r.left() - r.width());
            }
            if (r.height() < 0) {
                r.setTop(r.bottom());
                r.setBottom(r.top() - r.height());
            }
            
            // 记录图形创建历史
            QJsonObject shapeState = shapes_[selectedIndex_]->toJson();
            recordAction(ActionType::Add, selectedIndex_, QJsonObject(), shapeState);
            
            // 绘制完成后，切换回选择工具
            mode_ = ToolMode::None;
            setCursor(Qt::ArrowCursor);
        }
        
        update();
        return;
    }

    /* --- 拖拽时，如果移出了画布区域则删除 --- */
    if (selectedIndex_ != -1 && event->button() == Qt::LeftButton)
    {
        if (docPos.x() < 0 || docPos.y() < 0 || 
            docPos.x() > pageSize_.width() || docPos.y() > pageSize_.height())
        {
            // 记录删除前的状态
            QJsonObject stateBefore = shapes_[selectedIndex_]->toJson();
            int index = selectedIndex_;
            
            // 执行删除
            shapes_.erase(shapes_.begin() + selectedIndex_);
            selectedIndex_ = -1;
            
            // 记录删除操作
            recordAction(ActionType::Delete, index, stateBefore, QJsonObject());
            
            update();
        }
    }
 
    if (event->button() == Qt::LeftButton) {
        // 在拖动或调整大小结束时记录历史
        if (selectedIndex_ != -1 && !lastShapeState_.isEmpty()) {
            QJsonObject currentState = shapes_[selectedIndex_]->toJson();
            if (resizeHandle_ != ResizeHandle::None) {
                // 调整大小操作
                recordAction(ActionType::Resize, selectedIndex_, lastShapeState_, currentState);
            } else {
                // 移动操作
                recordAction(ActionType::Move, selectedIndex_, lastShapeState_, currentState);
            }
            lastShapeState_ = QJsonObject(); // 清空保存的状态
        }
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

    // 将放置位置从视图坐标转换为文档坐标
    QPointF docPos = viewToDoc(e->pos());
    
    // 检查放置位置是否在页面内
    if (docPos.x() < 0 || docPos.y() < 0 || 
        docPos.x() > pageSize_.width() || docPos.y() > pageSize_.height()) {
        return;
    }

    // 如果是连接器类型，切换到连接器绘制模式
    if (type == "connector") {
        setToolMode(ToolMode::DrawConnector);
        setCursor(Qt::CrossCursor);  // 设置十字光标
        e->acceptProposedAction();
        return;
    }

    std::unique_ptr<Shape> s;
    if (type == "rect")    s = std::make_unique<Rect>();
    else if (type == "ellipse") s = std::make_unique<Ellipse>();
    else if (type == "diamond") s = std::make_unique<Diamond>();
    else if (type == "triangle") s = std::make_unique<Triangle>();
    else if (type == "pentagon") s = std::make_unique<Pentagon>();
    else if (type == "hexagon") s = std::make_unique<Hexagon>();
    else if (type == "octagon") s = std::make_unique<Octagon>();
    else if (type == "roundedrect") s = std::make_unique<RoundedRect>();
    else if (type == "capsule") s = std::make_unique<Capsule>();
    if (!s) return;
    
    s->bounds = { docPos.x() - 50, docPos.y() - 30, 100, 60 };
    shapes_.push_back(std::move(s));
    
    // 选中新放置的图形
    selectedIndex_ = static_cast<int>(shapes_.size() - 1);
    
    // 记录图形创建历史
    QJsonObject shapeState = shapes_[selectedIndex_]->toJson();
    recordAction(ActionType::Add, selectedIndex_, QJsonObject(), shapeState);
    
    updatePropertyPanel();
    
    update();
    e->acceptProposedAction();
}

/* ======= 右键菜单 ======= */
void FlowView::contextMenuEvent(QContextMenuEvent* e)
{
    // 将位置从视图坐标转换为文档坐标
    QPointF docPos = viewToDoc(e->pos());
    
    // 先进行hit-test，查看是否点击了连接线
    selectedConnectorIndex_ = hitTestConnector(docPos);
    
    // 如果没有点击到连接线，再检查是否点击了图形
    if (selectedConnectorIndex_ == -1) {
        // 查找点击的图形
        selectedIndex_ = -1;
        for (int i = static_cast<int>(shapes_.size()) - 1; i >= 0; --i) {
            if (shapes_[i]->hitTest(docPos)) {
                selectedIndex_ = i;
                break;
            }
        }
    } else {
        // 如果点击了连接线，清除图形选择
        selectedIndex_ = -1;
    }
    
    update();
    updatePropertyPanel();

    QMenu menu(this);

    // 如果在页面内右键，则显示菜单
    if (docPos.x() >= 0 && docPos.y() >= 0 && 
        docPos.x() <= pageSize_.width() && docPos.y() <= pageSize_.height()) {
        
        // 如果选中了连接线
        if (selectedConnectorIndex_ != -1) {
            // 创建连接线菜单
            Connector& conn = connectors_[selectedConnectorIndex_];
            
            // 添加连接线颜色选项
            QAction* actConnectorColor = menu.addAction(tr("Connector Color"));
            connect(actConnectorColor, &QAction::triggered, this, [this, &conn]() {
                QColor color = QColorDialog::getColor(conn.color, this, tr("Select Connector Color"));
                if (color.isValid()) {
                    conn.color = color;
                    emit connectorColorChanged(color);
                    update();
                }
            });
            
            // 添加双向箭头切换选项
            QAction* actBidirectional = menu.addAction(tr("Bidirectional"));
            actBidirectional->setCheckable(true);
            actBidirectional->setChecked(conn.bidirectional);
            connect(actBidirectional, &QAction::toggled, this, [this, &conn](bool checked) {
                conn.bidirectional = checked;
                update();
            });
            
            menu.addSeparator();
            
            // 添加删除连接线选项
            QAction* actDeleteConn = menu.addAction(tr("Delete Connection"));
            connect(actDeleteConn, &QAction::triggered, this, [this]() {
                if (selectedConnectorIndex_ >= 0 && selectedConnectorIndex_ < static_cast<int>(connectors_.size())) {
                    // 找出连接线的源和目标图形索引
                    int srcIndex = -1, dstIndex = -1;
                    for (size_t i = 0; i < shapes_.size(); ++i) {
                        if (shapes_[i].get() == connectors_[selectedConnectorIndex_].src) srcIndex = static_cast<int>(i);
                        if (shapes_[i].get() == connectors_[selectedConnectorIndex_].dst) dstIndex = static_cast<int>(i);
                    }
                    
                    // 记录删除连接线历史
                    int index = selectedConnectorIndex_;
                    recordConnectorAction(ActionType::DeleteConn, index, srcIndex, dstIndex);
                    
                    // 执行删除
                    connectors_.erase(connectors_.begin() + selectedConnectorIndex_);
                    selectedConnectorIndex_ = -1;
                    update();
                }
            });
        }
        // 如果选中了图形
        else if (selectedIndex_ != -1) {
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

            // 控制是否有选中图形决定复制/粘贴的启用
            bool hasSel = selectedIndex_ != -1;
            actCopy->setEnabled(hasSel);
            actCut->setEnabled(hasSel);
            actDel->setEnabled(hasSel);
        }
        // 如果在空白处右击
        else {
            auto actPaste = menu.addAction(tr("Paste\tCtrl+V"), this, &FlowView::pasteClipboard);
        }
    }
    
    // 添加视图控制菜单
    menu.addSeparator();
    menu.addAction(tr("Zoom In\tCtrl++"), this, &FlowView::zoomIn);
    menu.addAction(tr("Zoom Out\tCtrl+-"), this, &FlowView::zoomOut);
    menu.addAction(tr("Reset Zoom\tCtrl+0"), this, &FlowView::resetZoom);
    menu.addAction(tr("Fit to Window\tCtrl+F"), this, &FlowView::fitToWindow);
    
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
    else if (type == "diamond") s = std::make_unique<Diamond>();
    else if (type == "triangle") s = std::make_unique<Triangle>();
    else if (type == "pentagon") s = std::make_unique<Pentagon>();
    else if (type == "hexagon") s = std::make_unique<Hexagon>();
    else if (type == "octagon") s = std::make_unique<Octagon>();
    else if (type == "roundedrect") s = std::make_unique<RoundedRect>();
    else if (type == "capsule") s = std::make_unique<Capsule>();
    if (!s) return;
    s->fromJson(obj);
    s->bounds.translate(10, 10);       // ΢ƫ
    shapes_.push_back(std::move(s));
    update();
}

void FlowView::deleteSelection()
{
    if (selectedIndex_ != -1) {
        // 记录删除前的图形状态
        QJsonObject stateBefore = shapes_[selectedIndex_]->toJson();
        int index = selectedIndex_;
        
        // 执行删除
        shapes_.erase(shapes_.begin() + selectedIndex_);
        selectedIndex_ = -1;
        
        // 记录删除操作
        recordAction(ActionType::Delete, index, stateBefore, QJsonObject());
        
        updatePropertyPanel();
        update();
    } else if (selectedConnectorIndex_ != -1) {
        // 记录删除连接线前，首先找出连接线的源和目标图形索引
        int srcIndex = -1, dstIndex = -1;
        for (size_t i = 0; i < shapes_.size(); ++i) {
            if (shapes_[i].get() == connectors_[selectedConnectorIndex_].src) srcIndex = static_cast<int>(i);
            if (shapes_[i].get() == connectors_[selectedConnectorIndex_].dst) dstIndex = static_cast<int>(i);
        }
        
        // 记录删除连接线操作
        int index = selectedConnectorIndex_;
        recordConnectorAction(ActionType::DeleteConn, index, srcIndex, dstIndex);
        
        // 执行删除
        connectors_.erase(connectors_.begin() + selectedConnectorIndex_);
        selectedConnectorIndex_ = -1;
        
        updatePropertyPanel();
        update();
    }
}

/* ======= Z-Order ======= */
void FlowView::bringToFront()
{
    if (selectedIndex_ == -1) return;
    
    // 记录操作前的状态（原来的位置索引）
    QJsonObject before;
    before["index"] = selectedIndex_;
    
    auto tmp = std::move(shapes_[selectedIndex_]);
    shapes_.erase(shapes_.begin() + selectedIndex_);
    shapes_.push_back(std::move(tmp));
    
    // 记录操作后的状态（新的位置索引）
    QJsonObject after;
    after["index"] = static_cast<int>(shapes_.size() - 1);
    
    // 记录层级操作
    recordAction(ActionType::ZOrder, selectedIndex_, before, after);
    
    selectedIndex_ = static_cast<int>(shapes_.size() - 1);
    update();
}

void FlowView::sendToBack()
{
    if (selectedIndex_ == -1) return;
    
    // 记录操作前的状态（原来的位置索引）
    QJsonObject before;
    before["index"] = selectedIndex_;
    
    auto tmp = std::move(shapes_[selectedIndex_]);
    shapes_.erase(shapes_.begin() + selectedIndex_);
    shapes_.insert(shapes_.begin(), std::move(tmp));
    
    // 记录操作后的状态（新的位置索引）
    QJsonObject after;
    after["index"] = 0;
    
    // 记录层级操作
    recordAction(ActionType::ZOrder, selectedIndex_, before, after);
    
    selectedIndex_ = 0;
    update();
}

void FlowView::moveUp()
{
    if (selectedIndex_ == -1 || selectedIndex_ == static_cast<int>(shapes_.size() - 1)) return;
    
    // 记录操作前的状态（原来的位置索引）
    QJsonObject before;
    before["index"] = selectedIndex_;
    
    auto tmp = std::move(shapes_[selectedIndex_]);
    shapes_.erase(shapes_.begin() + selectedIndex_);
    shapes_.insert(shapes_.begin() + selectedIndex_ + 1, std::move(tmp));
    
    // 记录操作后的状态（新的位置索引）
    QJsonObject after;
    after["index"] = selectedIndex_ + 1;
    
    // 记录层级操作
    recordAction(ActionType::ZOrder, selectedIndex_, before, after);
    
    selectedIndex_++;
    update();
}

void FlowView::moveDown()
{
    if (selectedIndex_ <= 0) return;
    
    // 记录操作前的状态（原来的位置索引）
    QJsonObject before;
    before["index"] = selectedIndex_;
    
    auto tmp = std::move(shapes_[selectedIndex_]);
    shapes_.erase(shapes_.begin() + selectedIndex_);
    shapes_.insert(shapes_.begin() + selectedIndex_ - 1, std::move(tmp));
    
    // 记录操作后的状态（新的位置索引）
    QJsonObject after;
    after["index"] = selectedIndex_ - 1;
    
    // 记录层级操作
    recordAction(ActionType::ZOrder, selectedIndex_, before, after);
    
    selectedIndex_--;
    update();
}


//ʵ setter slot
void FlowView::setFill(const QColor& c)
{
    if (selectedIndex_ != -1) {
        QJsonObject before = shapes_[selectedIndex_]->toJson();
        shapes_[selectedIndex_]->fillColor = c;
        QJsonObject after = shapes_[selectedIndex_]->toJson();
        recordAction(ActionType::Property, selectedIndex_, before, after);
        update();
    }
}
void FlowView::setStroke(const QColor& c)
{
    if (selectedIndex_ != -1) {
        QJsonObject before = shapes_[selectedIndex_]->toJson();
        shapes_[selectedIndex_]->strokeColor = c;
        QJsonObject after = shapes_[selectedIndex_]->toJson();
        recordAction(ActionType::Property, selectedIndex_, before, after);
        update();
    }
}
void FlowView::setWidth(qreal w)
{
    if (selectedIndex_ != -1) {
        QJsonObject before = shapes_[selectedIndex_]->toJson();
        shapes_[selectedIndex_]->strokeWidth = w;
        QJsonObject after = shapes_[selectedIndex_]->toJson();
        recordAction(ActionType::Property, selectedIndex_, before, after);
        update();
    }
}


void FlowView::updateConnectorsFor(Shape* movedShape)
{
    if (!movedShape) return;
    
    for (auto& conn : connectors_) {
        if (conn.src == movedShape || conn.dst == movedShape) {
            // 连接线的起点或终点被移动，随之更新连接线
            // 注意：这里不需要做任何事情，因为Connector类在绘制时
            // 会自动根据图形的位置计算连接点
        }
    }
}

void FlowView::mouseDoubleClickEvent(QMouseEvent* e)
{
    // 将视图坐标转换为文档坐标
    QPointF docPos = viewToDoc(e->pos());
    
    // 如果双击了已选中的图形，弹出文本编辑对话框
    if (selectedIndex_ != -1 && shapes_[selectedIndex_]->hitTest(docPos)) {
        TextEditDialog dlg(this);
        dlg.setText(shapes_[selectedIndex_]->text);
        dlg.setTextColor(shapes_[selectedIndex_]->textColor);
        dlg.setTextSize(shapes_[selectedIndex_]->textSize);
        
        if (dlg.exec() == QDialog::Accepted) {
            // 保存修改前的状态
            QJsonObject stateBefore = shapes_[selectedIndex_]->toJson();
            
            // 应用新文本
            shapes_[selectedIndex_]->text = dlg.getText();
            shapes_[selectedIndex_]->textColor = dlg.getTextColor();
            shapes_[selectedIndex_]->textSize = dlg.getTextSize();
            
            // 记录修改历史
            QJsonObject stateAfter = shapes_[selectedIndex_]->toJson();
            recordAction(ActionType::Property, selectedIndex_, stateBefore, stateAfter);
            
            update();
        }
        return;
    }
    
    // 如果双击了连接线，弹出连接线样式对话框
    int connIndex = hitTestConnector(docPos);
    if (connIndex >= 0 && connIndex < static_cast<int>(connectors_.size())) {
        selectedConnectorIndex_ = connIndex;
        
        // 弹出颜色选择器
        QColor color = QColorDialog::getColor(connectors_[connIndex].color, this, tr("Select Connector Color"));
        if (color.isValid()) {
            setConnectorColor(color);
        }
        
        update();
        return;
    }
    
    // 默认处理
    QWidget::mouseDoubleClickEvent(e);
}

// 添加文本颜色设置
void FlowView::setTextColor(const QColor& c)
{
    if (selectedIndex_ == -1 || !c.isValid()) return;
    shapes_[selectedIndex_]->textColor = c;
    // 更新属性面板显示
    updatePropertyPanel();
    update();
}

// 添加文本大小设置
void FlowView::setTextSize(int size)
{
    if (selectedIndex_ == -1 || size <= 0) return;
    shapes_[selectedIndex_]->textSize = size;
    // 更新属性面板显示
    updatePropertyPanel();
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
            connObj["bidirectional"] = conn.bidirectional;
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
            } else if (type == "diamond") {
                shape = std::make_unique<Diamond>();
            } else if (type == "triangle") {
                shape = std::make_unique<Triangle>();
            } else if (type == "pentagon") {
                shape = std::make_unique<Pentagon>();
            } else if (type == "hexagon") {
                shape = std::make_unique<Hexagon>();
            } else if (type == "octagon") {
                shape = std::make_unique<Octagon>();
            } else if (type == "roundedrect") {
                shape = std::make_unique<RoundedRect>();
            } else if (type == "capsule") {
                shape = std::make_unique<Capsule>();
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
                conn.bidirectional = connObj["bidirectional"].toBool(false);
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

// 视图坐标到文档坐标的转换
QPointF FlowView::viewToDoc(const QPointF& viewPoint) const
{
    return (viewPoint - viewOffset_) / scale_;
}

// 文档坐标到视图坐标的转换
QPointF FlowView::docToView(const QPointF& docPoint) const
{
    return docPoint * scale_ + viewOffset_;
}

// 绘制页面边界
void FlowView::drawPageBorder(QPainter& painter)
{
    QRectF pageRect = QRectF(docToView(QPointF(0, 0)), 
                             docToView(QPointF(pageSize_.width(), pageSize_.height())));
    
    // 绘制页面阴影
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 30));
    painter.drawRect(pageRect.translated(5, 5));
    
    // 绘制页面边框
    painter.setPen(QPen(Qt::gray, 1.0));
    painter.setBrush(backgroundColor_);
    painter.drawRect(pageRect);
}

// 鼠标滚轮事件处理
void FlowView::wheelEvent(QWheelEvent* event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Ctrl+滚轮用于缩放
        const qreal zoomFactor = 1.15;
        if (event->angleDelta().y() > 0) {
            // 放大
            scale_ *= zoomFactor;
        } else {
            // 缩小
            scale_ /= zoomFactor;
        }
        
        // 限制缩放范围，防止过大或过小
        scale_ = qBound(0.1, scale_, 5.0);
        
        // 更新视图
        update();
    } else {
        // 普通滚轮用于垂直滚动，Shift+滚轮用于水平滚动
        int dx = 0, dy = 0;
        if (event->modifiers() & Qt::ShiftModifier) {
            dx = -event->angleDelta().y();
        } else {
            dy = -event->angleDelta().y();
        }
        
        viewOffset_ += QPointF(dx, dy) * 0.5;
        update();
    }
    
    event->accept();
}

// 键盘事件处理
void FlowView::keyPressEvent(QKeyEvent* event)
{
    const int step = 20;
    
    switch (event->key()) {
        case Qt::Key_Escape:
            // Escape键用于取消当前操作，回到选择模式
            if (mode_ != ToolMode::None) {
                mode_ = ToolMode::None;
                currentConn_ = Connector{};
                setCursor(Qt::ArrowCursor);
                update();
                event->accept();
                return;
            }
            break;
            
        case Qt::Key_Z:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+Z 撤销
                if (event->modifiers() & Qt::ShiftModifier) {
                    // Ctrl+Shift+Z 重做
                    redo();
                } else {
                    // Ctrl+Z 撤销
                    undo();
                }
                event->accept();
                return;
            }
            break;
            
        case Qt::Key_Y:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+Y 重做
                redo();
                event->accept();
                return;
            }
            break;
            
        case Qt::Key_Plus:
        case Qt::Key_Equal:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+加号 放大
                zoomIn();
                event->accept();
                return;
            }
            break;
            
        case Qt::Key_Minus:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+减号 缩小
                zoomOut();
                event->accept();
                return;
            }
            break;
            
        case Qt::Key_0:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+0 重置缩放
                resetZoom();
                event->accept();
                return;
            }
            break;
            
        case Qt::Key_F:
            if (event->modifiers() & Qt::ControlModifier) {
                // Ctrl+F 适应窗口
                fitToWindow();
                event->accept();
                return;
            }
            break;
            
        case Qt::Key_Left:
            viewOffset_.rx() += step;
            update();
            event->accept();
            return;
            
        case Qt::Key_Right:
            viewOffset_.rx() -= step;
            update();
            event->accept();
            return;
            
        case Qt::Key_Up:
            viewOffset_.ry() += step;
            update();
            event->accept();
            return;
            
        case Qt::Key_Down:
            viewOffset_.ry() -= step;
            update();
            event->accept();
            return;
            
        case Qt::Key_Space:
            // 空格键是平移模式的开关
            setCursor(isPanning_ ? Qt::ArrowCursor : Qt::OpenHandCursor);
            isPanning_ = !isPanning_;
            event->accept();
            return;
    }
    
    QWidget::keyPressEvent(event);
}

// 实现缩放相关方法
void FlowView::zoomIn()
{
    scale_ *= 1.2;
    scale_ = qBound(0.1, scale_, 5.0);
    update();
}

void FlowView::zoomOut()
{
    scale_ /= 1.2;
    scale_ = qBound(0.1, scale_, 5.0);
    update();
}

void FlowView::resetZoom()
{
    scale_ = 1.0;
    viewOffset_ = QPointF(0, 0);
    update();
}

void FlowView::fitToWindow()
{
    // 计算合适的缩放比例和偏移量，使页面正好适合视图
    qreal scaleX = width() / (pageSize_.width() + 40.0);
    qreal scaleY = height() / (pageSize_.height() + 40.0);
    scale_ = qMin(scaleX, scaleY);
    
    // 居中显示
    viewOffset_ = QPointF((width() - pageSize_.width() * scale_) / 2,
                         (height() - pageSize_.height() * scale_) / 2);
    update();
}

// 绘制调整大小的控制柄
void FlowView::drawResizeHandles(QPainter& painter, const QRectF& rect)
{
    const int handleSize = 8;
    
    // 保存原来的笔和画刷
    QPen oldPen = painter.pen();
    QBrush oldBrush = painter.brush();
    
    // 设置控制柄样式
    painter.setPen(QPen(Qt::blue, 1));
    painter.setBrush(Qt::white);
    
    // 定义8个控制点位置
    QPointF handles[8] = {
        rect.topLeft(),                                    // 左上
        QPointF(rect.left() + rect.width()/2, rect.top()), // 上中
        rect.topRight(),                                   // 右上
        QPointF(rect.left(), rect.top() + rect.height()/2),// 左中
        QPointF(rect.right(), rect.top() + rect.height()/2),// 右中
        rect.bottomLeft(),                                 // 左下
        QPointF(rect.left() + rect.width()/2, rect.bottom()),// 下中
        rect.bottomRight()                                 // 右下
    };
    
    // 绘制8个控制点
    for (const QPointF& p : handles) {
        painter.drawRect(QRectF(p.x() - handleSize/2, p.y() - handleSize/2, handleSize, handleSize));
    }
    
    // 恢复原来的笔和画刷
    painter.setPen(oldPen);
    painter.setBrush(oldBrush);
}

// 检测点击在哪个调整柄上
FlowView::ResizeHandle FlowView::hitTestResizeHandles(const QPointF& docPoint, const QRectF& rect)
{
    const int handleSize = 8;
    
    // 定义8个控制点及其对应的ResizeHandle
    struct HandleInfo {
        QPointF pos;
        ResizeHandle handle;
    };
    
    HandleInfo handles[8] = {
        {rect.topLeft(), ResizeHandle::TopLeft},
        {QPointF(rect.left() + rect.width()/2, rect.top()), ResizeHandle::TopCenter},
        {rect.topRight(), ResizeHandle::TopRight},
        {QPointF(rect.left(), rect.top() + rect.height()/2), ResizeHandle::MiddleLeft},
        {QPointF(rect.right(), rect.top() + rect.height()/2), ResizeHandle::MiddleRight},
        {rect.bottomLeft(), ResizeHandle::BottomLeft},
        {QPointF(rect.left() + rect.width()/2, rect.bottom()), ResizeHandle::BottomCenter},
        {rect.bottomRight(), ResizeHandle::BottomRight}
    };
    
    // 检查点击是否在任何控制柄上
    for (const HandleInfo& info : handles) {
        QRectF handleRect(
            info.pos.x() - handleSize/2,
            info.pos.y() - handleSize/2,
            handleSize,
            handleSize
        );
        
        if (handleRect.contains(docPoint)) {
            return info.handle;
        }
    }
    
    return ResizeHandle::None;
}

// 调整矩形大小
void FlowView::resizeRect(QRectF& rect, ResizeHandle handle, const QPointF& offset)
{
    QRectF newRect = rect;
    
    switch (handle) {
        case ResizeHandle::TopLeft:
            newRect.setTopLeft(rect.topLeft() + offset);
            break;
            
        case ResizeHandle::TopCenter:
            newRect.setTop(rect.top() + offset.y());
            break;
            
        case ResizeHandle::TopRight:
            newRect.setTopRight(rect.topRight() + offset);
            break;
            
        case ResizeHandle::MiddleLeft:
            newRect.setLeft(rect.left() + offset.x());
            break;
            
        case ResizeHandle::MiddleRight:
            newRect.setRight(rect.right() + offset.x());
            break;
            
        case ResizeHandle::BottomLeft:
            newRect.setBottomLeft(rect.bottomLeft() + offset);
            break;
            
        case ResizeHandle::BottomCenter:
            newRect.setBottom(rect.bottom() + offset.y());
            break;
            
        case ResizeHandle::BottomRight:
            newRect.setBottomRight(rect.bottomRight() + offset);
            break;
            
        default:
            return;
    }
    
    // 确保矩形有最小尺寸并且宽高不为负数
    if (newRect.width() < 10) {
        if (newRect.left() != rect.left()) {
            newRect.setLeft(newRect.right() - 10);
        } else {
            newRect.setRight(newRect.left() + 10);
        }
    }
    
    if (newRect.height() < 10) {
        if (newRect.top() != rect.top()) {
            newRect.setTop(newRect.bottom() - 10);
        } else {
            newRect.setBottom(newRect.top() + 10);
        }
    }
    
    rect = newRect;
}

// 更新属性面板显示，包括尺寸属性
void FlowView::updatePropertyPanel()
{
    // 如果选中了连接线
    if (selectedConnectorIndex_ != -1) {
        auto& conn = connectors_[selectedConnectorIndex_];
        emit shapeAttr({}, conn.color, conn.width);
        emit shapeSize(0, 0);  // 连接线没有尺寸属性
        emit connectorColorChanged(conn.color);  // 发送连接线颜色
        return;
    }

    // 如果选中了图形
    if (selectedIndex_ != -1) {
        auto* shape = shapes_[selectedIndex_].get();
        emit shapeAttr(shape->fillColor, shape->strokeColor, shape->strokeWidth);
        
        // 发送尺寸信息
        int width = shape->bounds.width();
        int height = shape->bounds.height();
        emit shapeSize(width, height);
        
        // 发送文本属性信息
        emit textColorChanged(shape->textColor);
        emit textSizeChanged(shape->textSize);
        
        // 未选中连接线时，发送空颜色
        emit connectorColorChanged(QColor());
        
        return;
    }
    
    // 如果没有选中任何内容
    emit shapeAttr({}, {}, -1);
    emit shapeSize(0, 0);
    emit connectorColorChanged(QColor());
}

// 设置对象宽度
void FlowView::setObjectWidth(int width)
{
    if (selectedIndex_ == -1 || width <= 0) return;
    
    // 获取当前矩形
    auto& bounds = shapes_[selectedIndex_]->bounds;
    
    // 计算新宽度，保持左边缘不变
    QRectF newBounds = bounds;
    newBounds.setWidth(width);
    
    // 设置新矩形
    shapes_[selectedIndex_]->bounds = newBounds;
    
    // 更新连接器
    updateConnectorsFor(shapes_[selectedIndex_].get());
    
    update();
}

// 设置对象高度
void FlowView::setObjectHeight(int height)
{
    if (selectedIndex_ == -1 || height <= 0) return;
    
    // 获取当前矩形
    auto& bounds = shapes_[selectedIndex_]->bounds;
    
    // 计算新高度，保持顶边不变
    QRectF newBounds = bounds;
    newBounds.setHeight(height);
    
    // 设置新矩形
    shapes_[selectedIndex_]->bounds = newBounds;
    
    // 更新连接器
    updateConnectorsFor(shapes_[selectedIndex_].get());
    
    update();
}

void FlowView::setToolMode(ToolMode m)
{
    // 切换前先清除当前绘制状态
    if (m != mode_) {
        currentConn_ = Connector{};
    }
    
    mode_ = m;
    
    // 如果进入连接器模式，显示提示光标
    if (m == ToolMode::DrawConnector) {
        setCursor(Qt::CrossCursor);
    } else {
        setCursor(Qt::ArrowCursor);
    }
    
    update();
}

// 查找点击了哪个连接线
int FlowView::hitTestConnector(const QPointF& pt) const
{
    const double hitDistance = 12.0; // 增大点击误差范围，更容易选中(从8.0改为12.0)
    
    for (int i = 0; i < static_cast<int>(connectors_.size()); ++i) {
        const Connector& conn = connectors_[i];
        if (!conn.src || !conn.dst) continue;
        
        // 获取连接线的两个端点
        QPointF p1 = conn.src->getConnectionPoint(conn.dst->bounds.center());
        QPointF p2 = conn.dst->getConnectionPoint(p1);
        // 再次调整起点
        p1 = conn.src->getConnectionPoint(p2);
        
        // 计算点到直线的距离
        QLineF line(p1, p2);
        QPointF v = line.p2() - line.p1();
        double len = std::sqrt(v.x() * v.x() + v.y() * v.y());
        
        if (len < 1e-6) continue; // 防止除以0
        
        // 标准化方向向量
        v /= len;
        
        // 计算pt到线段p1-p2的投影
        QPointF w = pt - line.p1();
        double proj = QPointF::dotProduct(w, v);
        
        // 如果投影在线段范围外，检查是否在箭头附近
        if (proj < 0 || proj > len) {
            // 检查是否在箭头附近 (箭头位于p2点)
            double distanceToArrow = QLineF(pt, p2).length();
            if (distanceToArrow <= hitDistance * 2.5) { // 箭头区域用更大的检测范围(从2倍改为2.5倍)
                return i;
            }
            
            // 如果是双向箭头，也检查起点箭头
            if (conn.bidirectional) {
                double distanceToStartArrow = QLineF(pt, p1).length();
                if (distanceToStartArrow <= hitDistance * 2.5) {
                    return i;
                }
            }
            
            continue;
        }
        
        // 计算点到直线的垂直距离
        QPointF projPoint = line.p1() + v * proj;
        double distance = QLineF(pt, projPoint).length();
        
        // 如果距离足够小，说明点击在这条线上
        if (distance <= hitDistance) {
            return i;
        }
    }
    
    return -1;
}

// 设置连接线为双向箭头
void FlowView::setConnectorBidirectional(bool bidirectional)
{
    if (selectedConnectorIndex_ >= 0 && selectedConnectorIndex_ < connectors_.size()) {
        // 记录修改前的状态
        QJsonObject before;
        before["bidirectional"] = connectors_[selectedConnectorIndex_].bidirectional;
        before["color"] = connectors_[selectedConnectorIndex_].color.name();
        before["width"] = connectors_[selectedConnectorIndex_].width;
        
        // 执行修改
        connectors_[selectedConnectorIndex_].bidirectional = bidirectional;
        
        // 记录修改后的状态
        QJsonObject after;
        after["bidirectional"] = connectors_[selectedConnectorIndex_].bidirectional;
        after["color"] = connectors_[selectedConnectorIndex_].color.name();
        after["width"] = connectors_[selectedConnectorIndex_].width;
        
        // 查找连接线的源和目标图形索引
        int srcIndex = -1, dstIndex = -1;
        for (size_t i = 0; i < shapes_.size(); ++i) {
            if (shapes_[i].get() == connectors_[selectedConnectorIndex_].src) srcIndex = i;
            if (shapes_[i].get() == connectors_[selectedConnectorIndex_].dst) dstIndex = i;
        }
        
        // 记录属性修改操作
        ActionRecord record;
        record.type = ActionType::Property;
        record.elementIndex = selectedConnectorIndex_;
        record.stateBefore = before;
        record.stateAfter = after;
        record.srcIndex = srcIndex;
        record.dstIndex = dstIndex;
        
        if (!isUndoRedoing_) {
            undoStack_.push(record);
            clearRedoHistory();
        }
        
        update();
    }
}

// 切换连接线箭头方向
void FlowView::toggleConnectorDirection()
{
    if (selectedConnectorIndex_ >= 0 && selectedConnectorIndex_ < connectors_.size()) {
        Connector& conn = connectors_[selectedConnectorIndex_];
        
        // 记录修改前的状态和源目标索引
        QJsonObject before;
        before["bidirectional"] = conn.bidirectional;
        before["color"] = conn.color.name();
        before["width"] = conn.width;
        
        int oldSrcIndex = -1, oldDstIndex = -1;
        for (size_t i = 0; i < shapes_.size(); ++i) {
            if (shapes_[i].get() == conn.src) oldSrcIndex = i;
            if (shapes_[i].get() == conn.dst) oldDstIndex = i;
        }
        
        // 交换起点和终点
        std::swap(conn.src, conn.dst);
        
        // 记录修改后的状态和新的源目标索引
        QJsonObject after;
        after["bidirectional"] = conn.bidirectional;
        after["color"] = conn.color.name();
        after["width"] = conn.width;
        
        int newSrcIndex = -1, newDstIndex = -1;
        for (size_t i = 0; i < shapes_.size(); ++i) {
            if (shapes_[i].get() == conn.src) newSrcIndex = i;
            if (shapes_[i].get() == conn.dst) newDstIndex = i;
        }
        
        // 记录属性修改操作
        ActionRecord record;
        record.type = ActionType::Property;
        record.elementIndex = selectedConnectorIndex_;
        record.stateBefore = before;
        record.stateAfter = after;
        record.srcIndex = oldSrcIndex;
        record.dstIndex = oldDstIndex;
        
        if (!isUndoRedoing_) {
            undoStack_.push(record);
            clearRedoHistory();
        }
        
        update();
    }
}

// 添加连接线颜色设置
void FlowView::setConnectorColor(const QColor& c)
{
    if (selectedConnectorIndex_ == -1 || !c.isValid()) return;
    
    // 记录修改前的状态
    QJsonObject before;
    before["bidirectional"] = connectors_[selectedConnectorIndex_].bidirectional;
    before["color"] = connectors_[selectedConnectorIndex_].color.name();
    before["width"] = connectors_[selectedConnectorIndex_].width;
    
    // 执行修改
    connectors_[selectedConnectorIndex_].color = c;
    
    // 记录修改后的状态
    QJsonObject after;
    after["bidirectional"] = connectors_[selectedConnectorIndex_].bidirectional;
    after["color"] = connectors_[selectedConnectorIndex_].color.name();
    after["width"] = connectors_[selectedConnectorIndex_].width;
    
    // 查找连接线的源和目标图形索引
    int srcIndex = -1, dstIndex = -1;
    for (size_t i = 0; i < shapes_.size(); ++i) {
        if (shapes_[i].get() == connectors_[selectedConnectorIndex_].src) srcIndex = i;
        if (shapes_[i].get() == connectors_[selectedConnectorIndex_].dst) dstIndex = i;
    }
    
    // 记录属性修改操作
    ActionRecord record;
    record.type = ActionType::Property;
    record.elementIndex = selectedConnectorIndex_;
    record.stateBefore = before;
    record.stateAfter = after;
    record.srcIndex = srcIndex;
    record.dstIndex = dstIndex;
    
    if (!isUndoRedoing_) {
        undoStack_.push(record);
        clearRedoHistory();
    }
    
    // 更新UI
    emit connectorColorChanged(c);
    update();
}

// 记录操作历史
void FlowView::recordAction(ActionType type, int elementIndex, const QJsonObject& before, const QJsonObject& after)
{
    if (isUndoRedoing_) return; // 如果是在执行撤销/重做操作，不记录
    
    ActionRecord record;
    record.type = type;
    record.elementIndex = elementIndex;
    record.stateBefore = before;
    record.stateAfter = after;
    
    undoStack_.push(record);
    clearRedoHistory(); // 有新操作时清空重做历史
}

// 记录连接线操作历史
void FlowView::recordConnectorAction(ActionType type, int connIndex, int srcIndex, int dstIndex)
{
    if (isUndoRedoing_) return;
    
    ActionRecord record;
    record.type = type;
    record.elementIndex = connIndex;
    record.srcIndex = srcIndex;
    record.dstIndex = dstIndex;
    
    if (type == ActionType::AddConn) {
        // 添加连接线操作，记录连接线的属性
        if (connIndex >= 0 && connIndex < connectors_.size()) {
            QJsonObject connObj;
            connObj["color"] = connectors_[connIndex].color.name();
            connObj["width"] = connectors_[connIndex].width;
            connObj["bidirectional"] = connectors_[connIndex].bidirectional;
            record.stateAfter = connObj;
        }
    } else if (type == ActionType::DeleteConn) {
        // 删除连接线操作，记录被删除连接线的属性
        if (connIndex >= 0 && connIndex < connectors_.size()) {
            QJsonObject connObj;
            connObj["color"] = connectors_[connIndex].color.name();
            connObj["width"] = connectors_[connIndex].width;
            connObj["bidirectional"] = connectors_[connIndex].bidirectional;
            record.stateBefore = connObj;
        }
    }
    
    undoStack_.push(record);
    clearRedoHistory();
}

// 清空重做历史
void FlowView::clearRedoHistory()
{
    while (!redoStack_.empty()) {
        redoStack_.pop();
    }
}

// 撤销操作
void FlowView::undo()
{
    if (undoStack_.empty()) return;
    
    isUndoRedoing_ = true;
    ActionRecord record = undoStack_.top();
    undoStack_.pop();
    
    switch (record.type) {
        case ActionType::Add:
            // 撤销添加图形操作（删除图形）
            if (record.elementIndex >= 0 && record.elementIndex < shapes_.size()) {
                shapes_.erase(shapes_.begin() + record.elementIndex);
                if (selectedIndex_ == record.elementIndex) {
                    selectedIndex_ = -1;
                } else if (selectedIndex_ > record.elementIndex) {
                    selectedIndex_--;
                }
            }
            break;
            
        case ActionType::Delete:
            // 撤销删除图形操作（重新添加图形）
            {
                std::unique_ptr<Shape> s;
                QString type = record.stateBefore["type"].toString();
                if (type == "rect")         s = std::make_unique<Rect>();
                else if (type == "ellipse") s = std::make_unique<Ellipse>();
                else if (type == "diamond") s = std::make_unique<Diamond>();
                else if (type == "triangle") s = std::make_unique<Triangle>();
                else if (type == "pentagon") s = std::make_unique<Pentagon>();
                else if (type == "hexagon") s = std::make_unique<Hexagon>();
                else if (type == "octagon") s = std::make_unique<Octagon>();
                else if (type == "roundedrect") s = std::make_unique<RoundedRect>();
                else if (type == "capsule") s = std::make_unique<Capsule>();
                
                if (s) {
                    s->fromJson(record.stateBefore);
                    if (record.elementIndex >= 0 && record.elementIndex <= shapes_.size()) {
                        shapes_.insert(shapes_.begin() + record.elementIndex, std::move(s));
                        if (selectedIndex_ >= record.elementIndex) {
                            selectedIndex_++;
                        }
                    } else {
                        shapes_.push_back(std::move(s));
                    }
                }
            }
            break;
        
        case ActionType::Move:
        case ActionType::Resize:
        case ActionType::Property:
            // 撤销移动/调整大小/属性修改操作（恢复到之前的状态）
            if (record.elementIndex >= 0 && record.elementIndex < shapes_.size()) {
                QString type = shapes_[record.elementIndex]->toJson()["type"].toString();
                std::unique_ptr<Shape> s;
                if (type == "rect")         s = std::make_unique<Rect>();
                else if (type == "ellipse") s = std::make_unique<Ellipse>();
                else if (type == "diamond") s = std::make_unique<Diamond>();
                else if (type == "triangle") s = std::make_unique<Triangle>();
                else if (type == "pentagon") s = std::make_unique<Pentagon>();
                else if (type == "hexagon") s = std::make_unique<Hexagon>();
                else if (type == "octagon") s = std::make_unique<Octagon>();
                else if (type == "roundedrect") s = std::make_unique<RoundedRect>();
                else if (type == "capsule") s = std::make_unique<Capsule>();
                
                if (s) {
                    s->fromJson(record.stateBefore);
                    Shape* oldShape = shapes_[record.elementIndex].get();
                    shapes_[record.elementIndex] = std::move(s);
                    
                    // 更新所有指向这个图形的连接线
                    for (auto& conn : connectors_) {
                        if (conn.src == oldShape) {
                            conn.src = shapes_[record.elementIndex].get();
                        }
                        if (conn.dst == oldShape) {
                            conn.dst = shapes_[record.elementIndex].get();
                        }
                    }
                }
            } else if (record.elementIndex >= 0 && record.elementIndex < connectors_.size()) {
                // 处理连接线属性的撤销
                if (record.stateBefore.contains("color"))
                    connectors_[record.elementIndex].color = QColor(record.stateBefore["color"].toString());
                if (record.stateBefore.contains("width"))
                    connectors_[record.elementIndex].width = record.stateBefore["width"].toDouble(2.0);
                if (record.stateBefore.contains("bidirectional"))
                    connectors_[record.elementIndex].bidirectional = record.stateBefore["bidirectional"].toBool();
                
                // 如果源和目标发生了变化
                if (record.srcIndex >= 0 && record.srcIndex < shapes_.size())
                    connectors_[record.elementIndex].src = shapes_[record.srcIndex].get();
                if (record.dstIndex >= 0 && record.dstIndex < shapes_.size())
                    connectors_[record.elementIndex].dst = shapes_[record.dstIndex].get();
            }
            break;
            
        case ActionType::ZOrder:
            // 撤销层级调整操作
            if (record.stateBefore.contains("index") && record.stateAfter.contains("index")) {
                int oldIndex = record.stateBefore["index"].toInt();
                int newIndex = record.stateAfter["index"].toInt();
                
                if (newIndex >= 0 && newIndex < shapes_.size()) {
                    // 将图形从当前位置移回原来的位置
                    auto tmp = std::move(shapes_[newIndex]);
                    shapes_.erase(shapes_.begin() + newIndex);
                    
                    // 确保oldIndex在有效范围内
                    int insertPos = qBound(0, oldIndex, static_cast<int>(shapes_.size()));
                    shapes_.insert(shapes_.begin() + insertPos, std::move(tmp));
                    
                    // 更新选中索引
                    selectedIndex_ = insertPos;
                }
            }
            break;
            
        case ActionType::AddConn:
            // 撤销添加连接线操作（删除连接线）
            if (record.elementIndex >= 0 && record.elementIndex < connectors_.size()) {
                connectors_.erase(connectors_.begin() + record.elementIndex);
                if (selectedConnectorIndex_ == record.elementIndex) {
                    selectedConnectorIndex_ = -1;
                } else if (selectedConnectorIndex_ > record.elementIndex) {
                    selectedConnectorIndex_--;
                }
            }
            break;
            
        case ActionType::DeleteConn:
            // 撤销删除连接线操作（重新添加连接线）
            if (record.srcIndex >= 0 && record.srcIndex < shapes_.size() &&
                record.dstIndex >= 0 && record.dstIndex < shapes_.size()) {
                Connector conn;
                conn.src = shapes_[record.srcIndex].get();
                conn.dst = shapes_[record.dstIndex].get();
                
                // 恢复连接线属性
                if (record.stateBefore.contains("color"))
                    conn.color = QColor(record.stateBefore["color"].toString());
                if (record.stateBefore.contains("width"))
                    conn.width = record.stateBefore["width"].toDouble(2.0);
                if (record.stateBefore.contains("bidirectional"))
                    conn.bidirectional = record.stateBefore["bidirectional"].toBool();
                
                if (record.elementIndex >= 0 && record.elementIndex <= connectors_.size()) {
                    connectors_.insert(connectors_.begin() + record.elementIndex, conn);
                    if (selectedConnectorIndex_ >= record.elementIndex) {
                        selectedConnectorIndex_++;
                    }
                } else {
                    connectors_.push_back(conn);
                }
            }
            break;
    }
    
    // 将动作放入重做栈
    redoStack_.push(record);
    
    // 更新UI
    updatePropertyPanel();
    update();
    isUndoRedoing_ = false;
}

// 重做操作
void FlowView::redo()
{
    if (redoStack_.empty()) return;
    
    isUndoRedoing_ = true;
    ActionRecord record = redoStack_.top();
    redoStack_.pop();
    
    switch (record.type) {
        case ActionType::Add:
            // 重做添加图形操作
            {
                std::unique_ptr<Shape> s;
                QString type = record.stateAfter["type"].toString();
                if (type == "rect")         s = std::make_unique<Rect>();
                else if (type == "ellipse") s = std::make_unique<Ellipse>();
                else if (type == "diamond") s = std::make_unique<Diamond>();
                else if (type == "triangle") s = std::make_unique<Triangle>();
                else if (type == "pentagon") s = std::make_unique<Pentagon>();
                else if (type == "hexagon") s = std::make_unique<Hexagon>();
                else if (type == "octagon") s = std::make_unique<Octagon>();
                else if (type == "roundedrect") s = std::make_unique<RoundedRect>();
                else if (type == "capsule") s = std::make_unique<Capsule>();
                
                if (s) {
                    s->fromJson(record.stateAfter);
                    if (record.elementIndex >= 0 && record.elementIndex <= shapes_.size()) {
                        shapes_.insert(shapes_.begin() + record.elementIndex, std::move(s));
                        if (selectedIndex_ >= record.elementIndex) {
                            selectedIndex_++;
                        }
                    } else {
                        shapes_.push_back(std::move(s));
                    }
                }
            }
            break;
            
        case ActionType::Delete:
            // 重做删除图形操作
            if (record.elementIndex >= 0 && record.elementIndex < shapes_.size()) {
                shapes_.erase(shapes_.begin() + record.elementIndex);
                if (selectedIndex_ == record.elementIndex) {
                    selectedIndex_ = -1;
                } else if (selectedIndex_ > record.elementIndex) {
                    selectedIndex_--;
                }
            }
            break;
            
        case ActionType::Move:
        case ActionType::Resize:
        case ActionType::Property:
            // 重做移动/调整大小/属性修改操作
            if (record.elementIndex >= 0 && record.elementIndex < shapes_.size()) {
                QString type = shapes_[record.elementIndex]->toJson()["type"].toString();
                std::unique_ptr<Shape> s;
                if (type == "rect")         s = std::make_unique<Rect>();
                else if (type == "ellipse") s = std::make_unique<Ellipse>();
                else if (type == "diamond") s = std::make_unique<Diamond>();
                else if (type == "triangle") s = std::make_unique<Triangle>();
                else if (type == "pentagon") s = std::make_unique<Pentagon>();
                else if (type == "hexagon") s = std::make_unique<Hexagon>();
                else if (type == "octagon") s = std::make_unique<Octagon>();
                else if (type == "roundedrect") s = std::make_unique<RoundedRect>();
                else if (type == "capsule") s = std::make_unique<Capsule>();
                
                if (s) {
                    s->fromJson(record.stateAfter);
                    Shape* oldShape = shapes_[record.elementIndex].get();
                    shapes_[record.elementIndex] = std::move(s);
                    
                    // 更新所有指向这个图形的连接线
                    for (auto& conn : connectors_) {
                        if (conn.src == oldShape) {
                            conn.src = shapes_[record.elementIndex].get();
                        }
                        if (conn.dst == oldShape) {
                            conn.dst = shapes_[record.elementIndex].get();
                        }
                    }
                }
            } else if (record.elementIndex >= 0 && record.elementIndex < connectors_.size()) {
                // 处理连接线属性的重做
                if (record.stateAfter.contains("color"))
                    connectors_[record.elementIndex].color = QColor(record.stateAfter["color"].toString());
                if (record.stateAfter.contains("width"))
                    connectors_[record.elementIndex].width = record.stateAfter["width"].toDouble(2.0);
                if (record.stateAfter.contains("bidirectional"))
                    connectors_[record.elementIndex].bidirectional = record.stateAfter["bidirectional"].toBool();
                
                // 如果源和目标发生了变化
                if (record.srcIndex >= 0 && record.srcIndex < shapes_.size())
                    connectors_[record.elementIndex].src = shapes_[record.srcIndex].get();
                if (record.dstIndex >= 0 && record.dstIndex < shapes_.size())
                    connectors_[record.elementIndex].dst = shapes_[record.dstIndex].get();
            }
            break;
            
        case ActionType::ZOrder:
            // 重做层级调整操作
            if (record.stateBefore.contains("index") && record.stateAfter.contains("index")) {
                int oldIndex = record.stateBefore["index"].toInt();
                int newIndex = record.stateAfter["index"].toInt();
                
                if (oldIndex >= 0 && oldIndex < shapes_.size()) {
                    // 将图形从原来位置移动到新位置
                    auto tmp = std::move(shapes_[oldIndex]);
                    shapes_.erase(shapes_.begin() + oldIndex);
                    
                    // 确保newIndex在有效范围内
                    int insertPos = qBound(0, newIndex, static_cast<int>(shapes_.size()));
                    shapes_.insert(shapes_.begin() + insertPos, std::move(tmp));
                    
                    // 更新选中索引
                    selectedIndex_ = insertPos;
                }
            }
            break;
            
        case ActionType::AddConn:
            // 重做添加连接线操作
            if (record.srcIndex >= 0 && record.srcIndex < shapes_.size() &&
                record.dstIndex >= 0 && record.dstIndex < shapes_.size()) {
                Connector conn;
                conn.src = shapes_[record.srcIndex].get();
                conn.dst = shapes_[record.dstIndex].get();
                
                // 恢复连接线属性
                if (record.stateAfter.contains("color"))
                    conn.color = QColor(record.stateAfter["color"].toString());
                if (record.stateAfter.contains("width"))
                    conn.width = record.stateAfter["width"].toDouble(2.0);
                if (record.stateAfter.contains("bidirectional"))
                    conn.bidirectional = record.stateAfter["bidirectional"].toBool();
                
                if (record.elementIndex >= 0 && record.elementIndex <= connectors_.size()) {
                    connectors_.insert(connectors_.begin() + record.elementIndex, conn);
                    if (selectedConnectorIndex_ >= record.elementIndex) {
                        selectedConnectorIndex_++;
                    }
                } else {
                    connectors_.push_back(conn);
                }
            }
            break;
            
        case ActionType::DeleteConn:
            // 重做删除连接线操作
            if (record.elementIndex >= 0 && record.elementIndex < connectors_.size()) {
                connectors_.erase(connectors_.begin() + record.elementIndex);
                if (selectedConnectorIndex_ == record.elementIndex) {
                    selectedConnectorIndex_ = -1;
                } else if (selectedConnectorIndex_ > record.elementIndex) {
                    selectedConnectorIndex_--;
                }
            }
            break;
    }
    
    // 将动作放回撤销栈
    undoStack_.push(record);
    
    // 更新UI
    updatePropertyPanel();
    update();
    isUndoRedoing_ = false;
}
