#ifndef NOTIFICATION_WIDGET_H
#define NOTIFICATION_WIDGET_H

/// COMPONENT
#include <csapex_qt/export.h>
#include <csapex/model/notification.h>

/// PROJECT
#include <csapex/view/view_fwd.h>

/// SYSTEM
#include <QWidget>

class QLabel;
class QTimer;
class QGraphicsOpacityEffect;

namespace csapex
{
class CSAPEX_QT_EXPORT NotificationWidget : public QWidget
{
    Q_OBJECT

public:
    NotificationWidget(const Notification& notification, QWidget* parent = nullptr);
    ~NotificationWidget() override;

    void setNotification(const Notification& notification);
    const Notification& getNotification();

    bool eventFilter(QObject*, QEvent*) override;

    bool isFading() const;

Q_SIGNALS:
    void activated(AUUID id);
    void fade_start();
    void fade_end();

private Q_SLOTS:
    void fadeout();
    void shutdown();

protected:
    void paintEvent(QPaintEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* me) override;

    void updateNotification();
    QString getText();
    QString getRawText();

private:
    QTimer* timer_;
    QGraphicsOpacityEffect* eff;

    Notification notification_;
    std::string notification_msg_;

    QLabel* icon_;
    QLabel* label_;

    bool fading_;
};

}  // namespace csapex
#endif  // NOTIFICATION_WIDGET_H
