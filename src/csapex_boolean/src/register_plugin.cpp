/// HEADER
#include "register_plugin.h"

/// COMPONENT
#include <csapex_boolean/boolean_message.h>

/// PROJECT
#include <csapex/model/tag.h>
#include <csapex/manager/connection_type_manager.h>

/// SYSTEM
#include <pluginlib/class_list_macros.h>

PLUGINLIB_EXPORT_CLASS(csapex::boolean::RegisterPlugin, csapex::CorePlugin)

using namespace csapex;
using namespace boolean;

RegisterPlugin::RegisterPlugin()
{
}


void RegisterPlugin::init()
{
    Tag::createIfNotExists("Boolean");

    ConnectionTypeManager::registerMessage<connection_types::BooleanMessage>();
}
