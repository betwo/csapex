#ifndef CSAPEX_H
#define CSAPEX_H

/// PROJECT
#include <csapex/view/widgets/csapex_splashscreen.h>
#include <csapex/core/csapex_core.h>
#include <csapex/core/settings.h>
#include <csapex/command/command_fwd.h>
#include <csapex/utility/exceptions.h>
#include <csapex/core/exception_handler.h>
#include <csapex/model/observer.h>

/// SYSTEM
#include <QApplication>
#include <memory>

#define DEBUG 0

namespace csapex
{
class CsApexViewCore;
class CsApexWindow;

struct CsApexCoreApp : public QCoreApplication
{
    CsApexCoreApp(int& argc, char** argv, ExceptionHandler& handler);

    bool notify(QObject* receiver, QEvent* event) override;

private:
    ExceptionHandler& handler;
};

struct CsApexGuiApp : public QApplication
{
    CsApexGuiApp(int& argc, char** argv, ExceptionHandler& handler);

    bool notify(QObject* receiver, QEvent* event) override;

    void handleAssertionFailure(const csapex::Failure& assertion);

private:
    ExceptionHandler& handler;
};

struct Main : public QObject, public Observer
{
    Q_OBJECT

public:
    Main(std::unique_ptr<QCoreApplication>&& app, Settings& settings, ExceptionHandler& handler);
    ~Main() override;

    int run();

public Q_SLOTS:
    void showMessage(const QString& msg);

private:
    int runWithGui();
    int runHeadless();
    int runImpl();

    void checkRecoveryFile(CsApexViewCore& view_core, CsApexWindow& w);
    void askForRecoveryConfig(const std::string& config_to_load);
    void deleteRecoveryConfig();

private:
    std::unique_ptr<QCoreApplication> app;
    Settings& settings;

    ExceptionHandler& handler;
    CsApexSplashScreen* splash;

    CsApexCorePtr core;

    bool recover_needed;
};

}  // namespace csapex

#endif  // CSAPEX_H
