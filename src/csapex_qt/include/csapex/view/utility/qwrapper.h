#ifndef QWRAPPER_H
#define QWRAPPER_H

/// COMPONENT
#include <csapex_qt/export.h>

/// SYSTEM
#include <QSpinBox>
#include <QDoubleSpinBox>

namespace QWrapper
{
class CSAPEX_QT_EXPORT QSpinBoxExt : public QSpinBox
{
    Q_OBJECT

public Q_SLOTS:
    void setRange(int min, int max)
    {
        QSpinBox::setRange(min, max);
    }
};

class CSAPEX_QT_EXPORT QDoubleSpinBoxExt : public QDoubleSpinBox
{
    Q_OBJECT

public Q_SLOTS:
    void setRange(double min, double max)
    {
        QDoubleSpinBox::setRange(min, max);
    }
};
}  // namespace QWrapper

#endif  // QWRAPPER_H
