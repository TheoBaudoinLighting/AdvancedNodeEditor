#include "NodeEditor.h"
#include <algorithm>

namespace NodeEditorCore {

    int NodeEditor::addGroup(const std::string& name, const Vec2& pos, const Vec2& size) {
        int groupId = m_state.nextGroupId++;
        m_state.groups.emplace_back(groupId, name, pos, size);
        return groupId;
    }

    void NodeEditor::removeGroup(int groupId) {
        auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                              [groupId](const Group& group) { return group.id == groupId; });

        if (it != m_state.groups.end()) {
            for (int nodeId : it->nodes) {
                Node* node = getNode(nodeId);
                if (node) node->groupId = -1;
            }

            m_state.groups.erase(it);
        }
    }

    Group* NodeEditor::getGroup(int groupId) {
        auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                              [groupId](const Group& group) { return group.id == groupId; });
        return it != m_state.groups.end() ? &(*it) : nullptr;
    }

    const Group* NodeEditor::getGroup(int groupId) const {
        auto it = std::find_if(m_state.groups.begin(), m_state.groups.end(),
                              [groupId](const Group& group) { return group.id == groupId; });
        return it != m_state.groups.end() ? &(*it) : nullptr;
    }

    void NodeEditor::addNodeToGroup(int nodeId, int groupId) {
        Group* group = getGroup(groupId);
        Node* node = getNode(nodeId);

        if (!group || !node) return;

        if (node->groupId >= 0 && node->groupId != groupId) {
            Group* oldGroup = getGroup(node->groupId);
            if (oldGroup) {
                oldGroup->nodes.erase(nodeId);
            }
        }

        node->groupId = groupId;
        group->nodes.insert(nodeId);
    }

    void NodeEditor::removeNodeFromGroup(int nodeId, int groupId) {
        Group* group = getGroup(groupId);
        Node* node = getNode(nodeId);

        if (!group || !node || node->groupId != groupId) return;

        node->groupId = -1;
        group->nodes.erase(nodeId);
    }

}