/// HEADER
#include "box.h"

/// PROJECT
#include "ui_box.h"
#include "boxed_object.h"

/// SYSTEM
#include <QDragMoveEvent>
#include <QMenu>
#include <iostream>
#include <boost/foreach.hpp>

using namespace vision_evaluator;

const QString Box::MIME = "vision_evaluator/box";
const QString Box::MIME_MOVE = "vision_evaluator/box/move";

Box::Box(BoxedObject *content, QWidget* parent)
    : QWidget(parent), ui(new Ui::Box), down_(false), content_(content)
{
    ui->setupUi(this);

    setObjectName(ui->content->title());

    ui->content->installEventFilter(this);

    content_->setBox(this);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(ui->content, SIGNAL(toggled(bool)), this, SIGNAL(toggled(bool)));
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
}

void Box::addInput(ConnectorIn *in)
{
    in->setParent(NULL);
    ui->input_layout->addWidget(in);
    input.push_back(in);
}

void Box::addOutput(ConnectorOut *out)
{
    out->setParent(NULL);
    ui->output_layout->addWidget(out);
    output.push_back(out);
}

void Box::init(const QPoint &pos)
{
    setGeometry(pos.x(), pos.y(), 100, 100);

    QBoxLayout* layout = new QHBoxLayout;
    ui->content->setLayout(layout);

    content_->fill(layout);
}

Box::~Box()
{

}

bool Box::eventFilter(QObject * o, QEvent * e)
{
    QMouseEvent* em = dynamic_cast<QMouseEvent*>(e);
    if(o == ui->content) {
        if(e->type() == QEvent::MouseButtonPress && em->button() == Qt::LeftButton) {
            down_ = true;
        } else if(e->type() == QEvent::MouseButtonRelease && em->button() == Qt::LeftButton) {
            down_ = false;
        } else if(e->type() == QEvent::MouseMove) {
            if(down_) {
                e->ignore();

                QPoint offset = ui->content->geometry().topLeft();
                startDrag(-em->pos() - offset);

                down_ = false;
                return true;
            }
        }
    }

    return false;
}

void Box::paintEvent(QPaintEvent *e)
{
    ui->content->setTitle(objectName());
    resize(sizeHint());
}

void Box::mousePressEvent(QMouseEvent* e)
{
    if(e->button() == Qt::LeftButton) {
        startDrag(-e->pos());
    }
}

void Box::startDrag(QPoint offset)
{
    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = new QMimeData;
    mimeData->setText(Box::MIME_MOVE);
    mimeData->setParent(this);
    mimeData->setUserData(0, new MoveOffset(offset));
    drag->setMimeData(mimeData);

    drag->exec();
}

QPixmap Box::makePixmap(const std::string& label)
{
    int border = 3;

    QImage img(QSize(80, 80), QImage::Format_ARGB32);
    QPainter painter(&img);
    painter.fillRect(QRect(QPoint(0,0), img.size()), Qt::white);
    painter.setPen(QPen(Qt::red, border));
    painter.drawRect(QRect(QPoint(border/2,border/2), img.size() - QSize(border,border)));
    painter.drawText(QPoint(5, 40), label.c_str());
    return QPixmap::fromImage(img);
}

void Box::setOverlay(Overlay* o)
{
    overlay_ = o;

    BOOST_FOREACH(ConnectorIn* i, input) {
        i->setOverlay(o);
    }
    BOOST_FOREACH(ConnectorOut* i, output) {
        i->setOverlay(o);
    }
}

void Box::showContextMenu(const QPoint& pos)
{
    QPoint globalPos = mapToGlobal(pos);

    QString remove_txt("delete");

    QMenu menu;
    menu.addAction(remove_txt);

    QAction* selectedItem = menu.exec(globalPos);

    if(selectedItem) {
        if(selectedItem->text() == remove_txt) {
            deleteLater();
        }
    }
}
