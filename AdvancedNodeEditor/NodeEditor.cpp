#include "NodeEditor.h"

namespace NodeEditorCore {
    std::vector<int> NodeEditor::getEvaluationOrder() const {
        return NodeEvaluator::getEvaluationOrder(const_cast<NodeEditor &>(*this));
    }


    std::vector<UUID> NodeEditor::getEvaluationOrderUUIDs() const {
        std::vector<UUID> result;
        std::vector<int> order = getEvaluationOrder();

        for (int nodeId: order) {
            result.push_back(getNodeUUID(nodeId));
        }

        return result;
    }

    void NodeEditor::setGraphTitle(const std::string &title) {
        m_titleManager.setTitle(title);
    }

    std::string NodeEditor::getGraphTitle() const {
        return m_titleManager.getTitle();
    }

    void NodeEditor::setGraphTitlePosition(NodeEditorCore::GraphTitleManager::TitlePosition position) {
        auto config = m_titleManager.getConfig();
        config.position = position;
        m_titleManager.setConfig(config);
    }

    void NodeEditor::setGraphTitleStyle(GraphTitleManager::TitleStyle style) {
        auto config = m_titleManager.getConfig();
        config.style = style;
        m_titleManager.setConfig(config);
    }

    void NodeEditor::setConnectionStyle(ConnectionStyleManager::ConnectionStyle style) {
        auto config = m_connectionStyleManager.getConfig();
        config.style = style;
        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::setGridColor(const Color &color) {
        m_state.style.uiColors.grid = color;
    }

    Color NodeEditor::getGridColor() const {
        return m_state.style.uiColors.grid;
    }

    void NodeEditor::setBackgroundColor(const Color &color) {
        m_state.style.uiColors.background = color;
    }

    Color NodeEditor::getBackgroundColor() const {
        return m_state.style.uiColors.background;
    }

    void NodeEditor::setSubgraphDepthColor(int depth, const Color &color) {
        m_depthColors[depth] = color;
    }

    void NodeEditor::zoomToFit(float padding) {
        Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        bool hasNodes = false;

        for (const auto &node: m_state.nodes) {
            if (!isNodeInCurrentSubgraph(node)) continue;

            min.x = std::min(min.x, node.position.x);
            min.y = std::min(min.y, node.position.y);
            max.x = std::max(max.x, node.position.x + node.size.x);
            max.y = std::max(max.y, node.position.y + node.size.y);

            hasNodes = true;
        }

        if (!hasNodes) return;

        m_viewManager.setBoundingBoxProvider([this](Vec2 &outMin, Vec2 &outMax) {
            outMin = Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            outMax = Vec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

            for (const auto &node: m_state.nodes) {
                if (!isNodeInCurrentSubgraph(node)) continue;

                outMin.x = std::min(outMin.x, node.position.x);
                outMin.y = std::min(outMin.y, node.position.y);
                outMax.x = std::max(outMax.x, node.position.x + node.size.x);
                outMax.y = std::max(outMax.y, node.position.y + node.size.y);
            }
        });

        m_viewManager.zoomToFit(padding);

        m_state.viewPosition = m_viewManager.getViewPosition();
        m_state.viewScale = m_viewManager.getViewScale();
    }

    void NodeEditor::zoomToFitSelected(float padding) {
        Vec2 min(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        Vec2 max(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

        bool hasSelectedNodes = false;

        for (const auto &node: m_state.nodes) {
            if (!node.selected || !isNodeInCurrentSubgraph(node)) continue;

            min.x = std::min(min.x, node.position.x);
            min.y = std::min(min.y, node.position.y);
            max.x = std::max(max.x, node.position.x + node.size.x);
            max.y = std::max(max.y, node.position.y + node.size.y);

            hasSelectedNodes = true;
        }

        if (!hasSelectedNodes) {
            zoomToFit(padding);
            return;
        }

        m_viewManager.setBoundingBoxProvider([this](Vec2 &outMin, Vec2 &outMax) {
            outMin = Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            outMax = Vec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

            for (const auto &node: m_state.nodes) {
                if (!node.selected || !isNodeInCurrentSubgraph(node)) continue;

                outMin.x = std::min(outMin.x, node.position.x);
                outMin.y = std::min(outMin.y, node.position.y);
                outMax.x = std::max(outMax.x, node.position.x + node.size.x);
                outMax.y = std::max(outMax.y, node.position.y + node.size.y);
            }
        });

        m_viewManager.zoomToFit(padding);

        m_state.viewPosition = m_viewManager.getViewPosition();
        m_state.viewScale = m_viewManager.getViewScale();
    }

    void NodeEditor::smoothCenterView(float duration) {
        m_viewManager.setBoundingBoxProvider([this](Vec2 &outMin, Vec2 &outMax) {
            outMin = Vec2(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
            outMax = Vec2(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());

            for (const auto &node: m_state.nodes) {
                if (!isNodeInCurrentSubgraph(node)) continue;

                outMin.x = std::min(outMin.x, node.position.x);
                outMin.y = std::min(outMin.y, node.position.y);
                outMax.x = std::max(outMax.x, node.position.x + node.size.x);
                outMax.y = std::max(outMax.y, node.position.y + node.size.y);
            }
        });

        ViewManager::ViewState currentState(m_state.viewPosition, m_state.viewScale);
        m_viewManager.setViewPosition(m_state.viewPosition);
        m_viewManager.setViewScale(m_state.viewScale);

        m_viewManager.centerView();
        ViewManager::ViewState targetState(m_viewManager.getViewPosition(), m_viewManager.getViewScale());

        m_viewManager.setViewPosition(currentState.position);
        m_viewManager.setViewScale(currentState.scale);
        m_viewManager.startViewTransition(targetState, duration, ViewManager::ViewTransitionType::EaseInOut);
    }

    void NodeEditor::smoothCenterOnNode(int nodeId, float duration) {
        const Node *node = getNode(nodeId);
        if (!node) return;

        ViewManager::ViewState currentState(m_state.viewPosition, m_state.viewScale);
        m_viewManager.setViewPosition(m_state.viewPosition);
        m_viewManager.setViewScale(m_state.viewScale);

        Vec2 center = Vec2(
            node->position.x + node->size.x * 0.5f,
            node->position.y + node->size.y * 0.5f
        );

        ImVec2 windowSize = ImGui::GetWindowSize();
        Vec2 targetPosition = Vec2(
            windowSize.x * 0.5f - center.x * m_state.viewScale,
            windowSize.y * 0.5f - center.y * m_state.viewScale
        );

        ViewManager::ViewState targetState(targetPosition, m_state.viewScale);

        m_viewManager.startViewTransition(targetState, duration, ViewManager::ViewTransitionType::EaseInOut);
    }

    void NodeEditor::setShowSubgraphBreadcrumbs(bool show) {
        auto config = m_breadcrumbManager.getConfig();
        config.showSubgraphPath = show;
        m_breadcrumbManager.setConfig(config);
    }

    void NodeEditor::setSubgraphBreadcrumbStyle(GraphTitleManager::TitleStyle style) {
        auto config = m_breadcrumbManager.getConfig();
        config.style = style;
        m_breadcrumbManager.setConfig(config);
    }

    void NodeEditor::smoothCenterOnNodeByUUID(const UUID &uuid, float duration) {
        int nodeId = getNodeId(uuid);
        if (nodeId != -1) {
            smoothCenterOnNode(nodeId, duration);
        }
    }

    void NodeEditor::setGraphTitlePosition(TitlePosition position) {
        NodeEditorCore::GraphTitleManager::TitlePosition corePosition;

        switch (position) {
            case TitlePosition::TopLeft:
                corePosition = NodeEditorCore::GraphTitleManager::TitlePosition::TopLeft;
                break;
            case TitlePosition::TopCenter:
                corePosition = NodeEditorCore::GraphTitleManager::TitlePosition::TopCenter;
                break;
            case TitlePosition::TopRight:
                corePosition = NodeEditorCore::GraphTitleManager::TitlePosition::TopRight;
                break;
            case TitlePosition::BottomLeft:
                corePosition = NodeEditorCore::GraphTitleManager::TitlePosition::BottomLeft;
                break;
            case TitlePosition::BottomCenter:
                corePosition = NodeEditorCore::GraphTitleManager::TitlePosition::BottomCenter;
                break;
            case TitlePosition::BottomRight:
                corePosition = NodeEditorCore::GraphTitleManager::TitlePosition::BottomRight;
                break;
            case TitlePosition::Center:
                corePosition = NodeEditorCore::GraphTitleManager::TitlePosition::Center;
                break;
            case TitlePosition::Custom:
                corePosition = NodeEditorCore::GraphTitleManager::TitlePosition::Custom;
                break;
        }

        setGraphTitlePosition(corePosition);
    }

    void NodeEditor::setGraphTitleStyle(TitleStyle style) {
        NodeEditorCore::GraphTitleManager::TitleStyle coreStyle;

        switch (style) {
            case TitleStyle::Default:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Default;
                break;
            case TitleStyle::Minimal:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Minimal;
                break;
            case TitleStyle::Bordered:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Bordered;
                break;
            case TitleStyle::Filled:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Filled;
                break;
            case TitleStyle::FilledTransparent:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::FilledTransparent;
                break;
            case TitleStyle::Houdini:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Houdini;
                break;
            case TitleStyle::Unreal:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Unreal;
                break;
        }

        setGraphTitleStyle(coreStyle);
    }

    void NodeEditor::setGraphTitleColor(const Color &textColor, const Color &backgroundColor) {
        auto config = m_titleManager.getConfig();

        config.textColor = NodeEditorCore::Color(textColor.r, textColor.g, textColor.b, textColor.a);
        config.backgroundColor = NodeEditorCore::Color(backgroundColor.r, backgroundColor.g, backgroundColor.b,
                                                       backgroundColor.a);

        m_titleManager.setConfig(config);
    }

    void NodeEditor::setGraphTitleCustomPosition(const Vec2 &position) {
        auto config = m_titleManager.getConfig();

        config.customPosition = NodeEditorCore::Vec2(position.x, position.y);
        config.position = NodeEditorCore::GraphTitleManager::TitlePosition::Custom;

        m_titleManager.setConfig(config);
    }

    void NodeEditor::setConnectionStyle(ConnectionStyle style) {
        NodeEditorCore::ConnectionStyleManager::ConnectionStyle coreStyle;

        switch (style) {
            case ConnectionStyle::Bezier:
                coreStyle = NodeEditorCore::ConnectionStyleManager::ConnectionStyle::Bezier;
                break;
            case ConnectionStyle::StraightLine:
                coreStyle = NodeEditorCore::ConnectionStyleManager::ConnectionStyle::StraightLine;
                break;
            case ConnectionStyle::AngleLine:
                coreStyle = NodeEditorCore::ConnectionStyleManager::ConnectionStyle::AngleLine;
                break;
            case ConnectionStyle::MetroLine:
                coreStyle = NodeEditorCore::ConnectionStyleManager::ConnectionStyle::MetroLine;
                break;
        }

        setConnectionStyle(coreStyle);
    }

    NodeEditor::ConnectionStyle NodeEditor::getConnectionStyle() const {
        auto coreStyle = m_connectionStyleManager.getDefaultStyle();

        switch (coreStyle) {
            case NodeEditorCore::ConnectionStyleManager::ConnectionStyle::Bezier:
                return ConnectionStyle::Bezier;
            case NodeEditorCore::ConnectionStyleManager::ConnectionStyle::StraightLine:
                return ConnectionStyle::StraightLine;
            case NodeEditorCore::ConnectionStyleManager::ConnectionStyle::AngleLine:
                return ConnectionStyle::AngleLine;
            case NodeEditorCore::ConnectionStyleManager::ConnectionStyle::MetroLine:
                return ConnectionStyle::MetroLine;
            default:
                return ConnectionStyle::Bezier;
        }
    }

    void NodeEditor::setConnectionThickness(float thickness) {
        auto config = m_connectionStyleManager.getConfig();
        config.thickness = thickness;
        m_connectionStyleManager.setConfig(config);
    }

    float NodeEditor::getConnectionThickness() const {
        return m_connectionStyleManager.getConfig().thickness;
    }

    void NodeEditor::setConnectionColor(const Color &color) {
        auto config = m_connectionStyleManager.getConfig();

        config.startColor = NodeEditorCore::Color(color.r, color.g, color.b, color.a);
        config.endColor = NodeEditorCore::Color(color.r, color.g, color.b, color.a);
        config.useGradient = false;

        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::setConnectionGradient(const Color &startColor, const Color &endColor) {
        auto config = m_connectionStyleManager.getConfig();

        config.startColor = NodeEditorCore::Color(startColor.r, startColor.g, startColor.b, startColor.a);
        config.endColor = NodeEditorCore::Color(endColor.r, endColor.g, endColor.b, endColor.a);
        config.useGradient = true;

        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::setConnectionSelectedColor(const Color &color) {
        auto config = m_connectionStyleManager.getConfig();

        config.selectedColor = NodeEditorCore::Color(color.r, color.g, color.b, color.a);

        m_connectionStyleManager.setConfig(config);
    }

    void NodeEditor::setSubgraphBreadcrumbStyle(TitleStyle style) {
        NodeEditorCore::GraphTitleManager::TitleStyle coreStyle;

        switch (style) {
            case TitleStyle::Default:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Default;
                break;
            case TitleStyle::Minimal:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Minimal;
                break;
            case TitleStyle::Bordered:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Bordered;
                break;
            case TitleStyle::Filled:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Filled;
                break;
            case TitleStyle::FilledTransparent:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::FilledTransparent;
                break;
            case TitleStyle::Houdini:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Houdini;
                break;
            case TitleStyle::Unreal:
                coreStyle = NodeEditorCore::GraphTitleManager::TitleStyle::Unreal;
                break;
        }

        auto config = m_breadcrumbManager.getConfig();
        config.style = coreStyle;
        m_breadcrumbManager.setConfig(config);
    }

    void NodeEditor::enableMinimap(bool enable) {
        m_minimapManager.getConfig().interactable = enable;
        m_minimapManager.setNodePositionProvider([this]() {
            std::vector<std::pair<Vec2, Vec2> > nodes;
            for (const auto &node: m_state.nodes) {
                if (isNodeInCurrentSubgraph(node)) {
                    nodes.push_back(std::make_pair(node.position, node.size));
                }
            }
            return nodes;
        });

        m_minimapManager.setViewportChangeCallback([this](const Vec2 &newViewPos) {
            m_state.viewPosition = newViewPos;
            m_viewManager.setViewPosition(newViewPos);
        });

        updateMinimapBounds();
    }

    bool NodeEditor::isMinimapEnabled() const {
        return m_minimapManager.getConfig().interactable;
    }

    void NodeEditor::setMinimapPosition(const Vec2 &position) {
        auto config = m_minimapManager.getConfig();
        config.position = position;
        m_minimapManager.setConfig(config);
    }

    void NodeEditor::setMinimapSize(const Vec2 &size) {
        auto config = m_minimapManager.getConfig();
        config.size = size;
        m_minimapManager.setConfig(config);
    }
}
