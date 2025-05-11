#include "MainWindow.hpp"
#include "FlowView.hpp"
#include <QMenuBar>
#include <QToolBar>
#include <QShortcut> 
#include <QDockWidget>
#include <QListWidget>
#include <QShortcut>
#include <QMimeData>
#include <QDrag>

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
    fileMenu->addAction(tr("New"));
    fileMenu->addAction(tr("Open"));
    fileMenu->addAction(tr("Save"));

    auto editMenu = menuBar()->addMenu(tr("Edit"));
    editMenu->addAction(tr("Undo"));
    editMenu->addAction(tr("Redo"));

    /* ---------- Toolbar ---------- */
    auto toolBar = addToolBar(tr("Tools"));
    auto actRect = toolBar->addAction(tr("Rectangle"));
    auto actEllipse = toolBar->addAction(tr("Ellipse"));

    connect(actRect, &QAction::triggered, this, [view] {
        view->clearSelection();
        view->setToolMode(FlowView::ToolMode::DrawRect);
        });

    /* ---------- Palette Dock ---------- */
    auto paletteDock = new QDockWidget(tr("Palette"), this);
    paletteDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    auto* palette = new QListWidget(paletteDock);
    palette->setViewMode(QListView::IconMode);
    palette->setIconSize({ 48,48 });
    palette->setSpacing(4);
    palette->setDragEnabled(true);

    // 一级：矩形
    auto* itRect = new QListWidgetItem(QIcon(":/icons/rect.png"), tr("Rectangle"));
    itRect->setData(Qt::UserRole, "rect");
    palette->addItem(itRect);

    // 二级：椭圆
    auto* itEllipse = new QListWidgetItem(QIcon(":/icons/ellipse.png"), tr("Ellipse"));
    itEllipse->setData(Qt::UserRole, "ellipse");
    palette->addItem(itEllipse);

    // 拖拽实现
    palette->setDefaultDropAction(Qt::CopyAction);
    palette->installEventFilter(palette);          // 让 lambda 捕获

    QObject::connect(palette, &QListWidget::itemPressed,
        palette, [palette](QListWidgetItem* item) {
            if (!item) return;
            auto* mime = new QMimeData;
            mime->setData("application/x-flow-shape",
                item->data(Qt::UserRole).toByteArray());

            auto* drag = new QDrag(palette);
            drag->setMimeData(mime);
            drag->setHotSpot(QPoint(24, 24));
            drag->exec(Qt::CopyAction);
        });

    paletteDock->setWidget(palette);
    addDockWidget(Qt::LeftDockWidgetArea, paletteDock);

    //给工具栏再加一个椭圆按钮
    connect(actEllipse, &QAction::triggered, this, [view] {
        view->clearSelection();
        view->setToolMode(FlowView::ToolMode::DrawEllipse);
        });


    //添加快捷键
    new QShortcut(QKeySequence::Copy, view, SLOT(copySelection()));
    new QShortcut(QKeySequence::Cut, view, SLOT(cutSelection()));
    new QShortcut(QKeySequence::Paste, view, SLOT(pasteClipboard()));
    new QShortcut(QKeySequence::Delete, view, SLOT(deleteSelection()));

    new QShortcut(QKeySequence("Ctrl+]"), view, SLOT(bringToFront()));
    new QShortcut(QKeySequence("Ctrl+["), view, SLOT(sendToBack()));
    new QShortcut(QKeySequence("Ctrl+Up"), view, SLOT(moveUp()));
    new QShortcut(QKeySequence("Ctrl+Down"), view, SLOT(moveDown()));

}

