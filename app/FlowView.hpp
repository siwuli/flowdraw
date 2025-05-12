#pragma once
#include <QWidget>
#include <vector>
#include <memory>

#include "model/Shape.hpp"
#include "model/Rect.hpp"
#include "model/Ellipse.hpp"
#include "model/Connector.hpp"     // 所有连接线

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

    /* ---------- 文件操作 ---------- */
public:
    // 保存当前绘图到文件
    bool saveToFile(const QString& filename);
    // 从文件加载绘图
    bool loadFromFile(const QString& filename);
    // 导出为PNG图片
    bool exportToPng(const QString& filename);
    // 导出为SVG
    bool exportToSvg(const QString& filename);
    // 清空当前所有内容
    void clearAll();
    
    /* ---------- 页面设置 ---------- */
    void setBackgroundColor(const QColor& color);
    void setPageSize(int width, int height);
    void setGridVisible(bool visible);
    
    QColor backgroundColor() const { return backgroundColor_; }
    QSize pageSize() const { return pageSize_; }
    bool isGridVisible() const { return showGrid_; }

    /* ---------- 编辑器 / Z-Order 接口 ---------- */
public slots:
    void copySelection();
    void cutSelection();
    void pasteClipboard();
    void deleteSelection();

    void setFill(const QColor& c);
    void setStroke(const QColor& c);
    void setWidth(qreal w);
    
    // 文本属性设置
    void setTextColor(const QColor& c);
    void setTextSize(int size);
    void setText(const QString& text);

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
    void mouseDoubleClickEvent(QMouseEvent*) override;

    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;

private:
    /* ---------- 数据成员 ---------- */
    ToolMode mode_ = ToolMode::None;

    std::vector<std::unique_ptr<Shape>> shapes_; // 所有图形元素
    std::vector<Connector> connectors_;          // 所有连接线
    Connector currentConn_;                      // 当前正在绘制的临时连接线

    int     selectedIndex_ = -1;
    QPointF dragStart_;
    
    // 页面设置
    QColor backgroundColor_ = QColor("#fdfdfd");  // 背景颜色
    QSize pageSize_ = QSize(2000, 2000);          // 页面大小
    bool showGrid_ = true;                        // 是否显示网格
};
