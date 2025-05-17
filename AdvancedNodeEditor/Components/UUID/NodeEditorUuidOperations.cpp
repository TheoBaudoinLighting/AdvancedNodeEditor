#include "../../NodeEditor.h"

namespace NodeEditorCore {
    UUID NodeEditor::getNodeUUID(int nodeId) const {
        for (const auto &node: getNodes()) {
            if (node.id == nodeId) {
                return node.uuid;
            }
        }
        return "";
    }

    int NodeEditor::getNodeId(const UUID &uuid) const {
        for (const auto &node: getNodes()) {
            if (node.uuid == uuid) {
                return node.id;
            }
        }
        return -1;
    }

    Node *NodeEditor::getNodeByUUID(const UUID &uuid) {
        int nodeId = getNodeId(uuid);
        if (nodeId == -1) return nullptr;
        return getNode(nodeId);
    }

    const Node *NodeEditor::getNodeByUUID(const UUID &uuid) const {
        int nodeId = getNodeId(uuid);
        if (nodeId == -1) return nullptr;
        return getNode(nodeId);
    }

    void NodeEditor::removeNodeByUUID(const UUID &uuid) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            removeNode(nodeId);
        }
    }

    void NodeEditor::selectNodeByUUID(const UUID &uuid, bool append) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            selectNode(nodeId, append);
        }
    }

    std::vector<UUID> NodeEditor::getSelectedNodeUUIDs() const {
        std::vector<UUID> result;
        std::vector<int> selectedNodes = getSelectedNodes();

        for (int nodeId: selectedNodes) {
            result.push_back(getNodeUUID(nodeId));
        }

        return result;
    }

    UUID NodeEditor::getPinUUID(int nodeId, int pinId) const {
        const Node *node = getNode(nodeId);
        if (!node) return "";

        for (const auto &pin: node->inputs) {
            if (pin.id == pinId) {
                return pin.uuid;
            }
        }

        for (const auto &pin: node->outputs) {
            if (pin.id == pinId) {
                return pin.uuid;
            }
        }

        return "";
    }

    Pin *NodeEditor::getPinByUUID(const UUID &nodeUuid, const UUID &pinUuid) {
        Node *node = getNodeByUUID(nodeUuid);
        if (!node) return nullptr;

        for (auto &pin: node->inputs) {
            if (pin.uuid == pinUuid) {
                static thread_local Pin result;
                result.id = pin.id;
                result.uuid = pin.uuid;
                result.name = pin.name;
                result.isInput = pin.isInput;
                result.type = static_cast<PinType>(pin.type);
                result.shape = static_cast<PinShape>(pin.shape);
                result.connected = pin.connected;
                return &result;
            }
        }

        for (auto &pin: node->outputs) {
            if (pin.uuid == pinUuid) {
                static thread_local Pin result;
                result.id = pin.id;
                result.uuid = pin.uuid;
                result.name = pin.name;
                result.isInput = pin.isInput;
                result.type = static_cast<PinType>(pin.type);
                result.shape = static_cast<PinShape>(pin.shape);
                result.connected = pin.connected;
                return &result;
            }
        }

        return nullptr;
    }

    const Pin *NodeEditor::getPinByUUID(const UUID &nodeUuid, const UUID &pinUuid) const {
        const Node *node = getNodeByUUID(nodeUuid);
        if (!node) return nullptr;

        for (const auto &pin: node->inputs) {
            if (pin.uuid == pinUuid) {
                static thread_local Pin result;
                result.id = pin.id;
                result.uuid = pin.uuid;
                result.name = pin.name;
                result.isInput = pin.isInput;
                result.type = static_cast<PinType>(pin.type);
                result.shape = static_cast<PinShape>(pin.shape);
                result.connected = pin.connected;
                return &result;
            }
        }

        for (const auto &pin: node->outputs) {
            if (pin.uuid == pinUuid) {
                static thread_local Pin result;
                result.id = pin.id;
                result.uuid = pin.uuid;
                result.name = pin.name;
                result.isInput = pin.isInput;
                result.type = static_cast<PinType>(pin.type);
                result.shape = static_cast<PinShape>(pin.shape);
                result.connected = pin.connected;
                return &result;
            }
        }

        return nullptr;
    }

    UUID NodeEditor::getConnectionUUID(int connectionId) const {
        for (const auto &connection: getConnections()) {
            if (connection.id == connectionId) {
                return connection.uuid;
            }
        }
        return "";
    }

    void NodeEditor::removeConnectionByUUID(const UUID &uuid) {
        for (const auto &connection: getConnections()) {
            if (connection.uuid == uuid) {
                removeConnection(connection.id);
                break;
            }
        }
    }

    Group *NodeEditor::getGroupByUUID(const UUID &uuid) {
        for (auto &group: m_state.groups) {
            if (group.uuid == uuid) {
                return &group;
            }
        }
        return nullptr;
    }

    void NodeEditor::removeGroupByUUID(const UUID &uuid) {
        for (size_t i = 0; i < m_state.groups.size(); i++) {
            if (m_state.groups[i].uuid == uuid) {
                removeGroup(m_state.groups[i].id);
                break;
            }
        }
    }

    void NodeEditor::addNodeToGroupByUUID(const UUID &nodeUuid, const UUID &groupUuid) {
        int nodeId = getNodeId(nodeUuid);
        if (nodeId == -1) return;

        Group *group = getGroupByUUID(groupUuid);
        if (group) {
            addNodeToGroup(nodeId, group->id);
        }
    }

    void NodeEditor::updateNodeUuidMap() {
        m_state.nodeUuidMap.clear();
        for (auto &node: m_state.nodes) {
            m_state.nodeUuidMap[node.uuid] = &node;
        }
    }

    void NodeEditor::updateConnectionUuidMap() {
        m_state.connectionUuidMap.clear();
        for (auto &connection: m_state.connections) {
            m_state.connectionUuidMap[connection.uuid] = &connection;
        }
    }

    void NodeEditor::updateGroupUuidMap() {
        m_state.groupUuidMap.clear();
        for (auto &group: m_state.groups) {
            m_state.groupUuidMap[group.uuid] = &group;
        }
    }
}
