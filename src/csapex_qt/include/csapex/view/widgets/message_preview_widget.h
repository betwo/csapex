#ifndef MESSAGE_PREVIEW_WIDGET_H
#define MESSAGE_PREVIEW_WIDGET_H

/// COMPONENT
#include <csapex_qt/export.h>
#include <csapex/msg/output.h>
#include <csapex/msg/input.h>

/// SYSTEM
#include <QGraphicsView>
#include <QPointer>

namespace csapex
{
class MessagePreviewWidget;

namespace impl
{
class CSAPEX_QT_EXPORT PreviewInput : public Input
{
public:
    PreviewInput(QPointer<MessagePreviewWidget> parent);

    virtual void setToken(TokenPtr message) override;

    void detach();

private:
    QPointer<MessagePreviewWidget> parent_;
};
}  // namespace impl

class CSAPEX_QT_EXPORT MessagePreviewWidget : public QGraphicsView
{
    Q_OBJECT

public:
    MessagePreviewWidget();
    ~MessagePreviewWidget();

public:
    void connectTo(ConnectorPtr c);
    void disconnect();

    void setCallback(std::function<void(TokenData::ConstPtr)> cb);

    bool isConnected() const;

Q_SIGNALS:
    void displayImageRequest(const QImage& msg);
    void displayTextRequest(const QString& txt);

public Q_SLOTS:
    void displayImage(const QImage& msg);
    void displayText(const QString& txt);

private:
    void connectToImpl(OutputPtr out);
    void connectToImpl(InputPtr out);

private:
    ConnectionPtr connection_;
    std::shared_ptr<impl::PreviewInput> input_;

    QString displayed_;

    QGraphicsPixmapItem* pm_item_;
    QGraphicsTextItem* txt_item_;
};

}  // namespace csapex

#endif  // MESSAGE_PREVIEW_WIDGET_H
