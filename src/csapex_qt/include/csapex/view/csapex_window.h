#ifndef CSAPEX_WINDOW_H
#define CSAPEX_WINDOW_H

/// COMPONENT
#include <csapex_qt/export.h>
#include <csapex/view/view_fwd.h>
#include <csapex/core/core_fwd.h>
#include <csapex/plugin/plugin_fwd.h>
#include <csapex/command/command_fwd.h>
#include <csapex/model/model_fwd.h>
#include <csapex/scheduling/scheduling_fwd.h>
#include <csapex/utility/slim_signal.hpp>
#include <csapex/profiling/profiler.h>
#include <csapex/utility/utility_fwd.h>
#include <csapex/model/observer.h>

/// SYSTEM
#include <QMainWindow>
#include <QTimer>
#include <QFileSystemWatcher>
#include <QBoxLayout>

/// FORWARD DECLARATIONS
namespace Ui
{
class CsApexWindow;
}

namespace YAML
{
class Node;
}

class QLabel;

namespace csapex
{
/**
 * @brief The CsApexWindow class provides the window for the evaluator program
 */
class CSAPEX_QT_EXPORT CsApexWindow : public QMainWindow, public Observer
{
    Q_OBJECT

public:
    /**
     * @brief CsApexWindow
     * @param parent
     */
    explicit CsApexWindow(CsApexViewCore& core, QWidget* parent = 0);
    virtual ~CsApexWindow();

    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void moveEvent(QMoveEvent* event) override;

private Q_SLOTS:
    void updateMenu();
    void updateTitle();
    void tick();
    void init();
    void reloadStyleSheet(const QString& path);
    void loadStyleSheet(const QString& path);
    void showHelp(NodeBox* box);
    void showHowToInstall();

    void startStopServer();
    void updateServerOptions();

    void updateSelectionActions();
    void updateClipboardActions();

    void updatePluginIgnored(const QObject*& action);

    void addStateToSettings();
    void restoreWindowState();

public Q_SLOTS:
    void save();
    void saveAs();
    void saveAsCopy();
    void load();
    void reload();
    void reset();
    void clear();
    void undo();
    void redo();

    void triggerDisconnectEvent();

    void makeScreenshot();

    void start();
    void showStatusMessage(const std::string& msg);
    void showNotification(const Notification& notification);
    void updateNodeTypes();
    void updateSnippets();

    void updateDebugInfo();
    void updateUndoInfo();
    void updateNodeInfo();
    void updateThreadInfo();

    void about();
    void copyRight();
    void clearBlock();
    void resetActivity();
    void enableDebugProfiling(bool enabled);

    void loadTutorial(const QModelIndex& index);

Q_SIGNALS:
    void statusChanged(const QString& status);
    void showNotificationRequest(const Notification& notification);
    void closed();

    void updateMenuRequest();
    void updateUndoInfoRequest();
    void updateTitleRequest();
    void restoreWindowStateRequest();

    void disconnectEvent();

private:
    void construct();
    void setupDesigner();
    void updateServerStateLabel();

    bool eventFilter(QObject*, QEvent*);

    void createPluginsMenu();
    void createTutorialsMenu();
    void loadStyleSheet();

    void setupTimeline();
    void setupProfiling();
    void setupThreadManagement();

    std::string getConfigFile();

    void setStateChanged();
    bool isDirty() const;

private:
    CsApexViewCore& view_core_;

    std::shared_ptr<Profiler> profiler_;

    Ui::CsApexWindow* ui;

    Designer* designer_;
    MinimapWidget* minimap_;
    TracingLegend* tracing_legend_;
    TracingTimeline* tracing_timeline_;
    QLabel* client_server_state_label_;

    QTimer timer;
    QTimer auto_save_state_timer;

    bool init_;
    bool state_changed_;

    bool disconnected_;

    QFileSystemWatcher* style_sheet_watcher_;
    PluginLocatorPtr plugin_locator_;
};

}  // namespace csapex

#endif  // CSAPEX_WINDOW_H
