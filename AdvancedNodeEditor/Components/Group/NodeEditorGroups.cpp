#include "../../Core/NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {
    int NodeEditor::addGroup(const std::string &name, const Vec2 &pos, const Vec2 &size, const UUID &uuid) {
        int groupId = m_state.nextGroupId++;
        m_state.groups.emplace_back(groupId, name, pos, size);

        Group *group = getGroup(groupId);
        if (group) {
            group->uuid = uuid.empty() ? generateUUID() : uuid;
            updateGroupUuidMap();
        }

        return groupId;
    }

    void NodeEditor::removeGroup(int groupId) {
        auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                               [groupId](const Group &group) { return group.id == groupId; });

        if (it != m_state.groups.end()) {
            for (int nodeId: it->nodes) {
                Node *node = getNode(nodeId);
                if (node) node->groupId = -1;
            }

            m_state.groups.erase(it);
            updateGroupUuidMap();
        }
    }

    Group *NodeEditor::getGroup(int groupId) {
        auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                               [groupId](const Group &group) { return group.id == groupId; });
        return it != m_state.groups.end() ? &(*it) : nullptr;
    }

    const Group *NodeEditor::getGroup(int groupId) const {
        auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                               [groupId](const Group &group) { return group.id == groupId; });
        return it != m_state.groups.end() ? &(*it) : nullptr;
    }

    UUID NodeEditor::getGroupUUID(int groupId) const {
        const Group *group = getGroup(groupId);
        return group ? group->uuid : "";
    }

    int NodeEditor::getGroupId(const UUID &uuid) const {
        auto it = m_state.groupUuidMap.find(uuid);
        return it != m_state.groupUuidMap.end() ? it->second->id : -1;
    }

    void NodeEditor::addNodeToGroup(int nodeId, int groupId) {
        Group *group = getGroup(groupId);
        Node *node = getNode(nodeId);

        if (!group || !node) return;

        if (node->groupId >= 0 && node->groupId != groupId) {
            Group *oldGroup = getGroup(node->groupId);
            if (oldGroup) {
                oldGroup->nodes.erase(nodeId);
                oldGroup->nodeUuids.erase(node->uuid);
            }
        }

        node->groupId = groupId;
        group->nodes.insert(nodeId);
        group->nodeUuids.insert(node->uuid);
    }

    void NodeEditor::removeNodeFromGroup(int nodeId, int groupId) {
        Group *group = getGroup(groupId);
        Node *node = getNode(nodeId);

        if (!group || !node || node->groupId != groupId) return;

        node->groupId = -1;
        group->nodes.erase(nodeId);
        group->nodeUuids.erase(node->uuid);
    }
}
