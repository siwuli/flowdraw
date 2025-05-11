#pragma once
#include <QWidget>
#include <vector>
#include <memory>
#include "model/Shape.hpp"
#include "model/Rect.hpp"
#include "model/Ellipse.hpp"
#include <QDragEnterEvent>
#include <QDropEvent>


class FlowView : public QWidget
{
    Q_OBJECT
public:
    explicit FlowView(QWidget* parent = nullptr);

    /* ---------- 工具模式 ---------- */
    enum class ToolMode { None, DrawRect, DrawEllipse, DrawConnector };
    void setToolMode(ToolMode m) { mode_ = m; }

    /* ---------- 剪贴板 & 层次接口 ---------- */
public slots:                       // ← 让 QShortcut 能直接连接
    void copySelection();
    void cutSelection();
    void pasteClipboard();
    void deleteSelection();

    void bringToFront();
    void sendToBack();
    void moveUp();
    void moveDown();

protected:
    void paintEvent(QPaintEvent* e) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void contextMenuEvent(QContextMenuEvent* e) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dropEvent(QDropEvent* e) override;


private:
    ToolMode mode_ = ToolMode::None;

    QPointF dragStart_;
    int     selectedIndex_ = -1;
    std::vector<std::unique_ptr<Shape>> shapes_;
};
