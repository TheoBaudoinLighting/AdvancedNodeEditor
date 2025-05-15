#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/NodeEditor.h"

using namespace NodeEditorCore;

class ConnectionTests : public ::testing::Test {
protected:
    NodeEditor editor;
    
    void SetUp() override {
        editor.addNode("Node1", "Default", Vec2(100, 100));
        editor.addNode("Node2", "Default", Vec2(300, 100));
        
        editor.addPin(1, "Output", false, PinType::Blue);
        editor.addPin(2, "Input", true, PinType::Blue);
    }
};

TEST_F(ConnectionTests, AddConnection) {
    int connectionId = editor.addConnection(1, 1, 2, 1);
    EXPECT_EQ(connectionId, 1);
    
    const Connection* connection = editor.getConnection(connectionId);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->startNodeId, 1);
    EXPECT_EQ(connection->startPinId, 1);
    EXPECT_EQ(connection->endNodeId, 2);
    EXPECT_EQ(connection->endPinId, 1);
    EXPECT_FALSE(connection->selected);
}

TEST_F(ConnectionTests, GetConnection) {
    editor.addConnection(1, 1, 2, 1);
    
    const Connection* connection = editor.getConnection(1);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->id, 1);
    
    const Connection* nonExistentConnection = editor.getConnection(999);
    EXPECT_EQ(nonExistentConnection, nullptr);
}

TEST_F(ConnectionTests, RemoveConnection) {
    int connectionId = editor.addConnection(1, 1, 2, 1);
    
    editor.removeConnection(connectionId);
    
    const Connection* connection = editor.getConnection(connectionId);
    EXPECT_EQ(connection, nullptr);
}

TEST_F(ConnectionTests, IsConnected) {
    editor.addConnection(1, 1, 2, 1);
    
    EXPECT_TRUE(editor.isConnected(1, 1));
    EXPECT_TRUE(editor.isConnected(2, 1));
    EXPECT_FALSE(editor.isConnected(1, 999));
}

TEST_F(ConnectionTests, ConnectionExistenceTests) {
    int connectionId = editor.addConnection(1, 1, 2, 1);
    EXPECT_NE(connectionId, -1);

    int duplicateId = editor.addConnection(1, 1, 2, 1);
    EXPECT_EQ(duplicateId, -1);

    int nonExistentPinId = editor.addConnection(1, 999, 2, 1);
    EXPECT_EQ(nonExistentPinId, -1);

    int reverseId = editor.addConnection(2, 1, 1, 1);
    EXPECT_EQ(reverseId, -1);
}

TEST_F(ConnectionTests, ConnectionByUUID) {
    UUID node1Uuid = editor.getNodeUUID(1);
    UUID node2Uuid = editor.getNodeUUID(2);
    UUID pin1Uuid = editor.getPinUUID(1, 1);
    UUID pin2Uuid = editor.getPinUUID(2, 1);
    
    UUID connectionUuid = editor.addConnectionWithUUIDByUUID(node1Uuid, pin1Uuid, node2Uuid, pin2Uuid);
    EXPECT_FALSE(connectionUuid.empty());
    
    const Connection* connection = editor.getConnectionByUUID(connectionUuid);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->startNodeId, 1);
    EXPECT_EQ(connection->startPinId, 1);
    EXPECT_EQ(connection->endNodeId, 2);
    EXPECT_EQ(connection->endPinId, 1);
    
    editor.removeConnectionByUUID(connectionUuid);
    connection = editor.getConnectionByUUID(connectionUuid);
    EXPECT_EQ(connection, nullptr);
}

TEST_F(ConnectionTests, SelectConnection) {
    int connectionId = editor.addConnection(1, 1, 2, 1);
    
    editor.selectConnection(connectionId, false);
    
    const Connection* connection = editor.getConnection(connectionId);
    ASSERT_NE(connection, nullptr);
    EXPECT_TRUE(connection->selected);
    
    editor.deselectConnection(connectionId);
    connection = editor.getConnection(connectionId);
    ASSERT_NE(connection, nullptr);
    EXPECT_FALSE(connection->selected);
}

TEST_F(ConnectionTests, DeselectAllConnections) {
    int conn1 = editor.addConnection(1, 1, 2, 1);

    editor.addPin(1, "Output2", false, PinType::Red);
    editor.addPin(2, "Input2", true, PinType::Red);
    int conn2 = editor.addConnection(1, 2, 2, 2);

    editor.selectConnection(conn1, false);
    editor.selectConnection(conn2, true);

    const Connection* connection1 = editor.getConnection(conn1);
    const Connection* connection2 = editor.getConnection(conn2);

    ASSERT_NE(connection1, nullptr);
    ASSERT_NE(connection2, nullptr);
    EXPECT_TRUE(connection1->selected);
    EXPECT_TRUE(connection2->selected);

    editor.deselectAllConnections();

    connection1 = editor.getConnection(conn1);
    connection2 = editor.getConnection(conn2);

    ASSERT_NE(connection1, nullptr);
    ASSERT_NE(connection2, nullptr);
    EXPECT_FALSE(connection1->selected);
    EXPECT_FALSE(connection2->selected);
}