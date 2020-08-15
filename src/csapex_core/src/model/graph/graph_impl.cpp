/// HEADER
#include <csapex/model/graph/graph_impl.h>

/// PROJECT
#include <csapex/model/connection.h>
#include <csapex/model/node_facade.h>
#include <csapex/msg/input.h>
#include <csapex/msg/output.h>
#include <csapex/signal/event.h>
#include <csapex/signal/slot.h>
#include <csapex/msg/input_transition.h>
#include <csapex/msg/output_transition.h>
#include <csapex/model/node.h>
#include <csapex/model/node_facade_impl.h>
#include <csapex/model/node_handle.h>
#include <csapex/model/node_worker.h>
#include <csapex/model/node_state.h>
#include <csapex/model/graph/vertex.h>
#include <csapex/model/graph/edge.h>
#include <csapex/model/node.h>
#include <csapex/model/graph_facade_impl.h>
#include <csapex/model/subgraph_node.h>

using namespace csapex;

GraphImplementation::GraphImplementation() : in_transaction_(false), nf_(nullptr)
{
}

GraphImplementation::~GraphImplementation()
{
    clear();
}

AUUID GraphImplementation::getAbsoluteUUID() const
{
    return nf_->getUUID().getAbsoluteUUID();
}

void GraphImplementation::setNodeFacade(NodeFacadeImplementation* nf)
{
    nf_ = nf;
}

void GraphImplementation::resetActivity()
{
    auto connections = edges_;
    for (const ConnectionPtr& c : connections) {
        TokenPtr t = c->getToken();
        if (t) {
            t->setActivityModifier(ActivityModifier::NONE);
        }
    }

    auto vertices = vertices_;
    for (auto& vertex : vertices) {
        NodeFacadeImplementationPtr node = std::dynamic_pointer_cast<NodeFacadeImplementation>(vertex->getNodeFacade());
        apex_assert_hard(node);
        node->setActive(false);
    }
}

void GraphImplementation::clear()
{
    UUIDProvider::clearCache();

    beginTransaction();

    auto connections = edges_;
    for (const ConnectionPtr& c : connections) {
        deleteConnection(c);
    }
    apex_assert_hard(edges_.empty());

    auto vertices = vertices_;
    for (auto& vertex : vertices) {
        NodeFacadePtr node = vertex->getNodeFacade();
        deleteNode(node->getUUID());
    }
    apex_assert_hard(vertices_.empty());

    finalizeTransaction();
}

void GraphImplementation::addNode(NodeFacadeImplementationPtr nf)
{
    apex_assert_hard_msg(nf, "NodeFacade added is not null");
    graph::VertexPtr vertex = std::make_shared<graph::Vertex>(nf);
    vertices_.push_back(vertex);

    nf->getNodeHandle()->setVertex(vertex);

    sources_.insert(vertex);
    sinks_.insert(vertex);

    vertex_added(vertex);
    if (!in_transaction_) {
        analyzeGraph();
    }
}

std::vector<ConnectionPtr> GraphImplementation::getConnections()
{
    return edges_;
}

std::vector<ConnectionDescription> GraphImplementation::enumerateAllConnections() const
{
    std::vector<ConnectionDescription> result;
    result.reserve(edges_.size());
    for (const ConnectionPtr& c : edges_) {
        result.push_back(c->getDescription());
    }
    return result;
}

void GraphImplementation::deleteNode(const UUID& uuid)
{
    NodeHandle* node_handle = findNodeHandle(uuid);
    node_handle->stop();

    graph::VertexPtr removed;

    for (auto it = vertices_.begin(); it != vertices_.end();) {
        NodeFacadePtr node = (*it)->getNodeFacade();
        if (node->getUUID() == uuid) {
            removed = *it;
            vertices_.erase(it);

            break;

        } else {
            ++it;
        }
    }

    apex_assert_hard(removed);
    apex_assert_hard(removed == node_handle->getVertex());

    sources_.erase(removed);
    sinks_.erase(removed);

    for (const graph::VertexPtr& source : sources_) {
        apex_assert_neq(source, removed);
    }
    for (const graph::VertexPtr& sink : sinks_) {
        apex_assert_neq(sink, removed);
    }
    for (const graph::VertexPtr& remaining : vertices_) {
        apex_assert_neq(remaining, removed);
    }

    //        if(NodePtr node = removed->getNode().lock()) {
    //            if(GraphPtr child = std::dynamic_pointer_cast<Graph>(node)) {
    //                child->clear();
    //            }
    //        }

    vertex_removed(removed);
    if (!in_transaction_) {
        analyzeGraph();
    }
}

