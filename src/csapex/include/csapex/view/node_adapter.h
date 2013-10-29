#ifndef NODE_ADAPTER_H
#define NODE_ADAPTER_H

/// SYSTEM
#include <QLayout>
#include <boost/shared_ptr.hpp>

namespace csapex
{

class NodeAdapterBridge : public QObject
{
    Q_OBJECT

Q_SIGNALS:
    void guiChanged();

public:
    void triggerGuiChanged();

};

class NodeAdapter
{
public:
    typedef boost::shared_ptr<NodeAdapter> Ptr;

public:
    NodeAdapter();
    virtual ~NodeAdapter();

    void doSetupUi(QBoxLayout* layout);
    virtual void updateDynamicGui(QBoxLayout* layout);

protected:
    virtual void setupUi(QBoxLayout* layout);

protected:
    void guiChanged();

public:
    NodeAdapterBridge bridge;
    bool is_gui_setup_;
};

}

#endif // NODE_ADAPTER_H