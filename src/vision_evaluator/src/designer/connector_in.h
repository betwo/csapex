#ifndef CONNECTOR_IN_H
#define CONNECTOR_IN_H

/// COMPONENT
#include "connector.h"

namespace vision_evaluator {

/// FORWARDS DECLARATION
class ConnectorOut;

class ConnectorIn : public Connector
{
    Q_OBJECT

public:
    ConnectorIn(QWidget* parent);
    ~ConnectorIn();

    virtual bool tryConnect(Connector* other_side);
    virtual bool canConnect();
    virtual bool isConnected();
    virtual bool acknowledgeConnection(Connector* other_side);
    virtual void removeConnection(Connector* other_side);

    virtual bool isInput() {
        return true;
    }

public Q_SLOTS:
    virtual void removeAllConnections();

private:
    ConnectorOut* input;
};

}

#endif // CONNECTOR_IN_H