std::size_t GraphImplementation::countNodes()
{
    return vertices_.size();
}

bool GraphImplementation::addConnection(ConnectionPtr connection)
{
    apex_assert_hard(connection);
    edges_.push_back(connection);

    connection_observations_[connection.get()].push_back(connection->connection_changed.connect([this]() {
        if (!in_transaction_) {
            analyzeGraph();
        }
    }));

    if (!std::dynamic_pointer_cast<Event>(connection->from()) && !std::dynamic_pointer_cast<Slot>(connection->to())) {
        NodeHandle* n_from = findNodeHandleForConnector(connection->from()->getUUID());
        NodeHandle* n_to = findNodeHandleForConnector(connection->to()->getUUID());

        if (n_to != n_from) {
            apex_assert_hard(n_to->getUUID().getAbsoluteUUID() != n_from->getUUID().getAbsoluteUUID());

            graph::VertexPtr v_from = n_from->getVertex();
            graph::VertexPtr v_to = n_to->getVertex();

            if (v_from && v_to) {
                v_from->addChild(v_to);
                v_to->addParent(v_from);

                sources_.erase(v_to);
                sinks_.erase(v_from);
            }
        }
    }

    if (connection_added.isConnected()) {
        connection_added(connection->getDescription());
    }
    if (!in_transaction_) {
        analyzeGraph();
    }
    return true;
}

