#ifndef DESIGN_BOARD_H
#define DESIGN_BOARD_H

/// COMPONENT
#include <csapex/csapex_fwd.h>
#include <csapex/core/drag_io.h>

/// SYSTEM
#include <QWidget>

/// FORWARD DECLARATIONS
namespace Ui
{
class DesignBoard;
}

namespace csapex
{

class DesignBoard : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString class READ cssClass)

public:
    DesignBoard(CommandDispatcher *dispatcher, QWidget* parent = 0);
    virtual ~DesignBoard();

    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent* e);

    void keyPressEvent(QKeyEvent* e);
    void keyReleaseEvent(QKeyEvent* e);

    void wheelEvent(QWheelEvent * e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void mouseMoveEvent(QMouseEvent* e);

    void focusInEvent(QFocusEvent* e);
    void focusOutEvent(QFocusEvent* e);

    virtual bool eventFilter(QObject* o, QEvent* e);
    void dragEnterEvent(QDragEnterEvent* e);
    void dragMoveEvent(QDragMoveEvent* e);
    void dropEvent(QDropEvent* e);
    void dragLeaveEvent(QDragLeaveEvent * e);

    void enterEvent(QEvent * e);

    QString cssClass() {
        return QString("DesignBoard");
    }
    void enableGrid(bool);

public Q_SLOTS:
    void updateCursor();
    void showBoxDialog();
    void showContextMenuGlobal(const QPoint& pos);
    void showContextMenu(const QPoint& pos);
    void showContextMenuEditBox(Box* box, const QPoint& pos);
    void showContextMenuAddNode(const QPoint& global_pos);
    void findMinSize(Box* box);

    void addBoxEvent(Box* box);
    void removeBoxEvent(Box* box);
    void refresh();
    void reset();

private:
    Ui::DesignBoard* ui;

    CommandDispatcher* dispatcher_;
    Overlay* overlay;
    DragIO::Handler drag_io;

    bool space_;
    bool drag_;
    QPoint drag_start_pos_;
};

}
#endif // DESIGN_BOARD_H
