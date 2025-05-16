#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/NodeEditor.h"

using namespace NodeEditorCore;

class ConnectionTests : public ::testing::Test {
protected:
    NodeEditor editor;
    int outputPinId;
    int inputPinId;

    void SetUp() override {
        editor.addNode("Node1", "Default", Vec2(100, 100));
        editor.addNode("Node2", "Default", Vec2(300, 100));

        outputPinId = editor.addPin(1, "Output", false, PinType::Blue);
        inputPinId = editor.addPin(2, "Input", true, PinType::Blue);

        ASSERT_NE(outputPinId, -1);
        ASSERT_NE(inputPinId, -1);
    }
};

TEST_F(ConnectionTests, AddConnection) {
    int connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);
    EXPECT_EQ(connectionId, 1);

    const Connection* connection = editor.getConnection(connectionId);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->startNodeId, 1);
    EXPECT_EQ(connection->startPinId, outputPinId);
    EXPECT_EQ(connection->endNodeId, 2);
    EXPECT_EQ(connection->endPinId, inputPinId);
    EXPECT_FALSE(connection->selected);
}

TEST_F(ConnectionTests, GetConnection) {
    int connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);

    const Connection* connection = editor.getConnection(1);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->id, 1);

    const Connection* nonExistentConnection = editor.getConnection(999);
    EXPECT_EQ(nonExistentConnection, nullptr);
}

TEST_F(ConnectionTests, RemoveConnection) {
    int connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);

    editor.removeConnection(connectionId);

    const Connection* connection = editor.getConnection(connectionId);
    EXPECT_EQ(connection, nullptr);
}

TEST_F(ConnectionTests, IsConnected) {
    editor.addConnection(1, outputPinId, 2, inputPinId);

    EXPECT_TRUE(editor.isConnected(1, outputPinId));
    EXPECT_TRUE(editor.isConnected(2, inputPinId));
    EXPECT_FALSE(editor.isConnected(1, 999));
}

TEST_F(ConnectionTests, ConnectionExistenceTests) {
    int connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);
    EXPECT_NE(connectionId, -1);

    int duplicateId = editor.addConnection(1, outputPinId, 2, inputPinId);
    EXPECT_EQ(duplicateId, -1);

    int nonExistentPinId = editor.addConnection(1, 999, 2, inputPinId);
    EXPECT_EQ(nonExistentPinId, -1);

    int reverseId = editor.addConnection(2, inputPinId, 1, outputPinId);
    EXPECT_EQ(reverseId, -1);
}

TEST_F(ConnectionTests, ConnectionByUUID) {
    UUID node1Uuid = editor.getNodeUUID(1);
    UUID node2Uuid = editor.getNodeUUID(2);
    UUID pin1Uuid = editor.getPinUUID(1, outputPinId);
    UUID pin2Uuid = editor.getPinUUID(2, inputPinId);

    UUID connectionUuid = editor.addConnectionWithUUIDByUUID(node1Uuid, pin1Uuid, node2Uuid, pin2Uuid);
    EXPECT_FALSE(connectionUuid.empty());

    const Connection* connection = editor.getConnectionByUUID(connectionUuid);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->startNodeId, 1);
    EXPECT_EQ(connection->startPinId, outputPinId);
    EXPECT_EQ(connection->endNodeId, 2);
    EXPECT_EQ(connection->endPinId, inputPinId);

    editor.removeConnectionByUUID(connectionUuid);
    connection = editor.getConnectionByUUID(connectionUuid);
    EXPECT_EQ(connection, nullptr);
}

TEST_F(ConnectionTests, SelectConnection) {
    int connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);

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
    int conn1 = editor.addConnection(1, outputPinId, 2, inputPinId);

    int outputPinId2 = editor.addPin(1, "Output2", false, PinType::Red);
    int inputPinId2 = editor.addPin(2, "Input2", true, PinType::Red);
    int conn2 = editor.addConnection(1, outputPinId2, 2, inputPinId2);

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