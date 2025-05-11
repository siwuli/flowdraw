#include "MainWindow.hpp"
#include "FlowView.hpp"
#include <QMenuBar>
#include <QToolBar>
#include <QShortcut> 

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
    toolBar->addAction(tr("Ellipse"));

    connect(actRect, &QAction::triggered, this, [view] {
        view->setToolMode(FlowView::ToolMode::DrawRect);
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

