#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Editor/Controller/NodeEditorController.h"

using namespace NodeEditorCore;

class ControllerTests : public ::testing::Test {
protected:
    std::shared_ptr<NodeEditorController> controller;
    
    void SetUp() override {
        controller = std::make_shared<NodeEditorController>();
    }
};

TEST_F(ControllerTests, AddNode) {
    int nodeId = controller->addNode("TestNode", "Default", Vec2(100, 100));
    EXPECT_EQ(nodeId, 1);
    
    auto model = controller->getModel();
    const NodeEditorModel::Node* node = model->getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->name, "TestNode");
    EXPECT_EQ(node->type, "Default");
    EXPECT_EQ(node->position.x, 100);
    EXPECT_EQ(node->position.y, 100);
}

TEST_F(ControllerTests, UpdateNode) {
    int nodeId = controller->addNode("TestNode", "Default", Vec2(100, 100));
    
    controller->updateNode(nodeId, [](Node& node) {
        node.name = "UpdatedNode";
        node.iconSymbol = "U";
        node.disabled = true;
    });
    
    auto model = controller->getModel();
    const NodeEditorModel::Node* node = model->getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->name, "UpdatedNode");
    EXPECT_EQ(node->iconSymbol, "U");
    EXPECT_TRUE(node->disabled);
}

TEST_F(ControllerTests, AddPin) {
    int nodeId = controller->addNode("TestNode", "Default", Vec2(100, 100));
    
    int pinId = controller->addPin(nodeId, "TestPin", true, PinType::Blue);
    EXPECT_EQ(pinId, 1);
    
    auto model = controller->getModel();
    const Pin* pin = model->getPin(nodeId, pinId);
    ASSERT_NE(pin, nullptr);
    EXPECT_EQ(pin->name, "TestPin");
    EXPECT_TRUE(pin->isInput);
    EXPECT_EQ(pin->type, PinType::Blue);
}

TEST_F(ControllerTests, UpdatePin) {
    int nodeId = controller->addNode("TestNode", "Default", Vec2(100, 100));
    int pinId = controller->addPin(nodeId, "TestPin", true, PinType::Blue);
    
    controller->updatePin(nodeId, pinId, [](Pin& pin) {
        pin.name = "UpdatedPin";
        pin.type = PinType::Red;
    });
    
    auto model = controller->getModel();
    const Pin* pin = model->getPin(nodeId, pinId);
    ASSERT_NE(pin, nullptr);
    EXPECT_EQ(pin->name, "UpdatedPin");
    EXPECT_EQ(pin->type, PinType::Red);
}

TEST_F(ControllerTests, AddConnection) {
    int node1 = controller->addNode("Node1", "Default", Vec2(100, 100));
    int node2 = controller->addNode("Node2", "Default", Vec2(200, 100));
    
    int pin1 = controller->addPin(node1, "Output", false, PinType::Blue);
    int pin2 = controller->addPin(node2, "Input", true, PinType::Blue);
    
    int connectionId = controller->addConnection(node1, pin1, node2, pin2);
    EXPECT_EQ(connectionId, 1);
    
    auto model = controller->getModel();
    const NodeEditorModel::Connection* connection = model->getConnection(connectionId);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->startNodeId, node1);
    EXPECT_EQ(connection->startPinId, pin1);
    EXPECT_EQ(connection->endNodeId, node2);
    EXPECT_EQ(connection->endPinId, pin2);
}

TEST_F(ControllerTests, AddGroup) {
    int groupId = controller->addGroup("TestGroup", Vec2(50, 50), Vec2(200, 200));
    EXPECT_EQ(groupId, 1);
    
    auto model = controller->getModel();
    const Group* group = model->getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->name, "TestGroup");
    EXPECT_EQ(group->position.x, 50);
    EXPECT_EQ(group->position.y, 50);
    EXPECT_EQ(group->size.x, 200);
    EXPECT_EQ(group->size.y, 200);
}

TEST_F(ControllerTests, UpdateGroup) {
    int groupId = controller->addGroup("TestGroup", Vec2(50, 50), Vec2(200, 200));
    
    controller->updateGroup(groupId, [](Group& group) {
        group.name = "UpdatedGroup";
        group.collapsed = true;
    });
    
    auto model = controller->getModel();
    const Group* group = model->getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->name, "UpdatedGroup");
    EXPECT_TRUE(group->collapsed);
}

TEST_F(ControllerTests, AddNodeToGroup) {
    int nodeId = controller->addNode("TestNode", "Default", Vec2(100, 100));
    int groupId = controller->addGroup("TestGroup", Vec2(50, 50), Vec2(200, 200));
    
    controller->addNodeToGroup(nodeId, groupId);
    
    auto model = controller->getModel();
    const Group* group = model->getGroup(groupId);
    ASSERT_NE(group, nullptr);
    EXPECT_TRUE(group->nodes.find(nodeId) != group->nodes.end());
    
    const NodeEditorModel::Node* node = model->getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->groupId, groupId);
}

TEST_F(ControllerTests, CreateSubgraph) {
    int subgraphId = controller->createSubgraph("TestSubgraph");
    EXPECT_GT(subgraphId, 0);
    
    auto model = controller->getModel();
    const Subgraph* subgraph = model->getSubgraph(subgraphId);
    ASSERT_NE(subgraph, nullptr);
    EXPECT_EQ(subgraph->name, "TestSubgraph");
}

TEST_F(ControllerTests, SelectDeselectNodes) {
    int node1 = controller->addNode("Node1", "Default", Vec2(100, 100));
    int node2 = controller->addNode("Node2", "Default", Vec2(200, 100));
    
    controller->selectNode(node1, false);
    
    std::vector<int> selected = controller->getSelectedNodes();
    ASSERT_EQ(selected.size(), 1);
    EXPECT_EQ(selected[0], node1);
    
    controller->selectNode(node2, true);
    
    selected = controller->getSelectedNodes();
    ASSERT_EQ(selected.size(), 2);
    
    controller->deselectNode(node1);
    
    selected = controller->getSelectedNodes();
    ASSERT_EQ(selected.size(), 1);
    EXPECT_EQ(selected[0], node2);
    
    controller->deselectAllNodes();
    
    selected = controller->getSelectedNodes();
    EXPECT_TRUE(selected.empty());
}

TEST_F(ControllerTests, EventListeners) {
    bool eventFired = false;
    controller->addEventListener(EventType::NodeCreated, [&eventFired](const Event& event) {
        eventFired = true;
    });
    
    controller->addNode("TestNode", "Default", Vec2(100, 100));
    EXPECT_TRUE(eventFired);
}