#ifndef DIRECT_CONNECTION_H
#define DIRECT_CONNECTION_H

/// PROJECT
#include <csapex/model/connection.h>

namespace csapex
{
class CSAPEX_CORE_EXPORT DirectConnection : public Connection
{
public:
    static ConnectionPtr connect(OutputPtr from, InputPtr to);
    static ConnectionPtr connect(OutputPtr from, InputPtr to, int id);

public:
    ~DirectConnection() override;

protected:
    DirectConnection(OutputPtr from, InputPtr to);
    DirectConnection(OutputPtr from, InputPtr to, int id);
};

}  // namespace csapex

#endif  // DIRECT_CONNECTION_H
