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
    void shapeSize(int width, int height);  // 新增形状尺寸信号

public:
    explicit FlowView(QWidget* parent = nullptr);

    /* ---------- 工具模式 ---------- */
    enum class ToolMode { None, DrawRect, DrawEllipse, DrawDiamond, DrawConnector, DrawTriangle };
    void setToolMode(ToolMode m);

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
    
    // 对象尺寸设置
    void setObjectWidth(int width);
    void setObjectHeight(int height);
    
    // 连接线属性设置
    void setConnectorBidirectional(bool bidirectional);
    void toggleConnectorDirection();

    void updateConnectorsFor(Shape* movedShape);

    void bringToFront();
    void sendToBack();
    void moveUp();
    void moveDown();
    void clearSelection() { selectedIndex_ = -1; resizeHandle_ = ResizeHandle::None; update(); }
    
    // 视图控制
    void zoomIn();
    void zoomOut();
    void resetZoom();
    void fitToWindow();

protected:
    /* ---------- Qt 事件 ---------- */
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void wheelEvent(QWheelEvent*) override;  // 处理鼠标滚轮事件

    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;
    
    void keyPressEvent(QKeyEvent*) override;  // 处理键盘事件

    // 视图转换函数，将视图坐标转换到文档坐标
    QPointF viewToDoc(const QPointF& viewPoint) const;
    // 将文档坐标转换为视图坐标
    QPointF docToView(const QPointF& docPoint) const;
    // 绘制页面边界
    void drawPageBorder(QPainter& painter);
    
    // 调整大小相关功能
    enum class ResizeHandle {
        None,
        TopLeft, TopCenter, TopRight,
        MiddleLeft, MiddleRight,
        BottomLeft, BottomCenter, BottomRight
    };
    
    // 绘制调整大小的控制柄
    void drawResizeHandles(QPainter& painter, const QRectF& rect);
    // 检测点击在哪个调整柄上
    ResizeHandle hitTestResizeHandles(const QPointF& docPoint, const QRectF& rect);
    // 调整矩形大小
    void resizeRect(QRectF& rect, ResizeHandle handle, const QPointF& offset);
    
    // 更新属性面板显示
    void updatePropertyPanel();
    
    // 查找点击了哪个连接线
    int hitTestConnector(const QPointF& pt) const;

private:
    /* ---------- 数据成员 ---------- */
    ToolMode mode_ = ToolMode::None;

    std::vector<std::unique_ptr<Shape>> shapes_; // 所有图形元素
    std::vector<Connector> connectors_;          // 所有连接线
    Connector currentConn_;                      // 当前正在绘制的临时连接线
    
    int     selectedIndex_ = -1;   // 选中的图形索引
    int     selectedConnectorIndex_ = -1; // 选中的连接线索引
    QPointF dragStart_;
    
    // 调整大小相关
    ResizeHandle resizeHandle_ = ResizeHandle::None;
    
    // 页面设置
    QColor backgroundColor_ = QColor("#fdfdfd");  // 背景颜色
    QSize pageSize_ = QSize(2000, 2000);          // 页面大小
    bool showGrid_ = true;                        // 是否显示网格
    
    // 视图变换
    qreal scale_ = 1.0;            // 缩放比例
    QPointF viewOffset_ = {0, 0};  // 视图偏移量
    bool isPanning_ = false;       // 正在平移视图
    QPointF lastPanPoint_;         // 上次平移点
};
