#include "MainWindow.hpp"
#include "FlowView.hpp"
#include "PropertyPanel.hpp"
#include <QMenuBar>
#include <QToolBar>
#include <QShortcut> 
#include <QDockWidget>
#include <QListWidget>
#include <QShortcut>
#include <QMimeData>
#include <QDrag>
#include <QColorDialog>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolButton>
#include <QMenu>
#include <QDebug>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    resize(1200, 800);
    setWindowTitle("FlowDraw MVP");

    /* ---------- Central Widget ---------- */
    auto* view = new FlowView(this);   // 新建画布
    setCentralWidget(view);            // 设为中心部件

    /* ---------- Menu ---------- */
    auto fileMenu = menuBar()->addMenu(tr("File"));
    fileMenu->addAction(tr("New"), this, [this, view]() {
        view->clearAll();
    });
    
    fileMenu->addAction(tr("Open"), this, [this, view]() {
        QString filename = QFileDialog::getOpenFileName(
            this, tr("Open Flowchart"), QString(), 
            tr("Flowchart Files (*.flow);;All Files (*.*)"));
        if (!filename.isEmpty()) {
            if (!view->loadFromFile(filename)) {
                QMessageBox::warning(this, tr("Error"), tr("Cannot open file"));
            }
        }
    });
    
    fileMenu->addAction(tr("Save"), this, [this, view]() {
        QString filename = QFileDialog::getSaveFileName(
            this, tr("Save Flowchart"), QString(), 
            tr("Flowchart Files (*.flow)"));
        if (!filename.isEmpty()) {
            if (!view->saveToFile(filename)) {
                QMessageBox::warning(this, tr("Error"), tr("Cannot save file"));
            }
        }
    });
    
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Export PNG"), this, [this, view]() {
        QString filename = QFileDialog::getSaveFileName(
            this, tr("Export as PNG"), QString(), 
            tr("PNG Images (*.png)"));
        if (!filename.isEmpty()) {
            if (!view->exportToPng(filename)) {
                QMessageBox::warning(this, tr("Error"), tr("Cannot export PNG"));
            }
        }
    });
    
    fileMenu->addAction(tr("Export SVG"), this, [this, view]() {
        QString filename = QFileDialog::getSaveFileName(
            this, tr("Export as SVG"), QString(), 
            tr("SVG Images (*.svg)"));
        if (!filename.isEmpty()) {
            if (!view->exportToSvg(filename)) {
                QMessageBox::warning(this, tr("Error"), tr("Cannot export SVG"));
            }
        }
    });

    auto editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(tr("Undo"));
    editMenu->addAction(tr("Redo"));
    editMenu->addSeparator();
    editMenu->addAction(tr("Copy"), view, &FlowView::copySelection);
    editMenu->addAction(tr("Cut"), view, &FlowView::cutSelection);
    editMenu->addAction(tr("Paste"), view, &FlowView::pasteClipboard);
    editMenu->addAction(tr("Delete"), view, &FlowView::deleteSelection);
    
    auto arrangeMenu = menuBar()->addMenu(tr("Arrange"));
    arrangeMenu->addAction(tr("Bring to Front"), view, &FlowView::bringToFront);
    arrangeMenu->addAction(tr("Send to Back"), view, &FlowView::sendToBack);
    arrangeMenu->addAction(tr("Move Up"), view, &FlowView::moveUp);
    arrangeMenu->addAction(tr("Move Down"), view, &FlowView::moveDown);
    
    auto pageMenu = menuBar()->addMenu(tr("Page"));
    pageMenu->addAction(tr("Background Color"), this, [this, view]() {
        QColor color = QColorDialog::getColor(
            view->backgroundColor(), this, tr("Select Background Color"));
        if (color.isValid()) {
            view->setBackgroundColor(color);
        }
    });
    
    pageMenu->addAction(tr("Page Size"), this, [this, view]() {
        bool ok;
        int width = QInputDialog::getInt(this, tr("Set Page Width"),
                                       tr("Width (pixels):"), 
                                       view->pageSize().width(), 500, 5000, 100, &ok);
        if (!ok) return;
        
        int height = QInputDialog::getInt(this, tr("Set Page Height"),
                                       tr("Height (pixels):"), 
                                       view->pageSize().height(), 500, 5000, 100, &ok);
        if (ok) {
            view->setPageSize(width, height);
        }
    });
    
    // 创建网格显示选项并设置初始状态与FlowView中的isGridVisible一致
    auto gridAction = pageMenu->addAction(tr("Show Grid"), this, [view](bool checked) {
        view->setGridVisible(checked);
    });
    gridAction->setCheckable(true);
    gridAction->setChecked(view->isGridVisible());  // 设置初始状态
    
    // 添加视图菜单
    auto viewMenu = menuBar()->addMenu(tr("View"));
    viewMenu->addAction(tr("Zoom In\tCtrl++"), view, &FlowView::zoomIn);
    viewMenu->addAction(tr("Zoom Out\tCtrl+-"), view, &FlowView::zoomOut);
    viewMenu->addAction(tr("Reset Zoom\tCtrl+0"), view, &FlowView::resetZoom);
    viewMenu->addAction(tr("Fit to Window\tCtrl+F"), view, &FlowView::fitToWindow);

    /* ---------- Toolbar ---------- */
    auto toolBar = addToolBar(tr("Tools"));
    
    // 创建形状菜单按钮
    auto* shapesButton = new QToolButton;
    shapesButton->setText(tr("Shapes"));
    shapesButton->setPopupMode(QToolButton::InstantPopup); // 点击时立即显示菜单
    
    // 创建形状菜单
    auto* shapesMenu = new QMenu(shapesButton);
    
    // 添加各种形状到菜单
    auto actRect = shapesMenu->addAction(tr("Rectangle"));
    auto actEllipse = shapesMenu->addAction(tr("Ellipse"));
    auto actDiamond = shapesMenu->addAction(tr("Diamond"));
    auto actTriangle = shapesMenu->addAction(tr("Triangle"));
    auto actPentagon = shapesMenu->addAction(tr("Pentagon"));
    
    // 设置菜单到按钮
    shapesButton->setMenu(shapesMenu);
    toolBar->addWidget(shapesButton);
    
    // 连接器按钮
    auto actLine = toolBar->addAction(tr("Connector"));
    
    // 添加分隔符
    toolBar->addSeparator();
    
    // 添加视图控制工具按钮
    auto actZoomIn = toolBar->addAction(tr("Zoom In"));
    auto actZoomOut = toolBar->addAction(tr("Zoom Out"));
    auto actResetZoom = toolBar->addAction(tr("Reset Zoom"));
    auto actFitWindow = toolBar->addAction(tr("Fit to Window"));

    connect(actRect, &QAction::triggered, this, [view] {
        view->clearSelection();
        view->setToolMode(FlowView::ToolMode::DrawRect);
        });

    //椭圆按钮
    connect(actEllipse, &QAction::triggered, this, [view] {
        view->clearSelection();
        view->setToolMode(FlowView::ToolMode::DrawEllipse);
        });
        
    //菱形按钮
    connect(actDiamond, &QAction::triggered, this, [view] {
        view->clearSelection();
        view->setToolMode(FlowView::ToolMode::DrawDiamond);
        });
        
    //三角形按钮
    connect(actTriangle, &QAction::triggered, this, [view] {
        view->clearSelection();
        qDebug() << "Triangle draw mode activated";
        view->setToolMode(FlowView::ToolMode::DrawTriangle);
        });

    //五边形按钮
    connect(actPentagon, &QAction::triggered, this, [view] {
        view->clearSelection();
        qDebug() << "Pentagon draw mode activated";
        view->setToolMode(FlowView::ToolMode::DrawPentagon);
        });

    connect(actLine, &QAction::triggered, this, [view] {
        view->clearSelection();                               // 取消现有选中
        view->setToolMode(FlowView::ToolMode::DrawConnector);
        // 设置鼠标指针为十字形状，提示用户可以直接在画布上绘制连接线
        view->setCursor(Qt::CrossCursor);
        });

    // 连接视图控制按钮
    connect(actZoomIn, &QAction::triggered, view, &FlowView::zoomIn);
    connect(actZoomOut, &QAction::triggered, view, &FlowView::zoomOut);
    connect(actResetZoom, &QAction::triggered, view, &FlowView::resetZoom);
    connect(actFitWindow, &QAction::triggered, view, &FlowView::fitToWindow);

    /* ---------- Palette Dock ---------- */
    auto paletteDock = new QDockWidget(tr("Component Library"), this);
    paletteDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto* palette = new QListWidget(paletteDock);
    // 使用IconMode，以网格方式排列图标
    palette->setViewMode(QListView::IconMode);
    // 设置较大的图标
    palette->setIconSize(QSize(48, 48));
    // 设置网格大小，保证每行两个图标
    int gridWidth = ((palette->width() - 10) / 2);
    palette->setGridSize(QSize(gridWidth, 70));
    // 调整内容流动方式和换行
    palette->setFlow(QListView::LeftToRight);
    palette->setWrapping(true);
    palette->setResizeMode(QListView::Adjust);
    // 禁止拖动位置
    palette->setMovement(QListView::Static);
    palette->setDragEnabled(true);
    // 简洁美观的样式
    palette->setStyleSheet(
        "QListWidget {"
        "   background-color: #f8f8f8;"
        "}"
        "QListWidget::item {"
        "   background-color: white;"
        "   border: 1px solid #e0e0e0;"
        "   border-radius: 4px;"
        "   margin: 4px;"
        "}"
        "QListWidget::item:selected {"
        "   background-color: #e0e0f0;"
        "   border: 1px solid #a0a0d0;"
        "}"
        "QListWidget::item:hover {"
        "   background-color: #f0f0f0;"
        "   border: 1px solid #c0c0d0;"
        "}"
    );
    
    // 连接器图标（不显示文本）
    auto* itConnector = new QListWidgetItem(QIcon(":/icons/connector.svg"), "");
    itConnector->setData(Qt::UserRole, "connector");
    itConnector->setToolTip(tr("Connector")); // 使用工具提示显示功能
    palette->addItem(itConnector);
    
    // 矩形图标
    auto* itRect = new QListWidgetItem(QIcon(":/icons/rect.svg"), "");
    itRect->setData(Qt::UserRole, "rect");
    itRect->setToolTip(tr("Rectangle"));
    palette->addItem(itRect);

    // 椭圆图标
    auto* itEllipse = new QListWidgetItem(QIcon(":/icons/ellipse.svg"), "");
    itEllipse->setData(Qt::UserRole, "ellipse");
    itEllipse->setToolTip(tr("Ellipse"));
    palette->addItem(itEllipse);
    
    // 菱形图标
    auto* itDiamond = new QListWidgetItem(QIcon(":/icons/diamond.svg"), "");
    itDiamond->setData(Qt::UserRole, "diamond");
    itDiamond->setToolTip(tr("Diamond"));
    palette->addItem(itDiamond);

    // 三角形图标
    auto* itTriangle = new QListWidgetItem(QIcon(":/icons/triangle.svg"), "");
    itTriangle->setData(Qt::UserRole, "triangle");
    itTriangle->setToolTip(tr("Triangle"));
    palette->addItem(itTriangle);

    // 五边形图标
    auto* itPentagon = new QListWidgetItem(QIcon(":/icons/pentagon.svg"), "");
    itPentagon->setData(Qt::UserRole, "pentagon");
    itPentagon->setToolTip(tr("Pentagon"));
    palette->addItem(itPentagon);

    // 拖拽实现
    palette->setDefaultDropAction(Qt::CopyAction);
    palette->installEventFilter(palette);          // 让 lambda 捕获

    // 拖拽事件
    QObject::connect(palette, &QListWidget::itemPressed,
        palette, [palette](QListWidgetItem* item) {
            if (!item || (item->flags() & Qt::ItemIsEnabled) == 0) return;
            auto* mime = new QMimeData;
            mime->setData("application/x-flow-shape",
                item->data(Qt::UserRole).toByteArray());

            auto* drag = new QDrag(palette);
            drag->setMimeData(mime);
            drag->setHotSpot(QPoint(24, 24));
            drag->exec(Qt::CopyAction);
        });
    
    // 点击事件
    QObject::connect(palette, &QListWidget::itemClicked,
        this, [view, palette](QListWidgetItem* item) {
            if (!item || (item->flags() & Qt::ItemIsEnabled) == 0) return;
            
            QString type = item->data(Qt::UserRole).toString();
            if (type == "connector") {
                view->clearSelection();
                view->setToolMode(FlowView::ToolMode::DrawConnector);
                // 重要：将鼠标指针改为十字形状，提示用户可以直接在画布上绘制连接线
                view->setCursor(Qt::CrossCursor);
            } else if (type == "rect") {
                view->clearSelection();
                view->setToolMode(FlowView::ToolMode::DrawRect);
            } else if (type == "ellipse") {
                view->clearSelection();
                view->setToolMode(FlowView::ToolMode::DrawEllipse);
            } else if (type == "diamond") {
                view->clearSelection();
                view->setToolMode(FlowView::ToolMode::DrawDiamond);
            } else if (type == "triangle") {
                view->clearSelection();
                view->setToolMode(FlowView::ToolMode::DrawTriangle);
            } else if (type == "pentagon") {
                view->clearSelection();
                view->setToolMode(FlowView::ToolMode::DrawPentagon);
            }
        });

    paletteDock->setWidget(palette);
    addDockWidget(Qt::LeftDockWidgetArea, paletteDock);

    

    
    /* ---------- Property Dock ---------- */
    auto propDock = new QDockWidget(tr("Properties"), this);
    auto propPanel = new PropertyPanel(propDock);
    propDock->setWidget(propPanel);
    addDockWidget(Qt::RightDockWidgetArea, propDock);

    /* FlowView ⇄ PropertyPanel 双向绑定 */
    connect(view, &FlowView::shapeAttr,
        propPanel, &PropertyPanel::load);
    connect(view, &FlowView::shapeSize,
        propPanel, &PropertyPanel::loadSize);

    connect(propPanel, &PropertyPanel::fillChanged,
        view, &FlowView::setFill);
    connect(propPanel, &PropertyPanel::strokeChanged,
        view, &FlowView::setStroke);
    connect(propPanel, &PropertyPanel::widthChanged,
        view, &FlowView::setWidth);
    
    // 对象尺寸属性绑定
    connect(propPanel, &PropertyPanel::objectWidthChanged,
        view, &FlowView::setObjectWidth);
    connect(propPanel, &PropertyPanel::objectHeightChanged,
        view, &FlowView::setObjectHeight);
    
    // 文本属性绑定
    connect(propPanel, &PropertyPanel::textColorChanged,
        view, &FlowView::setTextColor);
    connect(propPanel, &PropertyPanel::textSizeChanged,
        view, &FlowView::setTextSize);



    //添加快捷键
    new QShortcut(QKeySequence::Copy, view, SLOT(copySelection()));
    new QShortcut(QKeySequence::Cut, view, SLOT(cutSelection()));
    new QShortcut(QKeySequence::Paste, view, SLOT(pasteClipboard()));
    new QShortcut(QKeySequence::Delete, view, SLOT(deleteSelection()));

    new QShortcut(QKeySequence("Ctrl+]"), view, SLOT(bringToFront()));
    new QShortcut(QKeySequence("Ctrl+["), view, SLOT(sendToBack()));
    new QShortcut(QKeySequence("Ctrl+Up"), view, SLOT(moveUp()));
    new QShortcut(QKeySequence("Ctrl+Down"), view, SLOT(moveDown()));
    
    // 添加视图控制快捷键
    new QShortcut(QKeySequence("Ctrl+="), view, SLOT(zoomIn()));
    new QShortcut(QKeySequence("Ctrl++"), view, SLOT(zoomIn()));
    new QShortcut(QKeySequence("Ctrl+-"), view, SLOT(zoomOut()));
    new QShortcut(QKeySequence("Ctrl+0"), view, SLOT(resetZoom()));
    new QShortcut(QKeySequence("Ctrl+F"), view, SLOT(fitToWindow()));
}

