#include <gtest/gtest.h>
#include "../../AdvancedNodeEditor/Core/NodeEditor.h"

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

    const Connection *connection = editor.getConnection(connectionId);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->startNodeId, 1);
    EXPECT_EQ(connection->startPinId, outputPinId);
    EXPECT_EQ(connection->endNodeId, 2);
    EXPECT_EQ(connection->endPinId, inputPinId);
    EXPECT_FALSE(connection->selected);
}

TEST_F(ConnectionTests, GetConnection) {
    int connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);

    const Connection *connection = editor.getConnection(1);
    ASSERT_NE(connection, nullptr);
    EXPECT_EQ(connection->id, 1);

    const Connection *nonExistentConnection = editor.getConnection(999);
    EXPECT_EQ(nonExistentConnection, nullptr);
}

TEST_F(ConnectionTests, RemoveConnection) {
    int connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);

    editor.removeConnection(connectionId);

    const Connection *connection = editor.getConnection(connectionId);
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

    const Connection *connection = editor.getConnectionByUUID(connectionUuid);
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

    const Connection *connection = editor.getConnection(connectionId);
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

    const Connection *connection1 = editor.getConnection(conn1);
    const Connection *connection2 = editor.getConnection(conn2);

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

class ConnectionSubgraphTests : public ::testing::Test {
protected:
    NodeEditor editor;
    int outputPinId;
    int inputPinId;
    int connectionId;
    int subgraphId;

    void SetUp() override {
        editor.addNode("Node1", "Default", Vec2(100, 100));
        editor.addNode("Node2", "Default", Vec2(300, 100));

        outputPinId = editor.addPin(1, "Output", false, PinType::Blue);
        inputPinId = editor.addPin(2, "Input", true, PinType::Blue);

        ASSERT_NE(outputPinId, -1);
        ASSERT_NE(inputPinId, -1);

        connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);
        ASSERT_NE(connectionId, -1);

        subgraphId = editor.createSubgraph("TestSubgraph", "", true);
        ASSERT_NE(subgraphId, -1);
    }
};

TEST_F(ConnectionSubgraphTests, ConnectionInSubgraph) {
    editor.addNodeToSubgraph(1, subgraphId);
    editor.addNodeToSubgraph(2, subgraphId);

    editor.addConnectionToSubgraph(connectionId, subgraphId);

    bool isInSubgraph = editor.isConnectionInSubgraph(connectionId, subgraphId);
    EXPECT_TRUE(isInSubgraph);

    std::vector<int> connections = editor.getConnectionsInSubgraph(subgraphId);
    EXPECT_FALSE(connections.empty());
    if (!connections.empty()) {
        EXPECT_EQ(connections[0], connectionId);
    }

    Connection* conn = editor.getConnection(connectionId);
    ASSERT_NE(conn, nullptr);
    EXPECT_EQ(conn->subgraphId, subgraphId);
}

TEST_F(ConnectionSubgraphTests, RemoveConnectionFromSubgraph) {
    editor.addNodeToSubgraph(1, subgraphId);
    editor.addNodeToSubgraph(2, subgraphId);
    editor.addConnectionToSubgraph(connectionId, subgraphId);

    EXPECT_TRUE(editor.isConnectionInSubgraph(connectionId, subgraphId));

    editor.removeConnectionFromSubgraph(connectionId, subgraphId);

    EXPECT_FALSE(editor.isConnectionInSubgraph(connectionId, subgraphId));

    std::vector<int> connections = editor.getConnectionsInSubgraph(subgraphId);
    EXPECT_TRUE(connections.empty());
}

TEST_F(ConnectionSubgraphTests, AddMultipleConnectionsToSubgraph) {
    int outputPinId2 = editor.addPin(1, "Output2", false, PinType::Red);
    int inputPinId2 = editor.addPin(2, "Input2", true, PinType::Red);
    int connectionId2 = editor.addConnection(1, outputPinId2, 2, inputPinId2);
    ASSERT_NE(connectionId2, -1);

    editor.addNodeToSubgraph(1, subgraphId);
    editor.addNodeToSubgraph(2, subgraphId);
    editor.addConnectionToSubgraph(connectionId, subgraphId);
    editor.addConnectionToSubgraph(connectionId2, subgraphId);

    EXPECT_TRUE(editor.isConnectionInSubgraph(connectionId, subgraphId));
    EXPECT_TRUE(editor.isConnectionInSubgraph(connectionId2, subgraphId));

    std::vector<int> connections = editor.getConnectionsInSubgraph(subgraphId);
    EXPECT_EQ(connections.size(), 2);
}

