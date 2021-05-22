#include <csapex/model/graph_facade.h>
#include <csapex/msg/generic_value_message.hpp>
#include <csapex/model/graph/graph_impl.h>
#include <csapex/model/execution_type.h>
#include <csapex/model/node_state.h>

#include <csapex_testing/test_exception_handler.h>
#include <csapex_testing/mockup_nodes.h>
#include <csapex_testing/stepping_test.h>

namespace csapex
{
class SchedulingTest : public SteppingTest
{
protected:
    NodeFacadeImplementationPtr makeNode(const std::string& type, const UUID& id, GraphPtr graph, ExecutionType exec_type)
    {
        NodeStatePtr state = std::make_shared<NodeState>(nullptr);
        state->setExecutionType(exec_type);
        NodeFacadeImplementationPtr nf = factory.makeNode(type, id, graph, state);

        EXPECT_NE(nullptr, nf);

        return nf;
    }

    void testStepping(ExecutionType exec_type)
    {
        NodeFacadeImplementationPtr src1 = makeNode("MockupSource", UUIDProvider::makeUUID_without_parent("src1"), graph, exec_type);
        main_graph_facade->addNode(src1);

        NodeFacadeImplementationPtr src2 = makeNode("MockupSource", UUIDProvider::makeUUID_without_parent("src2"), graph, exec_type);
        ASSERT_NE(nullptr, src2);
        main_graph_facade->addNode(src2);

        NodeFacadeImplementationPtr combiner = makeNode("DynamicMultiplier", UUIDProvider::makeUUID_without_parent("combiner"), graph, exec_type);
        ASSERT_NE(nullptr, combiner);
        main_graph_facade->addNode(combiner);

        NodeFacadeImplementationPtr sink_p = makeNode("MockupSink", UUIDProvider::makeUUID_without_parent("Sink"), graph, exec_type);
        main_graph_facade->addNode(sink_p);
        std::shared_ptr<MockupSink> sink = std::dynamic_pointer_cast<MockupSink>(sink_p->getNode());
        ASSERT_NE(nullptr, sink);

        // NESTED GRAPH
        NodeFacadeImplementationPtr sub_graph_node_facade = makeNode("csapex::Graph", graph->generateUUID("subgraph"), graph, exec_type);
        SubgraphNodePtr sub_graph = std::dynamic_pointer_cast<SubgraphNode>(sub_graph_node_facade->getNode());
        apex_assert_hard(sub_graph);

        GraphFacadeImplementation sub_graph_facade(executor, sub_graph->getLocalGraph(), sub_graph);

        NodeFacadeImplementationPtr m = makeNode("DynamicMultiplier", UUIDProvider::makeUUID_without_parent("m"), sub_graph->getLocalGraph(), exec_type);
        ASSERT_NE(nullptr, m);
        sub_graph_facade.addNode(m);

        apex_assert_hard(sub_graph_node_facade);
        graph->addNode(sub_graph_node_facade);

        auto type = connection_types::makeTokenType<connection_types::GenericValueMessage<int> >();

        auto in1_map = sub_graph->addForwardingInput(type, "forwarding", false);
        auto in2_map = sub_graph->addForwardingInput(type, "forwarding", false);
        auto out_map = sub_graph->addForwardingOutput(type, "forwarding");

        // forwarding connections
        sub_graph_facade.connect(in1_map.internal, m, "input_a");
        sub_graph_facade.connect(in2_map.internal, m, "input_b");
        sub_graph_facade.connect(m, "output", out_map.internal);

        // top level connections
        main_graph_facade->connect(combiner, "output", sink_p, "input");

        // crossing connections
        main_graph_facade->connect(src1, "output", in1_map.external);
        main_graph_facade->connect(src2, "output", in2_map.external);
        main_graph_facade->connect(out_map.external, combiner, "input_a");
        main_graph_facade->connect(out_map.external, combiner, "input_b");

        executor.start();

        // execution
        ASSERT_EQ(-1, sink->getValue());
        for (int iter = 0; iter < 100; ++iter) {
            ASSERT_NO_FATAL_FAILURE(step());

            int v = (iter * iter) * (iter * iter);
            ASSERT_EQ(v, sink->getValue());
        }
    }
};

TEST_F(SchedulingTest, SteppingWorksForProcessingGraphsDirect)
{
    // MAIN GRAPH
    testStepping(ExecutionType::DIRECT);
}

TEST_F(SchedulingTest, SteppingWorksForProcessingGraphsInSubprocess)
{
    // MAIN GRAPH
    testStepping(ExecutionType::SUBPROCESS);
}

TEST_F(SchedulingTest, SteppingWorksForSourceGraphs)
{
    // NESTED GRAPH
    NodeFacadeImplementationPtr sub_graph_node_facade = factory.makeNode("csapex::Graph", graph->generateUUID("subgraph"), graph);
    SubgraphNodePtr sub_graph = std::dynamic_pointer_cast<SubgraphNode>(sub_graph_node_facade->getNode());
    apex_assert_hard(sub_graph);

    NodeFacadeImplementationPtr sink_p = factory.makeNode("MockupSink", UUIDProvider::makeUUID_without_parent("Sink"), graph);
    main_graph_facade->addNode(sink_p);
    std::shared_ptr<MockupSink> sink = std::dynamic_pointer_cast<MockupSink>(sink_p->getNode());
    ASSERT_NE(nullptr, sink);

    GraphFacadeImplementation sub_graph_facade(executor, sub_graph->getLocalGraph(), sub_graph);

    NodeFacadeImplementationPtr src = factory.makeNode("MockupSource", UUIDProvider::makeUUID_without_parent("src"), graph);
    ASSERT_NE(nullptr, src);
    sub_graph_facade.addNode(src);

    apex_assert_hard(sub_graph_node_facade);
    graph->addNode(sub_graph_node_facade);

    auto type = connection_types::makeTokenType<connection_types::GenericValueMessage<int> >();

    auto out_map = sub_graph->addForwardingOutput(type, "forwarding");

    // crossing connections
    main_graph_facade->connect(out_map.external, sink_p, "input");

    // forwarding connections
    sub_graph_facade.connect(src, "output", out_map.internal);

    executor.start();

    auto source = std::dynamic_pointer_cast<MockupSource>(src->getNode());
    apex_assert_hard(source);

    // execution
    for (int iter = 0; iter < 100; ++iter) {
        ASSERT_NO_FATAL_FAILURE(step());

        ASSERT_TRUE(end_step_called);
        ASSERT_FALSE(end_step_called_more_than_once);
        ASSERT_EQ(iter + 1, source->getValue());
    }
}

TEST_F(SchedulingTest, SteppingWorksForUnconnectedSourceGraphs)
{
    // NESTED GRAPH
    NodeFacadeImplementationPtr sub_graph_node_facade = factory.makeNode("csapex::Graph", graph->generateUUID("subgraph"), graph);
    SubgraphNodePtr sub_graph = std::dynamic_pointer_cast<SubgraphNode>(sub_graph_node_facade->getNode());
    apex_assert_hard(sub_graph);

    GraphFacadeImplementation sub_graph_facade(executor, sub_graph->getLocalGraph(), sub_graph);

    NodeFacadeImplementationPtr src = factory.makeNode("MockupSource", UUIDProvider::makeUUID_without_parent("src"), graph);
    ASSERT_NE(nullptr, src);
    sub_graph_facade.addNode(src);

    apex_assert_hard(sub_graph_node_facade);
    graph->addNode(sub_graph_node_facade);

    auto type = connection_types::makeTokenType<connection_types::GenericValueMessage<int> >();

    auto out_map = sub_graph->addForwardingOutput(type, "forwarding");

    // forwarding connections
    sub_graph_facade.connect(src, "output", out_map.internal);

    executor.start();

    auto source = std::dynamic_pointer_cast<MockupSource>(src->getNode());
    apex_assert_hard(source);

    // execution
    for (int iter = 0; iter < 100; ++iter) {
        ASSERT_NO_FATAL_FAILURE(step());

        ASSERT_TRUE(end_step_called);
        ASSERT_FALSE(end_step_called_more_than_once);
        ASSERT_EQ(iter + 1, source->getValue());
    }
}

TEST_F(SchedulingTest, SteppingWorksForOutputToSlot)
{
}

TEST_F(SchedulingTest, SteppingWorksForEventToInput)
{
}
}  // namespace csapex
