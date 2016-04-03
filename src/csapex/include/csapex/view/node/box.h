#ifndef BOX_H
#define BOX_H

/// COMPONENT
#include <csapex/view/view_fwd.h>
#include <csapex/model/model_fwd.h>
#include <csapex/model/memento.h>
#include <csapex/command/command.h>
#include <csapex/model/graph.h>
#include <csapex/model/error_state.h>
#include <csapex/model/node_handle.h>

/// SYSTEM
#include <memory>
#include <functional>
#include <QIcon>
#include <QMenu>
#include <QWidget>
#include <QBoxLayout>
#include <QLabel>

/// FORWARD DECLARATIONS
namespace Ui
{
class Box;
}

class QSizeGrip;

namespace csapex
{


class NodeBox : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(QString class READ cssClass)

public:
    typedef std::shared_ptr<NodeBox> Ptr;

    QString cssClass() {
        return QString("NodeBox");
    }


public:
    static const QString MIME;

public:
    /// CONSTRUCTION
    NodeBox(Settings& settings,
            NodeHandlePtr handle, NodeWorkerPtr worker,
            QIcon icon, GraphView* parent = 0);
    NodeBox(Settings& settings,
            NodeHandlePtr handle,
            QIcon icon, GraphView* parent = 0);

    void setAdapter(NodeAdapterPtr adapter);

    virtual ~NodeBox();
    virtual void construct();
    virtual void init();

    /// MODIFIER
    Port* createPort(ConnectableWeakPtr connector, QBoxLayout *layout);
    void removePort(ConnectableWeakPtr connector);

    /// ACCESSORS
    Node* getNode() const;
    NodeWorker* getNodeWorker() const;
    NodeHandle* getNodeHandle() const;
    NodeAdapterPtr getNodeAdapter() const;

    GraphView* getGraphView() const;

    bool hasSubGraph() const;
    GraphFacade *getSubGraph() const;

    void setLabel(const std::string& label);
    void setLabel(const QString& label);
    std::string getLabel() const;

    bool isMinimizedSize() const;
    bool isFlipped() const;

    bool isError() const;
    ErrorState::ErrorLevel errorLevel() const;
    std::string errorMessage() const;

    QBoxLayout* getInputLayout();
    QBoxLayout* getOutputLayout();
    QBoxLayout* getSlotLayout();
    QBoxLayout* getTriggerLayout();

    /// UI CALLBACKS
    void moveEvent(QMoveEvent*);
    void triggerPlaced();

    bool isSelected() const;
    virtual void setSelected(bool selected);

    void keyPressEvent(QKeyEvent * e);

    void stop();

protected:
    void setupUi();
    void paintEvent(QPaintEvent* e);
    bool eventFilter(QObject*, QEvent*);


public:
    void updateBoxInformation(Graph* graph);

public Q_SLOTS:
    void setupUiAgain();


    void getInformation();
    void triggerMinimized();
    void refreshStylesheet();
    void killContent();
    void triggerFlipSides();
    void showProfiling(bool show);

    virtual void updateComponentInformation(Graph* graph);
    virtual void updateThreadInformation();
    void contextMenuEvent(QContextMenuEvent* e);

    void registerEvent(Connectable*);
    void unregisterEvent(Connectable*);

    void nodeStateChangedEvent();
    void enabledChangeEvent(bool val);

    void updateVisuals();

    void createVariadicPort(bool output, ConnectionTypeConstPtr type, const std::string& label, bool optional);
    void createVariadicPortAndConnect(Connectable* from, ConnectionTypeConstPtr type, const std::string& label, bool optional);
    void createVariadicPortAndMove(Connectable* from, ConnectionTypeConstPtr type, const std::string& label, bool optional);

public:
    csapex::slim_signal::Signal<void(CommandPtr)> executeCommand;

Q_SIGNALS:
    void toggled(bool);
    void flipped(bool);
    void minimized(bool);

    void renameRequest(NodeBox*);
    void helpRequest(NodeBox*);

    void changed(NodeBox*);
    void clicked(NodeBox*);
    void moveSelectionToBox(NodeBox*);

    void showContextMenuForBox(NodeBox* box, const QPoint& pos);

    void showSubGraphRequest(UUID graphid);

    void updateVisualsRequest();

    void nodeStateChanged();
    void enabledChange(bool val);


    void portAdded(Port*);
    void portRemoved(Port*);

protected:
    void resizeEvent(QResizeEvent * e);

    void registerInputEvent(Input* in);
    void registerOutputEvent(Output* out);

    QString getNodeState();

protected:
    GraphView* parent_;

    Ui::Box* ui;
    QSizeGrip* grip_;

    Settings& settings_;

    NodeHandleWeakPtr node_handle_;
    NodeWorkerWeakPtr node_worker_;
    NodeAdapterPtr adapter_;

    std::unordered_map<UUID, Port*, UUID::Hasher> port_map_;

    QIcon icon_;

    QIcon minimize_icon_;
    QIcon maximize_icon_;

    QLabel* info_exec;
    QLabel* info_compo;
    QLabel* info_thread;
    QLabel* info_error;

    bool initialized_;
    bool moved_;
};

}
#endif // BOX_H