TEST(ConnectionSubgraphFix, TestSpecificIssue) {
    NodeEditorCore::NodeEditor editor;

    editor.addNode("Node1", "Default", NodeEditorCore::Vec2(100, 100));
    editor.addNode("Node2", "Default", NodeEditorCore::Vec2(300, 100));

    int outputPinId = editor.addPin(1, "Output", false, NodeEditorCore::PinType::Blue);
    int inputPinId = editor.addPin(2, "Input", true, NodeEditorCore::PinType::Blue);

    ASSERT_NE(outputPinId, -1);
    ASSERT_NE(inputPinId, -1);

    int connectionId = editor.addConnection(1, outputPinId, 2, inputPinId);
    std::cout << "Connection créée avec ID: " << connectionId << std::endl;
    ASSERT_NE(connectionId, -1);

    int subgraphId = editor.createSubgraph("TestSubgraph", "", true);

    editor.addNodeToSubgraph(1, subgraphId);
    editor.addNodeToSubgraph(2, subgraphId);

    editor.addConnectionToSubgraph(connectionId, subgraphId);

    bool isInSubgraph = editor.isConnectionInSubgraph(connectionId, subgraphId);
    std::cout << "La connexion " << connectionId << " est-elle dans le subgraph " << subgraphId << "? "
              << (isInSubgraph ? "Oui" : "Non") << std::endl;

    std::vector<int> connections = editor.getConnectionsInSubgraph(subgraphId);
    std::cout << "Nombre de connexions dans le subgraph: " << connections.size() << std::endl;

    NodeEditorCore::Subgraph* subgraph = editor.getSubgraph(subgraphId);
    if (subgraph) {
        std::cout << "Subgraph " << subgraphId << " (" << subgraph->name << "):" << std::endl;
        std::cout << "  Connexions IDs: ";
        for (int connId : subgraph->connectionIds) {
            std::cout << connId << " ";
        }
        std::cout << std::endl;
    }

    NodeEditorCore::Connection* connection = editor.getConnection(connectionId);
    if (connection) {
        std::cout << "Connexion " << connectionId << ":" << std::endl;
        std::cout << "  subgraphId: " << connection->subgraphId << std::endl;
        std::cout << "  metadata.subgraphId: " << connection->metadata.getAttribute<int>("subgraphId", -999) << std::endl;
    } else {
        std::cout << "Impossible de trouver la connexion " << connectionId << std::endl;
    }

    std::cout << "Toutes les connexions disponibles:" << std::endl;
    for (const auto& conn : editor.getConnections()) {
        std::cout << "  ID: " << conn.id
                  << ", de " << conn.startNodeId << ":" << conn.startPinId
                  << " à " << conn.endNodeId << ":" << conn.endPinId
                  << ", subgraphId: " << conn.subgraphId
                  << std::endl;
    }

    EXPECT_TRUE(editor.isConnectionInSubgraph(connectionId, subgraphId));
    EXPECT_FALSE(connections.empty());
    if (!connections.empty()) {
        EXPECT_EQ(connections[0], connectionId);
    }

    NodeEditorCore::Connection* conn = editor.getConnection(connectionId);
    ASSERT_NE(conn, nullptr);
    EXPECT_EQ(conn->getSubgraphId(), subgraphId);
}

TEST(ConnectionDebugging, DiagnoseSubgraphConnections) {
    NodeEditor editor;

    int node1 = editor.addNode("Node1", "Default", Vec2(100, 100));
    int node2 = editor.addNode("Node2", "Default", Vec2(300, 100));

    int pin1 = editor.addPin(node1, "Output", false, PinType::Blue);
    int pin2 = editor.addPin(node2, "Input", true, PinType::Blue);

    int connId = editor.addConnection(node1, pin1, node2, pin2);
    std::cout << "Connection créée avec ID: " << connId << std::endl;

    Connection *conn = editor.getConnection(connId);
    ASSERT_NE(conn, nullptr) << "La connexion n'a pas été créée correctement";

    int subgraphId = editor.createSubgraph("TestSubgraph", "", true);
    std::cout << "Subgraph créé avec ID: " << subgraphId << std::endl;

    Subgraph *subgraph = editor.getSubgraph(subgraphId);
    ASSERT_NE(subgraph, nullptr) << "Le subgraph n'a pas été créé correctement";

    editor.addNodeToSubgraph(node1, subgraphId);
    editor.addNodeToSubgraph(node2, subgraphId);

    editor.addConnectionToSubgraph(connId, subgraphId);

    EXPECT_TRUE(editor.isConnectionInSubgraph(connId, subgraphId))
        << "isConnectionInSubgraph retourne false alors qu'il devrait retourner true";

    std::vector<int> connections = editor.getConnectionsInSubgraph(subgraphId);
    EXPECT_FALSE(connections.empty())
        << "La liste des connexions du subgraph est vide alors qu'elle ne devrait pas l'être";

    if (!connections.empty()) {
        EXPECT_EQ(connections[0], connId)
            << "La première connexion du subgraph n'a pas l'ID attendu";
    }

    conn = editor.getConnection(connId);
    ASSERT_NE(conn, nullptr) << "Impossible de récupérer la connexion après l'avoir ajoutée au subgraph";

    EXPECT_EQ(conn->getSubgraphId(), subgraphId)
        << "L'attribut subgraphId de la connexion n'a pas été correctement mis à jour";

    std::cout << "Connexion " << connId << " dans le subgraph " << subgraphId << ":" << std::endl;
    std::cout << "  subgraphId: " << conn->subgraphId << std::endl;
    std::cout << "  metadata.subgraphId: " << conn->metadata.getAttribute<int>("subgraphId", -999) << std::endl;

    std::cout << "Liste des connexions du subgraph:" << std::endl;
    for (int id: connections) {
        std::cout << "  " << id << std::endl;
    }
}

