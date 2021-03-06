#!/bin/bash -l

FULLNAME=$1

if [[ $(grep "::" <<< $1) ]]; then
    arrIN=(${1//::/ })
    NODE_NAME=${arrIN[-1]}
    NAMESPACES=(${arrIN[@]})
    unset NAMESPACES[${#NAMESPACES[@]}-1]
else
    NODE_NAME=$1
    NAMESPACES=("csapex")
    FULLNAME="csapex::$FULLNAME"
fi


OPENING_NS=$""
CLOSING_NS=$""

FILE_NAME=$(sed -e 's/\([A-Z]\)/_\L\1/g' -e 's/^_//' <<< $NODE_NAME)
FILE_NAME=$(tr '[:upper:]' '[:lower:]' <<< $FILE_NAME)


DESCRIPTION=$2

if [[ ! NODE_NAME || ! $DESCRIPTION ]]; then
    echo "usage: $0 <node-name> <description>"
    exit
fi

WORKING_DIR=`pwd`

###
### FIND PREFIX
###
PREFIX="NOT-FOUND"
while [[ `pwd` != "/" ]]; do    
    if [[ -f 'CMakeLists.txt' ]]; then
        PREFIX=`pwd`/
        break
    fi
    
    cd ..
done
if [[ $PREFIX == "NOT-FOUND" ]]; then
    echo "ERROR: cannot locate CMakeLists.txt"
    exit
fi

cd $PREFIX

DIR=${WORKING_DIR:${#PREFIX}}
if [[ "$DIR" == "" ]] ; then
    DIR="."
fi

###
### CMAKELISTS EXISTS, BEGIN PARSING
###
CMAKELIST=${PREFIX}CMakeLists.txt

###
### TEST IF NODE NAME IS FREE IN CMAKE LIST
###
if [[ `cat $CMAKELIST | grep "/[[:space:]]*$FILE_NAME" | wc -l` != 0 ]]; then
    echo "ERROR: $FILE_NAME already exists in CMakeLists.txt"
    exit
fi

###
### FIND PACKAGE XML
###
PACKAGEXML=package.xml
if [[ ! -f $PACKAGEXML ]]; then
    echo "ERROR: cannot locate the plugin xml file: $PACKAGEXML"
    exit
fi

LIBRARY=($(cat $CMAKELIST | grep "^[^#]*add_library"))
echo $LIBRARY
if [[ ! $LIBRARY ]]; then
    echo "make library entry"
    # uncomment library
    perl -0777 -i.original -pe 's/# add_library([^\n]*)\n# ([^\n]*)\n# \)/add_library$1\n $2\n\)/igs' $CMAKELIST
fi

###
### DETERMINE THE PROJECT NAME
###
PROJECT_NAME=$(cat $CMAKELIST | grep "project(" | sed "s/project(\(.*\))/\\1/");

###
### FIND PLUGIN XML INSIDE PACKAGE XML
###
PLUGINXML=$(cat $PACKAGEXML | grep "csapex" | grep "plugin=" | grep -Po 'plugin="\K[^"]*' | \
            sed -e "s|\${prefix}|${PREFIX}|")
if [[ "$PLUGINXML" == "" ]]; then
    echo "no plugin xml file found, creating one"
    LIB=$(cat $CMAKELIST | grep "^add_library" | sed "s/add_library(\(.*\)/lib\1.so/")
    LIBRARY_NAME=$(eval echo $LIB)
    PLUGINXML="plugin.xml"
    sed 's/.*<export>.*/&\n    <csapex plugin="${prefix}\/plugins.xml" \/>/' $PACKAGEXML -i
fi

if [[ ! -f $PLUGINXML ]]; then
    echo "Cannot locate the plugin xml file: $PLUGINXML, creating it"
    LIB=$(cat $CMAKELIST | grep "^add_library" | sed "s/add_library(\(.*\)/lib\1.so/")
    LIBRARY_NAME=$(eval echo $LIB)
    echo "<library path=\"$LIBRARY_NAME\">
</library>" > $PLUGINXML
fi

###
### TEST IF NODE NAME IS FREE IN PACKAGEXML
###
if [[ `cat $PLUGINXML | grep "type=.*$FULLNAME\"" | wc -l` != 0 ]]; then
    echo "ERROR: $NODE_NAME already exists in $PLUGINXML"
    exit
fi


###
### FIND NAME OF THE PLUGIN LIBRARY
###
LIBRARY=($(cat $PLUGINXML | grep "<library" | grep -Po 'path="lib\K[^"]*'))

if [[ ! $LIBRARY ]]; then
    echo "ERROR: cannot find library name in $PLUGINXML"
    exit
fi

if [[ ${#LIBRARY[@]} > 1 ]]; then
    echo "There are multiple libraries, please select the target:"
    select LIB in ${LIBRARY[@]};
    do
        LIBRARY=$LIB
        echo "You picked $LIBRARY"
        break
    done
else
    echo "Target library is $LIBRARY"
fi

###
### EVERYTHING THERE -> CREATE
###

NEW_FILE="$FILE_NAME.cpp"
NEW_XML_1="<class type=\"$FULLNAME\" base_class_type=\"csapex::Node\">"
NEW_XML_2="  <description>$DESCRIPTION</description>"
NEW_XML_3="<\/class>"


###
### MODIFY CMAKELISTS
###
if [[ $(grep "add_library.*$LIBRARY" $CMAKELIST) ]]; then
    WS=$(grep "add_library.*$LIBRARY" $CMAKELIST -A 1 | tail -n 1 | cut -d's' -f1 | sed 's/ //')
    ENTRY="${WS}$DIR/$NEW_FILE"
    sed -i "/add_library.*$LIBRARY\s*$/a\ $ENTRY"  $CMAKELIST
    sed -i "/add_library.*$LIBRARY\s*SHARED\s*$/a\ $ENTRY"  $CMAKELIST

else
    LIBRARY_VAR=$(echo $LIBRARY | sed "s/${PROJECT_NAME}/\${PROJECT_NAME}/")

    WS=$(grep "add_library.*$LIBRARY_VAR" $CMAKELIST -A 1 | tail -n 1 | cut -d's' -f1 | sed 's/ //')
    ENTRY="${WS}$DIR/$NEW_FILE"
    sed -i "/add_library.*$LIBRARY_VAR\s*$/a\ $ENTRY"  $CMAKELIST
    sed -i "/add_library.*$LIBRARY_VAR\s*SHARED\s*$/a\ $ENTRY"  $CMAKELIST
fi

###
### MODIFY PLUGINXML
###
ENTRY="$NEW_XML_1\n$NEW_XML_2\n$NEW_XML_3"
sed -i -e "/<library.*$LIBRARY/a $ENTRY"  $PLUGINXML


###
### GENERATE SOURCE
###


for ns in "${NAMESPACES[@]}"
do
    OPENING_NS="${OPENING_NS}namespace $ns
{
"
    CLOSING_NS="} // $ns
$CLOSING_NS"
done

echo "
/// PROJECT
#include <csapex/model/node.h>
#include <csapex/msg/io.h>
#include <csapex/param/parameter_factory.h>
#include <csapex/model/node_modifier.h>
#include <csapex/utility/register_apex_plugin.h>
#include <csapex/msg/generic_value_message.hpp>

using namespace csapex;
using namespace csapex::connection_types;

$OPENING_NS

class $NODE_NAME : public Node
{
public:
    $NODE_NAME()
    {
    }

    void setup(csapex::NodeModifier& modifier) override
    {
        in_ = modifier.addInput<std::string>(\"Input\");
        out_ = modifier.addOutput<std::string>(\"Output\");
    }

    void setupParameters(csapex::Parameterizable& params) override
    {
    }

    void process() override
    {
        std::string value = msg::getValue<std::string>(in_);

        MessageConstPtr message = msg::getMessage<GenericValueMessage<std::string>>(in_);
        apex_assert(message);
        msg::publish(out_, value + \"!\");
    }

private:
    Input* in_;
    Output* out_;

};

$CLOSING_NS

CSAPEX_REGISTER_CLASS($FULLNAME, csapex::Node)
"> $DIR/$NEW_FILE

###
### ADD TO GIT
###
if [[ -f $DIR/.git ]] || [[ -d $DIR/.git ]]; then
    git add $DIR/$NEW_FILE
fi
