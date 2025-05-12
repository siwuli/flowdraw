#pragma once
#include <QWidget>
#include <vector>
#include <memory>

#include "model/Shape.hpp"
#include "model/Rect.hpp"
#include "model/Ellipse.hpp"
#include "model/Connector.hpp"     // �� ��������

class FlowView : public QWidget
{
    Q_OBJECT

signals:
    void shapeAttr(const QColor& fill, const QColor& stroke, qreal width);

public:
    explicit FlowView(QWidget* parent = nullptr);

    /* ---------- ����ģʽ ---------- */
    enum class ToolMode { None, DrawRect, DrawEllipse, DrawConnector };
    void setToolMode(ToolMode m) { mode_ = m; }

    /* ---------- ������ / Z-Order �ӿ� ---------- */
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
    /* ---------- Qt �¼� ---------- */
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

    void dragEnterEvent(QDragEnterEvent*) override;
    void dropEvent(QDropEvent*) override;
    void contextMenuEvent(QContextMenuEvent*) override;

private:
    /* ---------- ���ݳ�Ա ---------- */
    ToolMode mode_ = ToolMode::None;

    std::vector<std::unique_ptr<Shape>> shapes_; // �� ����ͼ��
    std::vector<Connector> connectors_;          // �� ��������
    Connector currentConn_;                      // �� ���ڻ��Ƶ���ʱ����

    int     selectedIndex_ = -1;
    QPointF dragStart_;
};
