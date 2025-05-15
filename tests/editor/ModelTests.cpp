#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Editor/Model/NodeEditorModel.h"

using namespace NodeEditorCore;

class ModelTests : public ::testing::Test {
protected:
    std::shared_ptr<NodeEditorModel> model;
    
    void SetUp() override {
        model = std::make_shared<NodeEditorModel>();
    }
};

TEST_F(ModelTests, AddNode) {
    int nodeId = model->addNode("TestNode", "Default", Vec2(100, 100));
    EXPECT_EQ(nodeId, 1);
    
    const NodeEditorModel::Node* node = model->getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->name, "TestNode");
    EXPECT_EQ(node->type, "Default");
    EXPECT_EQ(node->position.x, 100);
    EXPECT_EQ(node->position.y, 100);
}

TEST_F(ModelTests, RemoveNode) {
    int nodeId = model->addNode("TestNode", "Default", Vec2(100, 100));
    
    model->removeNode(nodeId);
    
    const NodeEditorModel::Node* node = model->getNode(nodeId);
    EXPECT_EQ(node, nullptr);
}

TEST_F(ModelTests, GetNodes) {
    model->addNode("Node1", "Default", Vec2(100, 100));
    model->addNode("Node2", "Default", Vec2(200, 100));
    
    const auto& nodes = model->getNodes();
    EXPECT_EQ(nodes.size(), 2);
}

TEST_F(ModelTests, AddPin) {
    int nodeId = model->addNode("TestNode", "Default", Vec2(100, 100));
    
    int pinId = model->addPin(nodeId, "TestPin", true, PinType::Blue);
    EXPECT_EQ(pinId, 1);
    
    const Pin* pin = model->getPin(nodeId, pinId);
    ASSERT_NE(pin, nullptr);
    EXPECT_EQ(pin->name, "TestPin");
    EXPECT_TRUE(pin->isInput);
    EXPECT_EQ(pin->type, PinType::Blue);
}

TEST_F(ModelTests, RemovePin) {
    int nodeId = model->addNode("TestNode", "Default", Vec2(100, 100));
    int pinId = model->addPin(nodeId, "TestPin", true, PinType::Blue);
    
    model->removePin(nodeId, pinId);
    
    const Pin* pin = model->getPin(nodeId, pinId);
    EXPECT_EQ(pin, nullptr);
}

TEST_F(ModelTests, AddConnection) {
    int node1 = model->addNode("Node1", "Default", Vec2(100, 100));
    int node2 = model->addNode("Node2", "Default", Vec2(200, 100));
    
    int pin1 = model->addPin(node1, "Output", false, PinType::Blue);
    int pin2 = model->addPin(node2, "Input", true, PinType::Blue);
    
    int connectionId = model->addConnection(node1, pin1, node2, pin2);
    EXPECT_EQ(connectionId, 1);
    
    const NodeEditorModel::Connection* connection = model->getConnection(connectionId);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->startNodeId, node1);
    EXPECT_EQ(connection->startPinId, pin1);
    EXPECT_EQ(connection->endNodeId, node2);
    EXPECT_EQ(connection->endPinId, pin2);
}

TEST_F(ModelTests, RemoveConnection) {
    int node1 = model->addNode("Node1", "Default", Vec2(100, 100));
    int node2 = model->addNode("Node2", "Default", Vec2(200, 100));
    
    int pin1 = model->addPin(node1, "Output", false, PinType::Blue);
    int pin2 = model->addPin(node2, "Input", true, PinType::Blue);
    
    int connectionId = model->addConnection(node1, pin1, node2, pin2);
    
    model->removeConnection(connectionId);
    
    const NodeEditorModel::Connection* connection = model->getConnection(connectionId);
    EXPECT_EQ(connection, nullptr);
}

TEST_F(ModelTests, IsConnected) {
    int node1 = model->addNode("Node1", "Default", Vec2(100, 100));
    int node2 = model->addNode("Node2", "Default", Vec2(200, 100));
    
    int pin1 = model->addPin(node1, "Output", false, PinType::Blue);
    int pin2 = model->addPin(node2, "Input", true, PinType::Blue);
    
    model->addConnection(node1, pin1, node2, pin2);
    
    EXPECT_TRUE(model->isConnected(node1, pin1));
    EXPECT_TRUE(model->isConnected(node2, pin2));
    EXPECT_FALSE(model->isConnected(node1, 999));
}

