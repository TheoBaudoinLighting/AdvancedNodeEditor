#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Core/NodeEditor.h"

using namespace NodeEditorCore;

class NodeEditorTests : public ::testing::Test {
protected:
    NodeEditor editor;
};

TEST_F(NodeEditorTests, AddNode) {
    int nodeId = editor.addNode("TestNode", "Default", Vec2(100, 100));
    EXPECT_EQ(nodeId, 1);
    
    const Node* node = editor.getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_EQ(node->name, "TestNode");
    EXPECT_EQ(node->type, "Default");
    EXPECT_EQ(node->position.x, 100);
    EXPECT_EQ(node->position.y, 100);
}

TEST_F(NodeEditorTests, RemoveNode) {
    int nodeId = editor.addNode("TestNode", "Default", Vec2(100, 100));
    
    editor.removeNode(nodeId);
    
    const Node* node = editor.getNode(nodeId);
    EXPECT_EQ(node, nullptr);
}

TEST_F(NodeEditorTests, AddPin) {
    int nodeId = editor.addNode("TestNode", "Default", Vec2(100, 100));
    
    int inputPinId = editor.addPin(nodeId, "Input", true, PinType::Blue);
    int outputPinId = editor.addPin(nodeId, "Output", false, PinType::Red);
    
    EXPECT_EQ(inputPinId, 1);
    EXPECT_EQ(outputPinId, 2);
    
    const Pin* inputPin = editor.getPin(nodeId, inputPinId);
    ASSERT_NE(inputPin, nullptr);
    EXPECT_EQ(inputPin->name, "Input");
    EXPECT_TRUE(inputPin->isInput);
    EXPECT_EQ(inputPin->type, PinType::Blue);
    
    const Pin* outputPin = editor.getPin(nodeId, outputPinId);
    ASSERT_NE(outputPin, nullptr);
    EXPECT_EQ(outputPin->name, "Output");
    EXPECT_FALSE(outputPin->isInput);
    EXPECT_EQ(outputPin->type, PinType::Red);
}

TEST_F(NodeEditorTests, RemovePin) {
    int nodeId = editor.addNode("TestNode", "Default", Vec2(100, 100));
    int pinId = editor.addPin(nodeId, "Input", true, PinType::Blue);
    
    editor.removePin(nodeId, pinId);
    
    const Pin* pin = editor.getPin(nodeId, pinId);
    EXPECT_EQ(pin, nullptr);
}

TEST_F(NodeEditorTests, SelectNode) {
    int node1 = editor.addNode("Node1", "Default", Vec2(100, 100));
    int node2 = editor.addNode("Node2", "Default", Vec2(200, 100));
    
    editor.selectNode(node1, false);
    
    const Node* node1Ptr = editor.getNode(node1);
    const Node* node2Ptr = editor.getNode(node2);
    ASSERT_NE(node1Ptr, nullptr);
    ASSERT_NE(node2Ptr, nullptr);
    
    EXPECT_TRUE(node1Ptr->selected);
    EXPECT_FALSE(node2Ptr->selected);
    
    editor.selectNode(node2, true);
    
    node1Ptr = editor.getNode(node1);
    node2Ptr = editor.getNode(node2);
    ASSERT_NE(node1Ptr, nullptr);
    ASSERT_NE(node2Ptr, nullptr);
    
    EXPECT_TRUE(node1Ptr->selected);
    EXPECT_TRUE(node2Ptr->selected);
}

TEST_F(NodeEditorTests, DeselectNode) {
    int nodeId = editor.addNode("TestNode", "Default", Vec2(100, 100));
    
    editor.selectNode(nodeId, false);
    editor.deselectNode(nodeId);
    
    const Node* node = editor.getNode(nodeId);
    ASSERT_NE(node, nullptr);
    EXPECT_FALSE(node->selected);
}

TEST_F(NodeEditorTests, SelectAllNodes) {
    editor.addNode("Node1", "Default", Vec2(100, 100));
    editor.addNode("Node2", "Default", Vec2(200, 100));
    editor.addNode("Node3", "Default", Vec2(300, 100));
    
    editor.selectAllNodes();
    
    std::vector<int> selectedNodes = editor.getSelectedNodes();
    EXPECT_EQ(selectedNodes.size(), 3);
}

TEST_F(NodeEditorTests, DeselectAllNodes) {
    editor.addNode("Node1", "Default", Vec2(100, 100));
    editor.addNode("Node2", "Default", Vec2(200, 100));
    
    editor.selectAllNodes();
    editor.deselectAllNodes();
    
    std::vector<int> selectedNodes = editor.getSelectedNodes();
    EXPECT_TRUE(selectedNodes.empty());
}

TEST_F(NodeEditorTests, ViewPositionAndScale) {
    editor.setViewPosition(Vec2(100, 100));
    editor.setViewScale(2.0f);
    
    EXPECT_EQ(editor.getViewPosition().x, 100);
    EXPECT_EQ(editor.getViewPosition().y, 100);
    EXPECT_EQ(editor.getViewScale(), 2.0f);
}

TEST_F(NodeEditorTests, CoordinateConversion) {
    editor.setViewPosition(Vec2(100, 100));
    editor.setViewScale(2.0f);
    
    Vec2 screenPos(150, 150);
    Vec2 canvasPos = editor.screenToCanvas(screenPos);
    
    EXPECT_EQ(canvasPos.x, 25);
    EXPECT_EQ(canvasPos.y, 25);
    
    Vec2 convertedScreenPos = editor.canvasToScreen(canvasPos);
    EXPECT_EQ(convertedScreenPos.x, 150);
    EXPECT_EQ(convertedScreenPos.y, 150);
}

TEST_F(NodeEditorTests, UUIDOperations) {
    UUID nodeUuid = editor.addNodeWithUUID("TestNode", "Default", Vec2(100, 100));
    EXPECT_FALSE(nodeUuid.empty());
    
    int nodeId = editor.getNodeId(nodeUuid);
    EXPECT_GT(nodeId, 0);
    
    UUID retrievedUuid = editor.getNodeUUID(nodeId);
    EXPECT_EQ(retrievedUuid, nodeUuid);
    
    editor.selectNodeByUUID(nodeUuid, false);
    
    std::vector<UUID> selectedUuids = editor.getSelectedNodeUUIDs();
    EXPECT_EQ(selectedUuids.size(), 1);
    EXPECT_EQ(selectedUuids[0], nodeUuid);
    
    editor.removeNodeByUUID(nodeUuid);
    
    Node* node = editor.getNodeByUUID(nodeUuid);
    EXPECT_EQ(node, nullptr);
}

TEST_F(NodeEditorTests, RegisterNodeType) {
    editor.registerNodeType("CustomNode", "Test", "Test custom node",
                          [](const Vec2& pos) -> Node* {
                              Node* node = new Node(1, "CustomNode", "CustomNode", pos);
                              node->setIconSymbol("C");
                              return node;
                          });
    
    Node* customNode = editor.createNodeOfType("CustomNode", Vec2(100, 100));
    ASSERT_NE(customNode, nullptr);
    EXPECT_EQ(customNode->type, "CustomNode");
    EXPECT_EQ(customNode->iconSymbol, "C");
}