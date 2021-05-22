// #include <csapex/model/graph/graph_impl.h>
// #include <csapex/model/node.h>
// #include <csapex/model/node_handle.h>
// #include <csapex/model/node_facade_impl.h>
// #include <csapex/factory/node_factory_impl.h>
// #include <csapex/core/settings/settings_impl.h>
// #include <csapex/utility/uuid_provider.h>
// #include <csapex/model/subgraph_node.h>
// #include <csapex/model/graph/graph_impl.h>
// #include <csapex/msg/any_message.h>
// #include <csapex/msg/input.h>
// #include <csapex/msg/message_template.hpp>
// #include <csapex/msg/direct_connection.h>
// #include <csapex/msg/static_output.h>
// #include <csapex/msg/input.h>
// #include <csapex/utility/register_msg.h>

// #include <csapex_testing/mockup_msgs.h>
// #include <csapex_testing/stepping_test.h>
// #include <csapex_testing/csapex_test_case.h>

// namespace csapex
// {
// class TokenTypeTest : public SteppingTest
// {
// protected:
//     NodeFactoryImplementation factory;

//     TokenTypeTest() : factory(SettingsImplementation::NoSettings, nullptr), uuid_provider(std::make_shared<UUIDProvider>())
//     {
//     }

//     virtual ~TokenTypeTest()
//     {
//         // You can do clean-up work that doesn't throw exceptions here.
//     }

//     UUIDProviderPtr uuid_provider;
// };

// TEST_F(TokenTypeTest, BaseCanBeConnectedToBase)
// {
//     OutputPtr o = std::make_shared<StaticOutput>(uuid_provider->makeUUID("o1"));
//     InputPtr i = std::make_shared<Input>(uuid_provider->makeUUID("in"));

//     o->setType(std::make_shared<connection_types::BaseMessage>());
//     i->setType(std::make_shared<connection_types::BaseMessage>());

//     // is compatible
//     ASSERT_TRUE(Connection::isCompatibleWith(o.get(), i.get()));
//     ASSERT_TRUE(Connection::isCompatibleWith(i.get(), o.get()));
//     // can be connected
//     ASSERT_TRUE(Connection::canBeConnectedTo(o.get(), i.get()));
//     ASSERT_TRUE(Connection::canBeConnectedTo(i.get(), o.get()));

//     ConnectionPtr c = DirectConnection::connect(o, i);
//     ASSERT_NE(nullptr, c);
// }

// TEST_F(TokenTypeTest, ChildCanBeConnectedToChild)
// {
//     OutputPtr o = std::make_shared<StaticOutput>(uuid_provider->makeUUID("o1"));
//     InputPtr i = std::make_shared<Input>(uuid_provider->makeUUID("in"));

//     o->setType(std::make_shared<connection_types::ChildMessage>());
//     i->setType(std::make_shared<connection_types::ChildMessage>());

//     // is compatible
//     ASSERT_TRUE(Connection::isCompatibleWith(o.get(), i.get()));
//     ASSERT_TRUE(Connection::isCompatibleWith(i.get(), o.get()));
//     // can be connected
//     ASSERT_TRUE(Connection::canBeConnectedTo(o.get(), i.get()));
//     ASSERT_TRUE(Connection::canBeConnectedTo(i.get(), o.get()));

//     ConnectionPtr c = DirectConnection::connect(o, i);
//     ASSERT_NE(nullptr, c);
// }

// TEST_F(TokenTypeTest, BaseCannotBeConnectedToChild)
// {
//     OutputPtr o = std::make_shared<StaticOutput>(uuid_provider->makeUUID("o1"));
//     InputPtr i = std::make_shared<Input>(uuid_provider->makeUUID("in"));

//     o->setType(std::make_shared<connection_types::BaseMessage>());
//     i->setType(std::make_shared<connection_types::ChildMessage>());

//     // is compatible
//     ASSERT_FALSE(Connection::isCompatibleWith(o.get(), i.get()));
//     ASSERT_TRUE(Connection::isCompatibleWith(i.get(), o.get()));
//     // cannot be connected in both directions
//     ASSERT_FALSE(Connection::canBeConnectedTo(o.get(), i.get()));  // would involve down-cast
//     ASSERT_TRUE(Connection::canBeConnectedTo(i.get(), o.get()));   // needs upcast (ok)

//     ASSERT_EQ(nullptr, DirectConnection::connect(o, i));
// }

// TEST_F(TokenTypeTest, TestRenamed)
// {
//     SUCCEED();
// }

// }  // namespace csapex