TEST(ConnectionDiagnostic, VerifyPinsAndConnections) {
    NodeEditorCore::NodeEditor editor;

    int node1Id = editor.addNode("Node1", "Default", NodeEditorCore::Vec2(100, 100));
    int node2Id = editor.addNode("Node2", "Default", NodeEditorCore::Vec2(300, 100));

    std::cout << "Nœuds créés: " << node1Id << ", " << node2Id << std::endl;

    ASSERT_EQ(node1Id, 1) << "Premier nœud devrait avoir ID 1";
    ASSERT_EQ(node2Id, 2) << "Deuxième nœud devrait avoir ID 2";

    auto* node1 = editor.getNode(node1Id);
    auto* node2 = editor.getNode(node2Id);

    ASSERT_NE(node1, nullptr) << "Le nœud 1 devrait exister";
    ASSERT_NE(node2, nullptr) << "Le nœud 2 devrait exister";

    int pin1Id = editor.addPin(node1Id, "Output", false, NodeEditorCore::PinType::Blue);
    int pin2Id = editor.addPin(node2Id, "Input", true, NodeEditorCore::PinType::Blue);

    std::cout << "Pins créés: " << pin1Id << ", " << pin2Id << std::endl;

    ASSERT_GT(pin1Id, 0) << "Pin 1 devrait avoir un ID positif";
    ASSERT_GT(pin2Id, 0) << "Pin 2 devrait avoir un ID positif";

    auto* outputPin = node1->findPin(pin1Id);
    auto* inputPin = node2->findPin(pin2Id);

    ASSERT_NE(outputPin, nullptr) << "Le pin de sortie devrait exister dans le nœud 1";
    ASSERT_NE(inputPin, nullptr) << "Le pin d'entrée devrait exister dans le nœud 2";

    std::cout << "Pins dans le nœud 1:" << std::endl;
    std::cout << "  Entrées: ";
    for (const auto& pin : node1->inputs) {
        std::cout << pin.id << " ";
    }
    std::cout << std::endl;
    std::cout << "  Sorties: ";
    for (const auto& pin : node1->outputs) {
        std::cout << pin.id << " ";
    }
    std::cout << std::endl;

    std::cout << "Pins dans le nœud 2:" << std::endl;
    std::cout << "  Entrées: ";
    for (const auto& pin : node2->inputs) {
        std::cout << pin.id << " ";
    }
    std::cout << std::endl;
    std::cout << "  Sorties: ";
    for (const auto& pin : node2->outputs) {
        std::cout << pin.id << " ";
    }
    std::cout << std::endl;

    int connectionId = editor.addConnection(node1Id, pin1Id, node2Id, pin2Id);

    std::cout << "Connexion créée: " << connectionId << std::endl;

    ASSERT_GT(connectionId, 0) << "La connexion devrait avoir un ID positif";

    int subgraphId = editor.createSubgraph("TestSubgraph", "", true);
    std::cout << "Subgraph créé: " << subgraphId << std::endl;

    editor.addNodeToSubgraph(node1Id, subgraphId);
    editor.addNodeToSubgraph(node2Id, subgraphId);

    editor.addConnectionToSubgraph(connectionId, subgraphId);

    bool isInSubgraph = editor.isConnectionInSubgraph(connectionId, subgraphId);
    std::cout << "La connexion " << connectionId << " est dans le subgraph " << subgraphId << ": "
              << (isInSubgraph ? "Oui" : "Non") << std::endl;

    EXPECT_TRUE(isInSubgraph) << "La connexion devrait être dans le subgraph";

    std::vector<int> connections = editor.getConnectionsInSubgraph(subgraphId);

    std::cout << "Connexions dans le subgraph: ";
    for (int connId : connections) {
        std::cout << connId << " ";
    }
    std::cout << std::endl;

    EXPECT_FALSE(connections.empty()) << "La liste des connexions du subgraph ne devrait pas être vide";

    if (!connections.empty()) {
        EXPECT_EQ(connections[0], connectionId) << "La première connexion du subgraph devrait être celle qu'on a ajoutée";
    }
}