#pragma once
#include <QWidget>
#include <vector>
#include <memory>

#include "model/Shape.hpp"
#include "model/Rect.hpp"
#include "model/Ellipse.hpp"
#include "model/Connector.hpp"     // ① 连接线类

class FlowView : public QWidget
{
    Q_OBJECT

signals:
    void shapeAttr(const QColor& fill, const QColor& stroke, qreal width);

public:
    explicit FlowView(QWidget* parent = nullptr);

    /* ---------- 工具模式 ---------- */
    enum class ToolMode { None, DrawRect, DrawEllipse, DrawConnector };
    void setToolMode(ToolMode m) { mode_ = m; }

    /* ---------- 剪贴板 / Z-Order 接口 ---------- */
public slots:
    void copySelection();
    void cutSelection();
    void pasteClipboard();
    void deleteSelection();

    void setFill(const QColor& c);
    void setStroke(const QColor& c);
    void setWidth(qreal w);

    void updateConnectorsFor(Shape* movedShape);


    void bringToFront();
    void sendToBack();
    void moveUp();
    void moveDown();
    void clearSelection() { selectedIndex_ = -1; update(); }


protected:
    /* ---------- Qt 事件 ---------- */
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;

private:
    /* ---------- 数据成员 ---------- */
    ToolMode mode_ = ToolMode::None;

    std::vector<std::unique_ptr<Shape>> shapes_; // ② 所有图形
    std::vector<Connector> connectors_;          // ③ 所有连线
    Connector currentConn_;                      // ④ 正在绘制的临时连线

    int     selectedIndex_ = -1;
    QPointF dragStart_;
};