void GraphImplementation::deleteConnection(ConnectionPtr connection)
{
    apex_assert_hard(connection);

    connection_observations_[connection.get()].clear();

    if (connection->isDetached()) {
        auto c = std::find(edges_.begin(), edges_.end(), connection);
        if (c != edges_.end()) {
            edges_.erase(c);
        }
        return;
    }

    auto out = connection->from();
    auto in = connection->to();

    out->removeConnection(in.get());

    for (std::vector<ConnectionPtr>::iterator c = edges_.begin(); c != edges_.end();) {
        if (*connection == **c) {
            ConnectablePtr to = connection->to();

            UUID from_uuid = connection->from()->getUUID();
            NodeHandle* n_from = findNodeHandleForConnector(from_uuid);
            NodeHandle* n_to = findNodeHandleForConnector(connection->to()->getUUID());

            if (!std::dynamic_pointer_cast<Event>(connection->from()) && !std::dynamic_pointer_cast<Slot>(connection->to())) {
                // erase pointer from TO to FROM
                if (n_from != n_to) {
                    graph::VertexPtr v_from = n_from->getVertex();
                    graph::VertexPtr v_to = n_to->getVertex();

                    if (v_from && v_to) {
                        bool still_connected = false;
                        for (const ConnectionPtr& c : n_from->getOutputTransition()->getConnections()) {
                            if (c->isDetached()) {
                                continue;
                            }
                            ConnectablePtr to = c->to();
                            apex_assert_hard(to);
                            if (NodeHandlePtr child = std::dynamic_pointer_cast<NodeHandle>(to->getOwner())) {
                                if (child.get() == n_to) {
                                    still_connected = true;
                                    break;
                                }
                            }
                        }
                        if (!still_connected) {
                            v_to->removeParent(v_from.get());
                            v_from->removeChild(v_to.get());
                        }

                        if (!n_from->getOutputTransition()->hasConnection()) {
                            // verify that v_from is from this graph
                            for (const graph::VertexPtr& vertex : vertices_) {
                                if (vertex == v_from) {
                                    sinks_.insert(v_from);
                                    break;
                                }
                            }
                        }
                        if (!n_to->getInputTransition()->hasConnection()) {
                            // verify that v_to is from this graph
                            for (const graph::VertexPtr& vertex : vertices_) {
                                if (vertex == v_to) {
                                    sources_.insert(v_to);
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            edges_.erase(c);

            if (connection_removed.isConnected()) {
                connection_removed(connection->getDescription());
            }
            if (!in_transaction_) {
                analyzeGraph();
            }
            for (const auto& c : edges_) {
                apex_assert_hard(c);
            }

            return;

        } else {
            ++c;
        }
    }

    throw std::runtime_error("cannot delete connection");
}

void GraphImplementation::beginTransaction()
{
    in_transaction_ = true;
}

void GraphImplementation::finalizeTransaction()
{
    in_transaction_ = false;

    analyzeGraph();
}

void GraphImplementation::analyzeGraph()
{
    buildConnectedComponents();

    calculateDepths();

    state_changed();
}

void GraphImplementation::buildConnectedComponents()
{
    /* Find all connected sub components of this graph */
    std::deque<graph::Vertex*> unmarked;
    for (auto vertex : vertices_) {
        unmarked.push_back(vertex.get());
        vertex->getNodeCharacteristics().component = -1;
    }

    std::deque<graph::Vertex*> Q;
    int component = 0;
    while (!unmarked.empty()) {
        // take a random unmarked node to start bfs from
        auto* start = unmarked.front();
        Q.push_back(start);

        start->getNodeCharacteristics().component = component;

        while (!Q.empty()) {
            graph::Vertex* front = Q.front();
            Q.pop_front();

            NodeFacadeImplementationPtr local_facade = std::dynamic_pointer_cast<NodeFacadeImplementation>(front->getNodeFacade());
            apex_assert_hard(local_facade);

            checkNodeState(local_facade->getNodeHandle().get());

            auto it = std::find(unmarked.begin(), unmarked.end(), front);
            if (it == unmarked.end()) {
                continue;
            }
            unmarked.erase(it);

            // iterate all neighbors
            std::vector<graph::Vertex*> neighbors;
            for (auto parent : front->getParents()) {
                neighbors.push_back(parent.get());
            }
            for (auto child : front->getChildren()) {
                neighbors.push_back(child.get());
            }

            for (auto* neighbor : neighbors) {
                if (neighbor->getNodeCharacteristics().component == -1) {
                    neighbor->getNodeCharacteristics().component = component;
                    Q.push_back(neighbor);
                }
            }
        }

        ++component;
    }
}

std::set<graph::Vertex*> GraphImplementation::findVerticesThatJoinStreams()
{
    std::set<graph::Vertex*> joins;

    for (auto& vertex : vertices_) {
        vertex->getNodeCharacteristics().depth = -1;
    }

    // init node_depth_ and find merging nodes
    for (const auto& source : sources_) {
        source->getNodeCharacteristics().depth = 0;

        std::deque<const graph::Vertex*> Q;
        Q.push_back(source.get());
        while (!Q.empty()) {
            const graph::Vertex* top = Q.back();
            Q.pop_back();

            for (auto child : top->getChildren()) {
                if (child->getNodeCharacteristics().depth < 0) {
                    child->getNodeCharacteristics().depth = std::numeric_limits<int>::max();
                    Q.push_back(child.get());

                } else {
                    // already visited -> joins two or more "streams"
                    joins.insert(child.get());
                }
            }
        }
    }

    return joins;
}

std::set<graph::Vertex*> GraphImplementation::findVerticesThatNeedMessages()
{
    std::set<graph::Vertex*> vertices_that_need_messages;

    for (const auto& v : vertices_) {
        if (v->getNodeFacade()->isProcessingNothingMessages()) {
            vertices_that_need_messages.insert(v.get());
            break;
        }

        NodeFacadeImplementationPtr local_facade = std::dynamic_pointer_cast<NodeFacadeImplementation>(v->getNodeFacade());
        apex_assert_hard(local_facade);

        for (const auto& c : local_facade->getNodeHandle()->getOutputTransition()->getConnections()) {
            if (c->to()->isEssential()) {
                vertices_that_need_messages.insert(v.get());
                break;
            }
        }
    }

    return vertices_that_need_messages;
}

void GraphImplementation::calculateDepths()
{
    // start DFSs at each source. assign each node:
    // - depth: the minimum distance to any source
    // - joining: true, iff more than one path leads from any source to a node

    // initialize
    for (const graph::VertexPtr& vertex : vertices_) {
        NodeCharacteristics& characteristics = vertex->getNodeCharacteristics();
        characteristics.is_joining_vertex = false;
        characteristics.is_joining_vertex_counterpart = false;

        characteristics.is_combined_by_joining_vertex = false;
        characteristics.is_leading_to_joining_vertex = false;
        characteristics.is_leading_to_essential_vertex = false;
    }

    std::set<graph::Vertex*> essentials = findVerticesThatNeedMessages();

    for (const graph::Vertex* essential : essentials) {
        essential->getNodeCharacteristics().is_leading_to_essential_vertex = true;

        std::deque<const graph::Vertex*> Q;
        Q.push_back(essential);
        while (!Q.empty()) {
            const graph::Vertex* top = Q.back();
            Q.pop_back();

            for (auto parent : top->getParents()) {
                NodeCharacteristics& characteristics = parent->getNodeCharacteristics();
                if (!characteristics.is_leading_to_essential_vertex) {
                    characteristics.is_leading_to_essential_vertex = true;
                    Q.push_back(parent.get());
                }
            }
        }
    }

    std::set<graph::Vertex*> joins = findVerticesThatJoinStreams();

    // populate node_depth_ with minimal depths
    for (const auto& source : sources_) {
        source->getNodeCharacteristics().depth = 0;

        std::deque<const graph::Vertex*> Q;
        Q.push_back(source.get());
        while (!Q.empty()) {
            const graph::Vertex* top = Q.back();
            Q.pop_back();

            int top_depth = top->getNodeCharacteristics().depth;
            for (auto child : top->getChildren()) {
                int& child_depth = child->getNodeCharacteristics().depth;
                if (child_depth == std::numeric_limits<int>::max()) {
                    child_depth = top_depth + 1;
                    Q.push_back(child.get());

                } else {
                    // already visited
                    if (top_depth + 1 < child_depth) {
                        child_depth = top_depth + 1;
                        Q.push_back(child.get());
                    }
                }
            }
        }
    }

    for (graph::Vertex* vertex : joins) {
        vertex->getNodeCharacteristics().is_joining_vertex = true;

        // find counterparts
        //        int level = node_depth_.at(nh);

        //        const auto& parents = node_parents_.at(nh);
        //        int min_level = std::accumulate(parents.begin(), parents.end(), std::numeric_limits<int>::max(),
        //                                        [this](int level, NodeHandle* parent){ return std::min(level, node_depth_.at(parent)); });

        int min_level = vertex->getNodeCharacteristics().depth;

        // iterate all parents, until we find a single common parent
        // if no such parent exists, this joining node has no counterpart

        struct CompareNH : public std::binary_function<graph::Vertex*, graph::Vertex*, bool>
        {
            bool operator()(const graph::Vertex* lhs, const graph::Vertex* rhs) const
            {
                return lhs->getNodeCharacteristics().depth > rhs->getNodeCharacteristics().depth;
            }
        };
        // std::set<NodeHandle*, CompareNH> Q(parents.begin(), parents.end());
        std::deque<graph::Vertex*> Q({ vertex });
        std::set<graph::Vertex*> done;
        std::set<graph::Vertex*> minimum;

        bool is_closed = false;

        while (!Q.empty()) {
            graph::Vertex* top = *Q.begin();

            if (min_level > 0 && Q.size() == 1 && minimum.size() == 1) {
                // if the only minimum is not a source and the queue has only the current element -> closed
                is_closed = true;
                (*minimum.begin())->getNodeCharacteristics().is_joining_vertex_counterpart = true;
                break;
            }
            done.insert(top);
            Q.erase(Q.begin());
            for (graph::VertexPtr parent : top->getParents()) {
                if (done.find(parent.get()) != done.end()) {
                    continue;
                }
                if (std::find(Q.begin(), Q.end(), parent.get()) == Q.end()) {
                    Q.push_back(parent.get());
                    // TODO: implement this with a better datastructure, maybe priority queue...
                    std::sort(Q.begin(), Q.end(), CompareNH());
                }

                int level = parent->getNodeCharacteristics().depth;
                if (level < min_level) {
                    min_level = level;
                    minimum.clear();
                }
                if (level == min_level) {
                    minimum.insert(parent.get());
                }
            }
        }

        if (min_level == 0 && minimum.size() == 1) {
            // if exactly one source exists -> also closed
            is_closed = true;
            graph::Vertex* top = *minimum.begin();
            top->getNodeCharacteristics().is_joining_vertex_counterpart = true;
            done.erase(top);
        }

        for (graph::Vertex* top : done) {
            if (top != vertex) {
                top->getNodeCharacteristics().is_leading_to_joining_vertex = true;
            }
        }
        if (is_closed) {
            for (graph::Vertex* top : done) {
                if (top != vertex) {
                    top->getNodeCharacteristics().is_combined_by_joining_vertex = true;
                }
            }
        }

        //        todo:
        //        - remember for each of vertex join counterpart, which children are enclosed -> only send NoMessage to non-enclosed children.
    }
}

void GraphImplementation::checkNodeState(NodeHandle* nh)
{
    // check if the node should be enabled
    nh->getInputTransition()->checkIfEnabled();
    nh->getOutputTransition()->checkIfEnabled();
}

int GraphImplementation::getComponent(const UUID& node_uuid) const
{
    NodeHandle* node = findNodeHandleNoThrow(node_uuid);
    if (!node) {
        return -1;
    }

    graph::VertexPtr vertex = node->getVertex();
    return vertex->getNodeCharacteristics().component;
}

int GraphImplementation::getDepth(const UUID& node_uuid) const
{
    NodeHandle* node = findNodeHandleNoThrow(node_uuid);
    if (!node) {
        return -1;
    }

    graph::VertexPtr vertex = node->getVertex();
    return vertex->getNodeCharacteristics().depth;
}

// *** NODE **** /

Node* GraphImplementation::findNode(const UUID& uuid) const
{
    Node* node = findNodeNoThrow(uuid);
    if (node) {
        return node;
    }
    throw NodeNotFoundException(uuid.getFullName());
}

Node* GraphImplementation::findNodeNoThrow(const UUID& uuid) const noexcept
{
    NodeHandle* nh = findNodeHandleNoThrow(uuid);
    if (nh) {
        auto node = nh->getNode().lock();
        if (node) {
            return node.get();
        }
    }

    return nullptr;
}

Node* GraphImplementation::findNodeForConnector(const UUID& uuid) const
{
    try {
        return findNode(uuid.parentUUID());

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("cannot find node of connector \"") + uuid.getFullName() + ": " + e.what());
    }
}

// *** NODE HANDLE **** /

NodeHandle* GraphImplementation::findNodeHandleForConnector(const UUID& uuid) const
{
    try {
        return findNodeHandle(uuid.parentUUID());

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("cannot find handle of connector \"") + uuid.getFullName());
    }
}

NodeHandle* GraphImplementation::findNodeHandle(const UUID& uuid) const
{
    if (uuid.empty()) {
        apex_assert_hard(nf_);
        apex_assert_hard(nf_->getNodeHandle()->guard_ == -1);
        return nf_->getNodeHandle().get();
    }
    NodeHandle* node_handle = findNodeHandleNoThrow(uuid);
    if (node_handle) {
        apex_assert_hard(node_handle->guard_ == -1);
        return node_handle;
    }
    throw NodeHandleNotFoundException(uuid.getFullName());
}

NodeHandle* GraphImplementation::findNodeHandleNoThrow(const UUID& uuid) const noexcept
{
    if (uuid.empty()) {
        apex_assert_hard(nf_);
        apex_assert_hard(nf_->getNodeHandle()->guard_ == -1);
        return nf_->getNodeHandle().get();
    }
    if (uuid.composite()) {
        UUID root = uuid.rootUUID();

        NodeFacadePtr nf = findNodeFacade(root);
        if (nf->isGraph()) {
            GraphImplementationPtr local_graph = std::dynamic_pointer_cast<GraphImplementation>(nf->getSubgraph());
            apex_assert_hard(local_graph);

            return local_graph->findNodeHandleNoThrow(uuid.nestedUUID());
        }

    } else {
        for (const auto& vertex : vertices_) {
            NodeFacadePtr facade = vertex->getNodeFacade();
            if (facade->getUUID() == uuid) {
                NodeFacadeImplementationPtr local_facade = std::dynamic_pointer_cast<NodeFacadeImplementation>(facade);
                apex_assert_hard(local_facade);
                return local_facade->getNodeHandle().get();
            }
        }
    }

    return nullptr;
}

NodeHandle* GraphImplementation::findNodeHandleForConnectorNoThrow(const UUID& uuid) const noexcept
{
    return findNodeHandleNoThrow(uuid.parentUUID());
}

NodeHandle* GraphImplementation::findNodeHandleWithLabel(const std::string& label) const
{
    for (const auto& vertex : vertices_) {
        NodeFacadePtr facade = vertex->getNodeFacade();
        NodeStatePtr state = facade->getNodeState();
        if (state) {
            if (state->getLabel() == label) {
                NodeFacadeImplementationPtr local_facade = std::dynamic_pointer_cast<NodeFacadeImplementation>(facade);
                apex_assert_hard(local_facade);
                return local_facade->getNodeHandle().get();
            }
        }
    }
    return nullptr;
}

// *** NODE FACADE **** /

NodeFacadePtr GraphImplementation::findNodeFacadeForConnector(const UUID& uuid) const
{
    try {
        return findNodeFacade(uuid.parentUUID());

    } catch (const std::exception& e) {
        throw std::runtime_error(std::string("cannot find handle of connector \"") + uuid.getFullName());
    }
}

NodeFacadePtr GraphImplementation::findNodeFacade(const UUID& uuid) const
{
    if (uuid.empty()) {
        apex_assert_hard(nf_);
        return nf_->shared_from_this();
    }
    NodeFacadePtr node_facade = findNodeFacadeNoThrow(uuid);
    if (node_facade) {
        return node_facade;
    }
    throw NodeFacadeNotFoundException(uuid.getFullName());
}

NodeFacadePtr GraphImplementation::findNodeFacadeNoThrow(const UUID& uuid) const noexcept
{
    if (uuid.empty()) {
        apex_assert_hard(nf_);
        return nf_->shared_from_this();
    }
    if (uuid.composite()) {
        NodeFacadePtr root = findNodeFacadeNoThrow(uuid.rootUUID());
        if (root && root->isGraph()) {
            GraphPtr graph = root->getSubgraph();
            apex_assert_hard(graph);

            return graph->findNodeFacadeNoThrow(uuid.nestedUUID());
        }

    } else {
        for (const auto& vertex : vertices_) {
            NodeFacadePtr nf = vertex->getNodeFacade();
            if (nf->getUUID() == uuid) {
                return nf;
            }
        }
    }

    return nullptr;
}

NodeFacadePtr GraphImplementation::findNodeFacadeForConnectorNoThrow(const UUID& uuid) const noexcept
{
    return findNodeFacadeNoThrow(uuid.parentUUID());
}

NodeFacadePtr GraphImplementation::findNodeFacadeWithLabel(const std::string& label) const
{
    for (const auto& vertex : vertices_) {
        NodeFacadePtr nf = vertex->getNodeFacade();
        if (nf->getLabel() == label) {
            return nf;
        }
    }

    return nullptr;
}

// ******* /

Graph* GraphImplementation::findSubgraph(const UUID& uuid) const
{
    NodeHandle* nh = findNodeHandle(uuid);
    if (SubgraphNodePtr graph = std::dynamic_pointer_cast<SubgraphNode>(nh->getNode().lock())) {
        return graph->getGraph().get();
    }

    throw std::runtime_error(std::string("cannot find graph \"") + uuid.getFullName() + "\"");
}

std::vector<UUID> GraphImplementation::getAllNodeUUIDs() const
{
    std::vector<UUID> uuids;
    for (const auto& vertex : vertices_) {
        uuids.push_back(vertex->getUUID());
    }
    return uuids;
}

std::vector<NodeHandle*> GraphImplementation::getAllNodeHandles()
{
    std::vector<NodeHandle*> node_handles;
    for (const auto& vertex : vertices_) {
        NodeFacadePtr facade = vertex->getNodeFacade();
        NodeFacadeImplementationPtr local_facade = std::dynamic_pointer_cast<NodeFacadeImplementation>(facade);
        apex_assert_hard(local_facade);
        node_handles.push_back(local_facade->getNodeHandle().get());
    }

    return node_handles;
}

std::vector<NodeFacadePtr> GraphImplementation::getAllNodeFacades()
{
    std::vector<NodeFacadePtr> node_facades;
    for (const graph::VertexPtr& vertex : vertices_) {
        NodeFacadePtr nf = vertex->getNodeFacade();
        node_facades.push_back(nf);
    }

    return node_facades;
}
std::vector<NodeFacadeImplementationPtr> GraphImplementation::getAllLocalNodeFacades()
{
    std::vector<NodeFacadeImplementationPtr> node_facades;
    for (const graph::VertexPtr& vertex : vertices_) {
        NodeFacadeImplementationPtr nf = std::dynamic_pointer_cast<NodeFacadeImplementation>(vertex->getNodeFacade());
        apex_assert_hard(nf);
        node_facades.push_back(nf);
    }

    return node_facades;
}

ConnectablePtr GraphImplementation::findConnectable(const UUID& uuid)
{
    ConnectorPtr connector = findConnector(uuid);
    return std::dynamic_pointer_cast<Connectable>(connector);
}

ConnectorPtr GraphImplementation::findConnector(const UUID& uuid)
{
    ConnectorPtr res = findConnectorNoThrow(uuid);
    if (!res) {
        throw std::runtime_error(std::string("cannot find connector with UUID=") + uuid.getFullName());
    }
    return res;
}

ConnectorPtr GraphImplementation::findConnectorNoThrow(const UUID& uuid) noexcept
{
    NodeHandle* owner = findNodeHandleNoThrow(uuid.parentUUID());
    if (!owner) {
        return nullptr;
    }

    return owner->getConnectorNoThrow(uuid);
}

bool GraphImplementation::isConnected(const UUID& from, const UUID& to) const
{
    for (const auto& connection : edges_) {
        if (connection->from()->getUUID() == from && connection->to()->getUUID() == to) {
            return true;
        }
    }

    return false;
}

ConnectionPtr GraphImplementation::getConnectionWithId(int id)
{
    for (const ConnectionPtr& connection : edges_) {
        if (connection->id() == id) {
            return connection;
        }
    }

    return nullptr;
}

ConnectionPtr GraphImplementation::getConnection(const UUID& from, const UUID& to)
{
    for (const ConnectionPtr& connection : edges_) {
        if (connection->from()->getUUID() == from && connection->to()->getUUID() == to) {
            return connection;
        }
    }

    return nullptr;
}

Graph::vertex_iterator GraphImplementation::begin()
{
    return vertices_.begin();
}

const Graph::vertex_const_iterator GraphImplementation::begin() const
{
    return vertices_.cbegin();
}

Graph::vertex_iterator GraphImplementation::end()
{
    return vertices_.end();
}

const Graph::vertex_const_iterator GraphImplementation::end() const
{
    return vertices_.cend();
}
