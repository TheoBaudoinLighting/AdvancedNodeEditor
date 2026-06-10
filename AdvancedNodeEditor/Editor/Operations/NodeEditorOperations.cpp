#include "../../Core/NodeEditor.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <map>
#include <queue>
#include <unordered_map>
#include <unordered_set>

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
            for (auto &node: m_state.nodes) {
                node.selected = false;
            }
        }

        for (auto &node: m_state.nodes) {
            if (node.id == nodeId) {
                node.selected = true;
                break;
            }
        }
    }

    void NodeEditor::deselectNode(int nodeId) {
        for (auto &node: m_state.nodes) {
            if (node.id == nodeId) {
                node.selected = false;
                break;
            }
        }
    }

    void NodeEditor::selectAllNodes() {
        for (auto &node: m_state.nodes) {
            if ((m_state.currentSubgraphId >= 0 && node.subgraphId == m_state.currentSubgraphId) ||
                (m_state.currentSubgraphId == -1 && node.subgraphId == -1)) {
                node.selected = true;
            }
        }
    }

    void NodeEditor::deselectAllNodes() {
        for (auto &node: m_state.nodes) {
            node.selected = false;
        }
    }

    void NodeEditor::alignSelectedNodesLeft() {
        float left = FLT_MAX;
        std::vector<Node *> targets;

        for (auto &node: m_state.nodes) {
            if (!node.selected || !isNodeInCurrentSubgraph(node)) continue;
            left = std::min(left, node.position.x);
            targets.push_back(&node);
        }

        if (targets.empty()) {
            for (auto &node: m_state.nodes) {
                if (!isNodeInCurrentSubgraph(node)) continue;
                left = std::min(left, node.position.x);
                targets.push_back(&node);
            }
        }

        if (targets.size() < 2) return;

        for (Node *node: targets) {
            node->position.x = left;
        }
        requestZoomToFitOnNextRender(80.0f);
    }

    void NodeEditor::alignSelectedNodesRight() {
        float right = -FLT_MAX;
        std::vector<Node *> targets;

        for (auto &node: m_state.nodes) {
            if (!node.selected || !isNodeInCurrentSubgraph(node)) continue;
            right = std::max(right, node.position.x + node.size.x);
            targets.push_back(&node);
        }

        if (targets.empty()) {
            for (auto &node: m_state.nodes) {
                if (!isNodeInCurrentSubgraph(node)) continue;
                right = std::max(right, node.position.x + node.size.x);
                targets.push_back(&node);
            }
        }

        if (targets.size() < 2) return;

        for (Node *node: targets) {
            node->position.x = right - node->size.x;
        }
        requestZoomToFitOnNextRender(80.0f);
    }

    void NodeEditor::autoLayoutSelectedNodesTopToBottom() {
        std::vector<Node *> targets;
        for (auto &node: m_state.nodes) {
            if (node.selected && isNodeInCurrentSubgraph(node)) {
                targets.push_back(&node);
            }
        }

        if (targets.empty()) {
            for (auto &node: m_state.nodes) {
                if (isNodeInCurrentSubgraph(node)) {
                    targets.push_back(&node);
                }
            }
        }

        if (targets.empty()) return;

        std::sort(targets.begin(), targets.end(), [](const Node *lhs, const Node *rhs) {
            if (std::abs(lhs->position.y - rhs->position.y) > 0.01f) return lhs->position.y < rhs->position.y;
            if (std::abs(lhs->position.x - rhs->position.x) > 0.01f) return lhs->position.x < rhs->position.x;
            return lhs->id < rhs->id;
        });

        float minX = FLT_MAX;
        float minY = FLT_MAX;
        float maxX = -FLT_MAX;
        float maxY = -FLT_MAX;
        float maxNodeWidth = 0.0f;
        float maxNodeHeight = 0.0f;
        std::unordered_map<int, Node *> nodeById;
        std::unordered_set<int> targetIds;

        for (Node *node: targets) {
            minX = std::min(minX, node->position.x);
            minY = std::min(minY, node->position.y);
            maxX = std::max(maxX, node->position.x + node->size.x);
            maxY = std::max(maxY, node->position.y + node->size.y);
            maxNodeWidth = std::max(maxNodeWidth, node->size.x);
            maxNodeHeight = std::max(maxNodeHeight, node->size.y);
            nodeById[node->id] = node;
            targetIds.insert(node->id);
        }

        std::unordered_map<int, std::vector<int>> outgoing;
        std::unordered_map<int, int> indegree;
        std::unordered_map<int, int> depth;
        for (Node *node: targets) {
            indegree[node->id] = 0;
            depth[node->id] = 0;
        }

        size_t internalConnectionCount = 0;
        for (const Connection &connection: m_state.connections) {
            if (!targetIds.count(connection.startNodeId) || !targetIds.count(connection.endNodeId)) continue;
            outgoing[connection.startNodeId].push_back(connection.endNodeId);
            ++indegree[connection.endNodeId];
            ++internalConnectionCount;
        }

        std::map<int, std::vector<Node *>> layers;
        if (internalConnectionCount == 0) {
            for (size_t i = 0; i < targets.size(); ++i) {
                layers[static_cast<int>(i)].push_back(targets[i]);
            }
        } else {
            auto nodeOrderLess = [&nodeById](int lhs, int rhs) {
                const Node *left = nodeById.at(lhs);
                const Node *right = nodeById.at(rhs);
                if (std::abs(left->position.y - right->position.y) > 0.01f) return left->position.y < right->position.y;
                if (std::abs(left->position.x - right->position.x) > 0.01f) return left->position.x < right->position.x;
                return lhs < rhs;
            };

            std::vector<int> ready;
            for (Node *node: targets) {
                if (indegree[node->id] == 0) ready.push_back(node->id);
            }
            std::sort(ready.begin(), ready.end(), nodeOrderLess);

            std::unordered_set<int> visited;
            while (!ready.empty()) {
                const int nodeId = ready.front();
                ready.erase(ready.begin());
                if (!visited.insert(nodeId).second) continue;

                for (int nextId: outgoing[nodeId]) {
                    depth[nextId] = std::max(depth[nextId], depth[nodeId] + 1);
                    --indegree[nextId];
                    if (indegree[nextId] == 0) {
                        ready.push_back(nextId);
                        std::sort(ready.begin(), ready.end(), nodeOrderLess);
                    }
                }
            }

            int fallbackDepth = 0;
            for (const auto &[_, value]: depth) {
                fallbackDepth = std::max(fallbackDepth, value + 1);
            }
            for (Node *node: targets) {
                if (!visited.count(node->id)) {
                    depth[node->id] = fallbackDepth++;
                }
                layers[depth[node->id]].push_back(node);
            }

            for (auto &[_, layerNodes]: layers) {
                std::sort(layerNodes.begin(), layerNodes.end(), [](const Node *lhs, const Node *rhs) {
                    if (std::abs(lhs->position.x - rhs->position.x) > 0.01f) return lhs->position.x < rhs->position.x;
                    if (std::abs(lhs->position.y - rhs->position.y) > 0.01f) return lhs->position.y < rhs->position.y;
                    return lhs->id < rhs->id;
                });
            }
        }

        const Vec2 originalCenter((minX + maxX) * 0.5f, (minY + maxY) * 0.5f);
        const float horizontalSpacing = std::max(maxNodeWidth + 140.0f, 280.0f);
        const float verticalSpacing = std::max(maxNodeHeight + 120.0f, 170.0f);
        const float totalHeight = static_cast<float>(layers.size() > 1 ? layers.size() - 1 : 0) * verticalSpacing;
        const float startY = originalCenter.y - totalHeight * 0.5f;

        int layerIndex = 0;
        for (auto &[_, layerNodes]: layers) {
            const float layerWidth = static_cast<float>(layerNodes.size() > 1 ? layerNodes.size() - 1 : 0) * horizontalSpacing;
            const float startX = originalCenter.x - layerWidth * 0.5f;

            for (size_t i = 0; i < layerNodes.size(); ++i) {
                layerNodes[i]->position = Vec2(
                    startX + static_cast<float>(i) * horizontalSpacing,
                    startY + static_cast<float>(layerIndex) * verticalSpacing
                );
            }
            ++layerIndex;
        }

        zoomToFit(100.0f);
        requestZoomToFitOnNextRender(100.0f);
    }

    void NodeEditor::toggleNodeSnapEnabled() {
        m_nodeSnapEnabled = !m_nodeSnapEnabled;
    }

    std::vector<int> NodeEditor::getSelectedNodes() const {
        std::vector<int> selectedNodes;
        for (const auto &node: m_state.nodes) {
            if (node.selected) {
                selectedNodes.push_back(node.id);
            }
        }
        return selectedNodes;
    }

    void NodeEditor::applyViewTransform(const Vec2 &pos, float scale) {
        m_viewManager.setViewPosition(pos);
        m_viewManager.setViewScale(scale);
        m_state.viewPosition = m_viewManager.getViewPosition();
        m_state.viewScale = m_viewManager.getViewScale();
    }

    void NodeEditor::setViewPosition(const Vec2 &position) {
        applyViewTransform(position, m_state.viewScale);
    }

    Vec2 NodeEditor::getViewPosition() const {
        return m_state.viewPosition;
    }

    void NodeEditor::setViewScale(float scale) {
        applyViewTransform(m_state.viewPosition, std::max(m_viewManager.getMinZoom(), std::min(scale, m_viewManager.getMaxZoom())));
    }

    float NodeEditor::getViewScale() const {
        return m_state.viewScale;
    }

    void NodeEditor::zoomToFitWithSize(float windowWidth, float windowHeight, float padding) {
        Vec2 min(FLT_MAX, FLT_MAX);
        Vec2 max(-FLT_MAX, -FLT_MAX);
        bool hasNodes = false;

        for (const auto &node: m_state.nodes) {
            if (!isNodeInCurrentSubgraph(node)) continue;

            min.x = std::min(min.x, node.position.x);
            min.y = std::min(min.y, node.position.y);
            max.x = std::max(max.x, node.position.x + node.size.x);
            max.y = std::max(max.y, node.position.y + node.size.y);
            hasNodes = true;
        }

        if (!hasNodes) {
            applyViewTransform(Vec2(0.0f, 0.0f), 1.0f);
            return;
        }

        const float effectiveWidth = std::max(windowWidth, 1.0f);
        const float effectiveHeight = std::max(windowHeight, 1.0f);
        const float contentWidth = std::max(max.x - min.x + padding * 2.0f, 1.0f);
        const float contentHeight = std::max(max.y - min.y + padding * 2.0f, 1.0f);

        float scale = std::min(effectiveWidth / contentWidth, effectiveHeight / contentHeight);
        scale = std::max(m_viewManager.getMinZoom(), std::min(scale, m_viewManager.getMaxZoom()));

        const Vec2 center((min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f);
        const Vec2 viewCenter(effectiveWidth * 0.5f, effectiveHeight * 0.5f);
        const Vec2 viewPosition(
            viewCenter.x - center.x * scale,
            viewCenter.y - center.y * scale
        );

        applyViewTransform(viewPosition, scale);
    }

    void NodeEditor::centerViewWithSize(float windowWidth, float windowHeight) {
        if (m_state.nodes.empty()) {
            applyViewTransform(Vec2(0, 0), m_state.viewScale);
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
            applyViewTransform(Vec2(0, 0), m_state.viewScale);
            return;
        }

        Vec2 center = Vec2(
            (min.x + max.x) * 0.5f,
            (min.y + max.y) * 0.5f
        );

        applyViewTransform(Vec2(
            windowWidth * 0.5f - center.x * m_state.viewScale,
            windowHeight * 0.5f - center.y * m_state.viewScale
        ), m_state.viewScale);
    }


    void NodeEditor::centerOnNodeWithSize(int nodeId, float windowWidth, float windowHeight) {
        const Node *node = getNode(nodeId);
        if (!node) return;

        Vec2 center = Vec2(
            node->position.x + node->size.x * 0.5f,
            node->position.y + node->size.y * 0.5f
        );

        applyViewTransform(Vec2(
            windowWidth * 0.5f - center.x * m_state.viewScale,
            windowHeight * 0.5f - center.y * m_state.viewScale
        ), m_state.viewScale);
    }


    void NodeEditor::setStyle(const NodeEditorStyle &style) {
        m_state.style = style;
        m_state.magnetThreshold = 20.0f * (style.pinRadius / 3.3f);
    }

    ListenerHandle NodeEditor::addNodeCreatedListener(NodeCallback callback) {
        return m_state.nodeCreatedListeners.add(std::move(callback));
    }

    ListenerHandle NodeEditor::addNodeRemovedListener(NodeCallback callback) {
        return m_state.nodeRemovedListeners.add(std::move(callback));
    }

    ListenerHandle NodeEditor::addConnectionCreatedListener(ConnectionCallback callback) {
        return m_state.connectionCreatedListeners.add(std::move(callback));
    }

    ListenerHandle NodeEditor::addConnectionRemovedListener(ConnectionCallback callback) {
        return m_state.connectionRemovedListeners.add(std::move(callback));
    }

    void NodeEditor::removeNodeCreatedListener(ListenerHandle handle) {
        m_state.nodeCreatedListeners.remove(handle);
    }

    void NodeEditor::removeNodeRemovedListener(ListenerHandle handle) {
        m_state.nodeRemovedListeners.remove(handle);
    }

    void NodeEditor::removeConnectionCreatedListener(ListenerHandle handle) {
        m_state.connectionCreatedListeners.remove(handle);
    }

    void NodeEditor::removeConnectionRemovedListener(ListenerHandle handle) {
        m_state.connectionRemovedListeners.remove(handle);
    }

    void NodeEditor::setCanConnectCallback(CanConnectCallback callback) {
        m_state.canConnectCallback = callback;
    }

    void NodeEditor::setCallbacksSuppressed(bool suppressed) {
        m_callbacksSuppressed = suppressed;
    }

    bool NodeEditor::areCallbacksSuppressed() const {
        return m_callbacksSuppressed;
    }

    Vec2 NodeEditor::screenToCanvas(const Vec2 &pos) const {
        return Vec2(
            (pos.x - m_state.canvasPos.x - m_state.viewPosition.x) / m_state.viewScale,
            (pos.y - m_state.canvasPos.y - m_state.viewPosition.y) / m_state.viewScale
        );
    }

    Vec2 NodeEditor::canvasToScreen(const Vec2 &pos) const {
        return Vec2(
            pos.x * m_state.viewScale + m_state.viewPosition.x + m_state.canvasPos.x,
            pos.y * m_state.viewScale + m_state.viewPosition.y + m_state.canvasPos.y
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
