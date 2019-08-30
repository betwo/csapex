#ifndef TUTORIAL_TREE_MODEL_H
#define TUTORIAL_TREE_MODEL_H

/// COMPONENT
#include <csapex_qt/export.h>
#include <csapex/plugin/plugin_fwd.h>

/// SYSTEM
#include <QStandardItemModel>
#include <QFile>

class QTreeWidget;
class QTreeWidgetItem;

namespace csapex
{
class Settings;

class CSAPEX_QT_EXPORT TutorialTreeModel
{
public:
    struct ReadMe
    {
        QString title;
        QString description;
    };

public:
    TutorialTreeModel(Settings& settings, PluginLocator& plugin_locator);
    ~TutorialTreeModel();

    void fill(QTreeWidget* tree);

private:
    template <typename Path>
    void importDirectory(QTreeWidgetItem* parent, const Path& path);

    ReadMe parseReadMe(QFile& file);

private:
    Settings& settings_;
    PluginLocator& plugin_locator_;
    QTreeWidget* tree_;

    QMap<QString, QTreeWidgetItem*> top_level_;
};

}  // namespace csapex

#endif  // TUTORIAL_TREE_MODEL_H
