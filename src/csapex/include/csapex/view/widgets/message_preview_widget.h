#ifndef MESSAGE_PREVIEW_WIDGET_H
#define MESSAGE_PREVIEW_WIDGET_H

/// COMPONENT
#include <csapex/msg/output.h>
#include <csapex/msg/input.h>

/// SYSTEM
#include <QGraphicsView>

namespace csapex
{

class MessagePreviewWidget;

namespace impl {
class PreviewInput : public Input
{
public:
    PreviewInput(MessagePreviewWidget* parent);

    virtual void inputMessage(ConnectionType::ConstPtr message) override;

private:
    MessagePreviewWidget* parent_;
};
}

class MessagePreviewWidget : public QGraphicsView
{
    Q_OBJECT

public:
    MessagePreviewWidget();
    ~MessagePreviewWidget();

public:
    void connectTo(Connectable* c);
    void disconnect();

    void setCallback(std::function<void(ConnectionType::ConstPtr)> cb);

    void display(const ConnectionTypeConstPtr& msg);

    bool isConnected() const;

private:
    void connectToImpl(Output* out);
    void connectToImpl(Input* out);

private:
    ConnectionPtr connection_;
    std::shared_ptr<impl::PreviewInput> input_;
};

}

#endif // MESSAGE_PREVIEW_WIDGET_H