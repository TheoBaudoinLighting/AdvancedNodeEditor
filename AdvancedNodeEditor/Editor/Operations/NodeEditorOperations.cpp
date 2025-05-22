#include "../../Core/NodeEditor.h"
#include <algorithm>
#include <cfloat>

namespace NodeEditorCore {
    void NodeEditor::processBoxSelection(const ImVec2 &canvasPos) {
        ImVec2 mousePos = ImGui::GetMousePos();
        ImVec2 boxMin = ImVec2(
            std::min(m_state.boxSelectStart.x, mousePos.x),
            std::min(m_state.boxSelectStart.y, mousePos.y)
        );

        ImVec2 boxMax = ImVec2(
            std::max(m_state.boxSelectStart.x, mousePos.x),
            std::max(m_state.boxSelectStart.y, mousePos.y)
        );

        for (auto &node: m_state.nodes) {
            ImVec2 nodePos = canvasToScreen(node.position).toImVec2();
            ImVec2 nodeSize = Vec2(node.size.x * m_state.viewScale, node.size.y * m_state.viewScale).toImVec2();
            ImVec2 nodeMin = nodePos;
            ImVec2 nodeMax = ImVec2(nodePos.x + nodeSize.x, nodePos.y + nodeSize.y);

            bool intersected = !(nodeMax.x < boxMin.x || nodeMin.x > boxMax.x ||
                                 nodeMax.y < boxMin.y || nodeMin.y > boxMax.y);

            if (intersected) {
                node.selected = true;
            } else if (!ImGui::GetIO().KeyCtrl) {
                node.selected = false;
            }
        }
    }

    void NodeEditor::selectNode(int nodeId, bool append) {
        if (!append) {
            for (auto &node : m_state.nodes) {
                node.selected = false;
            }
        }

        for (auto &node : m_state.nodes) {
            if (node.id == nodeId) {
                node.selected = true;
                break;
            }
        }
    }

    void NodeEditor::deselectNode(int nodeId) {
        for (auto &node : m_state.nodes) {
            if (node.id == nodeId) {
                node.selected = false;
                break;
            }
        }
    }

    void NodeEditor::selectAllNodes() {
        for (auto &node : m_state.nodes) {
            if ((m_state.currentSubgraphId >= 0 && node.subgraphId == m_state.currentSubgraphId) ||
                (m_state.currentSubgraphId == -1 && node.subgraphId == -1)) {
                node.selected = true;
                }
        }
    }

    void NodeEditor::deselectAllNodes() {
        for (auto &node : m_state.nodes) {
            node.selected = false;
        }
    }

    std::vector<int> NodeEditor::getSelectedNodes() const {
        std::vector<int> selectedNodes;
        for (const auto &node : m_state.nodes) {
            if (node.selected) {
                selectedNodes.push_back(node.id);
            }
        }
        return selectedNodes;
    }

    void NodeEditor::setViewPosition(const Vec2 &position) {
        m_state.viewPosition = position;
    }

    Vec2 NodeEditor::getViewPosition() const {
        return m_state.viewPosition;
    }

    void NodeEditor::setViewScale(float scale) {
        m_state.viewScale = std::max(0.1f, std::min(scale, 5.0f));
    }

    float NodeEditor::getViewScale() const {
        return m_state.viewScale;
    }

    void NodeEditor::centerViewWithSize(float windowWidth, float windowHeight) {
        if (m_state.nodes.empty()) {
            m_state.viewPosition = Vec2(0, 0);
            return;
        }

        Vec2 min(FLT_MAX, FLT_MAX);
        Vec2 max(-FLT_MAX, -FLT_MAX);

        for (const auto &node: m_state.nodes) {
            min.x = std::min(min.x, node.position.x);
            min.y = std::min(min.y, node.position.y);
            max.x = std::max(max.x, node.position.x + node.size.x);
            max.y = std::max(max.y, node.position.y + node.size.y);
        }

        if (min.x == FLT_MAX) {
            m_state.viewPosition = Vec2(0, 0);
            return;
        }

        Vec2 center = Vec2(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        );

        m_state.viewPosition = Vec2(
            windowWidth * 0.5f - center.x * m_state.viewScale,
            windowHeight * 0.5f - center.y * m_state.viewScale
        );
    }


    void NodeEditor::centerOnNodeWithSize(int nodeId, float windowWidth, float windowHeight) {
        const Node *node = getNode(nodeId);
        if (!node) return;

        Vec2 center = Vec2(
            node->position.x + node->size.x * 0.5f,
            node->position.y + node->size.y * 0.5f
        );

        m_state.viewPosition = Vec2(
            windowWidth * 0.5f - center.x * m_state.viewScale,
            windowHeight * 0.5f - center.y * m_state.viewScale
        );
    }


    void NodeEditor::setStyle(const NodeEditorStyle &style) {
        m_state.style = style;
    }

    void NodeEditor::setNodeCreatedCallback(NodeCallback callback) {
        m_state.nodeCreatedCallback = callback;
    }

    void NodeEditor::setNodeRemovedCallback(NodeCallback callback) {
        m_state.nodeRemovedCallback = callback;
    }

    void NodeEditor::setConnectionCreatedCallback(ConnectionCallback callback) {
        m_state.connectionCreatedCallback = callback;
    }

    void NodeEditor::setConnectionRemovedCallback(ConnectionCallback callback) {
        m_state.connectionRemovedCallback = callback;
    }

    void NodeEditor::setCanConnectCallback(CanConnectCallback callback) {
        m_state.canConnectCallback = callback;
    }

    Vec2 NodeEditor::screenToCanvas(const Vec2 &pos) const {
        return Vec2(
            (pos.x - m_state.viewPosition.x) / m_state.viewScale,
            (pos.y - m_state.viewPosition.y) / m_state.viewScale
        );
    }

    Vec2 NodeEditor::canvasToScreen(const Vec2 &pos) const {
        return Vec2(
            pos.x * m_state.viewScale + m_state.viewPosition.x,
            pos.y * m_state.viewScale + m_state.viewPosition.y
        );
    }

    bool NodeEditor::isPointInRect(const ImVec2 &point, const ImVec2 &rectMin, const ImVec2 &rectMax) const {
        return point.x >= rectMin.x && point.x <= rectMax.x &&
               point.y >= rectMin.y && point.y <= rectMax.y;
    }

    bool NodeEditor::isPointNearLine(const ImVec2 &point, const ImVec2 &lineStart, const ImVec2 &lineEnd,
                                     float threshold) const {
        float length2 = (lineEnd.x - lineStart.x) * (lineEnd.x - lineStart.x) +
                        (lineEnd.y - lineStart.y) * (lineEnd.y - lineStart.y);

        if (length2 <= 0.0001f) {
            float dx = point.x - lineStart.x;
            float dy = point.y - lineStart.y;
            return (dx * dx + dy * dy) <= (threshold * threshold);
        }

        float t = ((point.x - lineStart.x) * (lineEnd.x - lineStart.x) +
                   (point.y - lineStart.y) * (lineEnd.y - lineStart.y)) / length2;

        t = std::max(0.0f, std::min(1.0f, t));

        float projX = lineStart.x + t * (lineEnd.x - lineStart.x);
        float projY = lineStart.y + t * (lineEnd.y - lineStart.y);

        float dx = point.x - projX;
        float dy = point.y - projY;

        return (dx * dx + dy * dy) <= (threshold * threshold);
    }


}