TEST_F(ModelTests, SelectNode) {
    int node1 = model->addNode("Node1", "Default", Vec2(100, 100));
    int node2 = model->addNode("Node2", "Default", Vec2(200, 100));
    
    model->selectNode(node1, false);
    
    std::vector<int> selected = model->getSelectedNodes();
    ASSERT_EQ(selected.size(), 1);
    EXPECT_EQ(selected[0], node1);
    
    model->selectNode(node2, true);
    
    selected = model->getSelectedNodes();
    ASSERT_EQ(selected.size(), 2);
    EXPECT_EQ(selected[0], node1);
    EXPECT_EQ(selected[1], node2);
}

TEST_F(ModelTests, DeselectNode) {
    int node1 = model->addNode("Node1", "Default", Vec2(100, 100));
    int node2 = model->addNode("Node2", "Default", Vec2(200, 100));
    
    model->selectNode(node1, false);
    model->selectNode(node2, true);
    
    model->deselectNode(node1);
    
    std::vector<int> selected = model->getSelectedNodes();
    ASSERT_EQ(selected.size(), 1);
    EXPECT_EQ(selected[0], node2);
}

TEST_F(ModelTests, SelectAllNodes) {
    model->addNode("Node1", "Default", Vec2(100, 100));
    model->addNode("Node2", "Default", Vec2(200, 100));
    model->addNode("Node3", "Default", Vec2(300, 100));
    
    model->selectAllNodes();
    
    std::vector<int> selected = model->getSelectedNodes();
    EXPECT_EQ(selected.size(), 3);
}

TEST_F(ModelTests, DeselectAllNodes) {
    model->addNode("Node1", "Default", Vec2(100, 100));
    model->addNode("Node2", "Default", Vec2(200, 100));
    
    model->selectAllNodes();
    model->deselectAllNodes();
    
    std::vector<int> selected = model->getSelectedNodes();
    EXPECT_TRUE(selected.empty());
}

TEST_F(ModelTests, AddGroup) {
    int groupId = model->addGroup("TestGroup", Vec2(50, 50), Vec2(200, 200));
    EXPECT_EQ(groupId, 1);
    
    const Group* group = model->getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->name, "TestGroup");
    EXPECT_EQ(group->position.x, 50);
    EXPECT_EQ(group->position.y, 50);
    EXPECT_EQ(group->size.x, 200);
    EXPECT_EQ(group->size.y, 200);
}

TEST_F(ModelTests, AddNodeToGroup) {
    int nodeId = model->addNode("TestNode", "Default", Vec2(100, 100));
    int groupId = model->addGroup("TestGroup", Vec2(50, 50), Vec2(200, 200));
    
    model->addNodeToGroup(nodeId, groupId);
    
    const Group* group = model->getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_TRUE(group->nodes.find(nodeId) != group->nodes.end());
    
    const NodeEditorModel::Node* node = model->getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->groupId, groupId);
}

TEST_F(ModelTests, CreateSubgraph) {
    int subgraphId = model->createSubgraph("TestSubgraph");
    EXPECT_GT(subgraphId, 0);
    
    const Subgraph* subgraph = model->getSubgraph(subgraphId);
    ASSERT_NE(subgraph, nullptr);
    EXPECT_EQ(subgraph->name, "TestSubgraph");
}

TEST_F(ModelTests, SetGetState) {
    model->setState("testKey", 42);
    model->setState("testString", std::string("value"));
    
    EXPECT_TRUE(model->hasState("testKey"));
    EXPECT_EQ(model->getState<int>("testKey"), 42);
    EXPECT_EQ(model->getState<std::string>("testString"), "value");
    EXPECT_EQ(model->getState<int>("nonExistent", 100), 100);
}

TEST_F(ModelTests, EventListeners) {
    bool eventFired = false;
    model->addEventListener(EventType::NodeCreated, [&eventFired](const Event& event) {
        eventFired = true;
    });
    
    model->addNode("TestNode", "Default", Vec2(100, 100));
    EXPECT_TRUE(eventFired);
}