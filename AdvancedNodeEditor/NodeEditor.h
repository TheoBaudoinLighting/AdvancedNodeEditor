#ifndef NODE_EDITOR_H
#define NODE_EDITOR_H

#include "Core/Style/StyleDefinitions.h"
#include "Core/Types/CoreTypes.h"
#include "Editor/View/NodeBoundingBoxManager.h"
#include <functional>
#include <stack>
#include <vector>
#include <map>
#include <memory>
#include <any>
#include <string>

#include "Core/Style/ConnectionStyleManager.h"
#include "Editor/View/GraphTitleManager.h"
#include "Editor/View/MinimapManager.h"
#include "Editor/View/ViewManager.h"
#include "Evaluation/NodeEditorEvaluation.h"
#include "Rendering/NodeEditorAnimationManager.h"

namespace NodeEditorCore {
    enum class InteractionMode;

    struct NodeTypeInfo {
        std::string name;
        std::string category;
        std::string description;
        std::function<Node*(const Vec2&)> builder;
    };

    using NodeCallback = std::function<void(int nodeId, const UUID& nodeUuid)>;
    using ConnectionCallback = std::function<void(int connectionId, const UUID& connectionUuid)>;
    using CanConnectCallback = std::function<bool(const Pin& startPin, const Pin& endPin)>;
    using NodeEvaluateCallback = std::function<void(Node&, const std::unordered_map<std::string, std::any>&)>;
    using NodeEvaluateCallbackUUID = std::function<void(const UUID&, Node&, const std::unordered_map<std::string, std::any>&)>;
    using ActionCallback = std::function<void(const std::string&, const std::unordered_map<std::string, std::any>&)>;
    using NodeOverlayCallback = std::function<void(ImDrawList*, const Node&, const ImVec2&)>;
    using NodeOverlayCallbackUUID = std::function<void(ImDrawList*, const UUID&, const Node&, const ImVec2&)>;

    class NodeEvaluationContext {
    public:
        void setValue(const std::string& key, const std::any& value) {
            m_values[key] = value;
        }

        template<typename T>
        T getValue(const std::string& key, const T& defaultValue = T()) const {
            auto it = m_values.find(key);
            if (it != m_values.end()) {
                try {
                    return std::any_cast<T>(it->second);
                } catch (const std::bad_any_cast&) {
                }
            }
            return defaultValue;
        }

        std::unordered_map<std::string, std::any> getValues() const {
            return m_values;
        }

    private:
        std::unordered_map<std::string, std::any> m_values;
    };

    class NodeEditor {
    public:

        enum class ArrangementType {
            Grid,
            Horizontal,
            Vertical,
            Circle
        };

        NodeEditor();
        ~NodeEditor();

        void beginFrame();
        void render();
        void endFrame();

        int addNode(const std::string& name, const std::string& type, const Vec2& pos, const UUID& uuid = "");
        UUID addNodeWithUUID(const std::string& name, const std::string& type, const Vec2& position);
        void removeNode(int nodeId);
        void removeNodeByUUID(const UUID& uuid);

        Node* getNode(int nodeId);
        Node* getNodeByUUID(const UUID& uuid);
        const Node* getNode(int nodeId) const;
        const Node* getNodeByUUID(const UUID& uuid) const;
        const std::vector<Node>& getNodes() const;

        UUID getNodeUUID(int nodeId) const;
        int getNodeId(const UUID& uuid) const;

        void updateNodeBoundingBoxes();
        void enableNodeAvoidance(bool enable);
        bool isNodeAvoidanceEnabled() const;

        int addPin(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Blue,
                  PinShape shape = PinShape::Circle, const UUID& uuid = "");
        int addPinByNodeUUID(const UUID& nodeUuid, const std::string& name, bool isInput,
                            PinType type = PinType::Blue, PinShape shape = PinShape::Circle, const UUID& uuid = "");
        UUID addPinWithUUID(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Blue,
                          PinShape shape = PinShape::Circle);
        UUID addPinWithUUIDByNodeUUID(const UUID& nodeUuid, const std::string& name, bool isInput,
                                     PinType type = PinType::Blue, PinShape shape = PinShape::Circle);

        void removePin(int nodeId, int pinId);
        void removePinByUUID(const UUID& nodeUuid, const UUID& pinUuid);

        Pin* getPin(int nodeId, int pinId);
        Pin* getPinByUUID(const UUID& nodeUuid, const UUID& pinUuid);
        const Pin* getPin(int nodeId, int pinId) const;
        const Pin* getPinByUUID(const UUID& nodeUuid, const UUID& pinUuid) const;
        UUID getPinUUID(int nodeId, int pinId) const;

        int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId, const UUID& uuid = "");
        int addConnectionByUUID(const UUID& startNodeUuid, const UUID& startPinUuid,
                              const UUID& endNodeUuid, const UUID& endPinUuid, const UUID& uuid = "");
        UUID addConnectionWithUUID(int startNodeId, int startPinId, int endNodeId, int endPinId);
        UUID addConnectionWithUUIDByUUID(const UUID& startNodeUuid, const UUID& startPinUuid,
                                      const UUID& endNodeUuid, const UUID& endPinUuid);

        void removeConnection(int connectionId);
        void removeConnectionByUUID(const UUID& uuid);

        Connection* getConnection(int connectionId);
        Connection* getConnectionByUUID(const UUID& uuid);
        const Connection* getConnection(int connectionId) const;
        const Connection* getConnectionByUUID(const UUID& uuid) const;
        const std::vector<Connection>& getConnections() const;

        UUID getConnectionUUID(int connectionId) const;
        int getConnectionId(const UUID& uuid) const;

        bool isConnected(int nodeId, int pinId) const;
        bool isConnectedByUUID(const UUID& nodeUuid, const UUID& pinUuid) const;

        int addGroup(const std::string& name, const Vec2& pos, const Vec2& size, const UUID& uuid = "");
        UUID addGroupWithUUID(const std::string& name, const Vec2& position, const Vec2& size);
        void removeGroup(int groupId);
        void removeGroupByUUID(const UUID& uuid);

        Group* getGroup(int groupId);
        Group* getGroupByUUID(const UUID& uuid);
        const Group* getGroup(int groupId) const;

        UUID getGroupUUID(int groupId) const;
        int getGroupId(const UUID& uuid) const;

        void addNodeToGroup(int nodeId, int groupId);
        void addNodeToGroupByUUID(const UUID& nodeUuid, const UUID& groupUuid);
        void removeNodeFromGroup(int nodeId, int groupId);

        void selectNode(int nodeId, bool append = false);
        void selectNodeByUUID(const UUID& uuid, bool append = false);
        void deselectNode(int nodeId);
        void selectAllNodes();
        void deselectAllNodes();
        std::vector<int> getSelectedNodes() const;
        std::vector<UUID> getSelectedNodeUUIDs() const;

        void setViewPosition(const Vec2& position);
        Vec2 getViewPosition() const;

        void setViewScale(float scale);
        float getViewScale() const;

        void centerView();
        void centerOnNode(int nodeId);
        void centerOnNodeByUUID(const UUID& uuid);
        void centerViewWithSize(float windowWidth, float windowHeight);
        void centerOnNodeWithSize(int nodeId, float windowWidth, float windowHeight);

        void setStyle(const NodeEditorStyle& style);

        void setNodeCreatedCallback(NodeCallback callback);
        void setNodeRemovedCallback(NodeCallback callback);
        void setConnectionCreatedCallback(ConnectionCallback callback);
        void setConnectionRemovedCallback(ConnectionCallback callback);
        void setCanConnectCallback(CanConnectCallback callback);

        Vec2 screenToCanvas(const Vec2& pos) const;
        Vec2 canvasToScreen(const Vec2& pos) const;

        int getHoveredNodeId() const;
        UUID getHoveredNodeUUID() const;
        int getHoveredPinId() const;
        UUID getHoveredPinUUID() const;
        int getHoveredConnectionId() const;
        UUID getHoveredConnectionUUID() const;
        int getHoveredGroupId() const;
        UUID getHoveredGroupUUID() const;

        void selectConnection(int connectionId, bool append = false);
        void selectConnectionByUUID(const UUID& uuid, bool append = false);
        void deselectConnection(int connectionId);
        void deselectConnectionByUUID(const UUID& uuid);
        void deselectAllConnections();

        void registerNodeType(const std::string& type, const std::string& category, const std::string& description,
                            std::function<Node*(const Vec2&)> builder);
        Node* createNodeOfType(const std::string& type, const Vec2& position);

        int createSubgraph(const std::string& name, const UUID& uuid = "");
        int createSubgraph(const std::string &name, const UUID &uuid = "", bool createDefaultNodes = true);
        UUID createSubgraphWithUUID(const std::string& name);

        Subgraph *getSubgraph(int subgraphId);

        Subgraph* getSubgraphByUUID(const UUID& uuid);

        void removeSubgraph(int subgraphId);

        void debugSubgraph(int subgraphId);

        int addInputPinToSubgraph(int subgraphId, const std::string &name, PinType type);

        int addOutputPinToSubgraph(int subgraphId, const std::string &name, PinType type);

        void synchronizeSubgraphConnections(int subgraphId, int subgraphNodeId);
        void handleSubgraphConnections(int connectionId);

        UUID getSubgraphUUID(int subgraphId) const;
        int getSubgraphId(const UUID& uuid) const;

        void addNodeToSubgraph(int nodeId, int subgraphId);
        void removeNodeFromSubgraph(int nodeId, int subgraphId);

        void addConnectionToSubgraph(int connectionId, int subgraphId);

        bool isConnectionInSubgraph(int connectionId, int subgraphId) const;

        void removeConnectionFromSubgraph(int connectionId, int subgraphId);

        std::vector<int> getNodesInSubgraph(int subgraphId) const;
        std::vector<int> getConnectionsInSubgraph(int subgraphId) const;

        void saveSubgraphViewState(int subgraphId);
        void restoreSubgraphViewState(int subgraphId);

        void setCurrentSubgraphId(int subgraphId);
        void setCurrentSubgraphByUUID(const UUID& uuid);
        int getCurrentSubgraphId() const;
        UUID getCurrentSubgraphUUID() const;

        bool enterSubgraph(int subgraphId);
        bool enterSubgraphByUUID(const UUID& uuid);
        bool exitSubgraph();

        Node* createSubgraphNode(int subgraphId, const std::string& name, const Vec2& position, const UUID& uuid = "");

        bool isNodeInCurrentSubgraph(const Node& node) const;
        bool isSubgraphContainer(const Node& node) const;
        bool isNodeInSubgraph(const Node& node, int subgraphId) const;
        bool isNodeInSubgraphByUUID(const Node& node, const UUID& subgraphUuid) const;
        bool isSubgraphContainer(int nodeId) const;
        bool isSubgraphContainerByUUID(const UUID& uuid) const;
        int getSubgraphFromNode(int nodeId) const;
        UUID getSubgraphUUIDFromNode(const UUID& nodeUuid) const;
        int getNodeSubgraph(int nodeId) const;
        UUID getNodeSubgraphUUID(const UUID& nodeUuid) const;

        void setSubgraphIdForNode(int nodeId, int subgraphId);
        void setSubgraphUUIDForNode(int nodeId, const UUID& uuid);
        UUID getSubgraphUUIDForNode(int nodeId) const;
        int getSubgraphIdForNode(int nodeId) const;

        void setDebugMode(bool enable) { m_debugMode = enable; }
        bool isDebugMode() const { return m_debugMode; }

        std::vector<int> getEvaluationOrder() const;
        std::vector<UUID> getEvaluationOrderUUIDs() const;
        std::vector<NodeEvaluator::ConnectionInfo> getInputConnections(int nodeId);
        std::vector<NodeEvaluator::ConnectionInfo> getInputConnectionsByUUID(const UUID& nodeUuid);
        std::vector<NodeEvaluator::ConnectionInfo> getOutputConnections(int nodeId);
        std::vector<NodeEvaluator::ConnectionInfo> getOutputConnectionsByUUID(const UUID& nodeUuid);

        ViewManager& getViewManager() { return m_viewManager; }
        GraphTitleManager& getTitleManager() { return m_titleManager; }
        ConnectionStyleManager& getConnectionStyleManager() { return m_connectionStyleManager; }

        void setNodeExecuting(int nodeId, bool executing) {
            m_animationManager.setNodeExecuting(nodeId, executing);
        }

        void arrangeNodesWithAnimation(const std::vector<int>& nodeIds, const ArrangementType type);


        void setGraphTitle(const std::string& title);
        std::string getGraphTitle() const;

        enum class TitlePosition {
            TopLeft,
            TopCenter,
            TopRight,
            BottomLeft,
            BottomCenter,
            BottomRight,
            Center,
            Custom
        };

        enum class TitleStyle {
            Default,
            Minimal,
            Bordered,
            Filled,
            FilledTransparent,
            Houdini,
            Unreal
        };

        void setGraphTitlePosition(GraphTitleManager::TitlePosition position);
        void setGraphTitleStyle(GraphTitleManager::TitleStyle style);
        void setGraphTitlePosition(TitlePosition position);
        void setGraphTitleStyle(TitleStyle style);
        void setGraphTitleColor(const Color& textColor, const Color& backgroundColor = Color(0.2f, 0.2f, 0.2f, 0.7f));
        void setGraphTitleCustomPosition(const Vec2& position);

        enum class ConnectionStyle {
            Bezier,
            StraightLine,
            AngleLine,
            MetroLine
        };

        void setConnectionStyle(ConnectionStyleManager::ConnectionStyle style);
        void setConnectionStyle(ConnectionStyle style);
        ConnectionStyle getConnectionStyle() const;
        void setConnectionThickness(float thickness);
        float getConnectionThickness() const;
        void setConnectionColor(const Color& color);
        void setConnectionGradient(const Color& startColor, const Color& endColor);
        void setConnectionSelectedColor(const Color& color);

        void setGridColor(const Color& color);
        Color getGridColor() const;
        void setBackgroundColor(const Color& color);
        Color getBackgroundColor() const;
        void setSubgraphDepthColor(int depth, const Color& color);

        void zoomToFit(float padding = 50.0f);
        void zoomToFitSelected(float padding = 50.0f);
        void smoothCenterView(float duration = 0.3f);
        void smoothCenterOnNode(int nodeId, float duration = 0.3f);
        void smoothCenterOnNodeByUUID(const UUID& uuid, float duration = 0.3f);


        bool isShowingSubgraphBreadcrumbs() const;
        void setShowSubgraphBreadcrumbs(bool show);
        void setSubgraphBreadcrumbStyle(GraphTitleManager::TitleStyle style);
        void setSubgraphBreadcrumbStyle(TitleStyle style);
        int getSubgraphDepth(int subgraphId) const;
        void drawSubgraphBreadcrumbs(ImDrawList* drawList, const ImVec2& canvasPos);

        void enableMinimap(bool enable);
        bool isMinimapEnabled() const;
        MinimapManager& getMinimapManager() { return m_minimapManager; }
        const MinimapManager& getMinimapManager() const { return m_minimapManager; }
        void setMinimapPosition(const Vec2& position);
        void setMinimapSize(const Vec2& size);

        void activateConnectionFlowTemporary(int connectionId, float duration);

        void activateConnectionFlowInfinite(int connectionId);

        void deactivateConnectionFlow(int connectionId);

        void activateAllConnectionFlows(bool temporary, float duration);

        void deactivateAllConnectionFlows();

        void toggleConnectionFlow(int connectionId, bool active, bool temporary, float duration);

        void updateMinimapBounds();

        std::vector<UUID> getAllNodeUUIDs() const;
        std::vector<UUID> getAllConnectionUUIDs() const;
        std::vector<UUID> getAllGroupUUIDs() const;

    private:
        struct State {
            std::vector<Node> nodes;
            UUIDMap<Node*> nodeUuidMap;
            std::vector<Connection> connections;
            UUIDMap<Connection*> connectionUuidMap;
            std::vector<Group> groups;
            UUIDMap<Group*> groupUuidMap;

            Vec2 viewPosition;
            float viewScale;

            int nextNodeId;
            int nextPinId;
            int nextConnectionId;
            int nextGroupId;

            int hoveredNodeId;
            UUID hoveredNodeUuid;
            int hoveredPinId;
            UUID hoveredPinUuid;
            int hoveredConnectionId;
            UUID hoveredConnectionUuid;
            int hoveredGroupId;
            UUID hoveredGroupUuid;

            int activeNodeId;
            UUID activeNodeUuid;
            int activeConnectionId;
            UUID activeConnectionUuid;
            int activeGroupId;
            UUID activeGroupUuid;

            bool connecting;
            int connectingNodeId;
            UUID connectingNodeUuid;
            int connectingPinId;
            UUID connectingPinUuid;

            bool dragging;
            Vec2 dragOffset;

            bool boxSelecting;
            Vec2 boxSelectStart;

            int magnetPinNodeId = -1;
            int magnetPinId = -1;
            UUID magnetPinNodeUuid = "";
            UUID magnetPinUuid = "";
            float magnetThreshold = 20.0f;
            bool canConnectToMagnetPin = true;

            NodeEditorStyle style;

            NodeCallback nodeCreatedCallback;
            NodeCallback nodeRemovedCallback;
            ConnectionCallback connectionCreatedCallback;
            ConnectionCallback connectionRemovedCallback;
            CanConnectCallback canConnectCallback;

            int currentSubgraphId;
            UUID currentSubgraphUuid;

            InteractionMode interactionMode;
            int contextMenuNodeId;
            UUID contextMenuNodeUuid;
            int contextMenuConnectionId;
            UUID contextMenuConnectionUuid;
            int contextMenuGroupId;
            UUID contextMenuGroupUuid;
            int contextMenuPinId;
            UUID contextMenuPinUuid;
            Vec2 dragStart;
            Vec2 groupStartSize;
            Vec2 contextMenuPos;

            std::unordered_map<int, Vec2> draggedNodePositions;

            State();
        };

        struct ConnectionInfo {
            int id;
            UUID uuid;
            int startNodeId;
            UUID startNodeUuid;
            int endNodeId;
            UUID endNodeUuid;
        };

        State m_state;
        bool m_debugMode;
        std::stack<int> m_subgraphStack;
        std::stack<UUID> m_subgraphUuidStack;
        std::map<int, std::shared_ptr<Subgraph>> m_subgraphs;
        UUIDMap<std::shared_ptr<Subgraph>> m_subgraphsByUuid;
        NodeEvaluateCallback m_nodeEvaluateCallback;
        NodeEvaluateCallbackUUID m_nodeEvaluateCallbackUUID;
        ActionCallback m_actionCallback;
        NodeOverlayCallback m_nodeOverlayCallback;
        NodeOverlayCallbackUUID m_nodeOverlayCallbackUUID;
        std::unordered_map<std::string, NodeTypeInfo> m_registeredNodeTypes;
        MinimapManager m_minimapManager;
        bool m_minimapEnabled;
        ViewManager m_viewManager;
        GraphTitleManager m_titleManager;
        GraphTitleManager m_breadcrumbManager;
        ConnectionStyleManager m_connectionStyleManager;
        std::unordered_map<int, Color> m_depthColors;
        std::shared_ptr<NodeBoundingBoxManager> m_nodeBoundingBoxManager;
        AnimationManager m_animationManager;
        bool m_nodeAvoidanceEnabled;

        void processInteraction();
        void processBoxSelection(const ImVec2& canvasPos);
        void processNodeDragging();
        void processConnectionCreation();
        void drawGrid(ImDrawList* drawList, const ImVec2& canvasPos);
        void drawConnections(ImDrawList* drawList, const ImVec2& canvasPos);
        void drawNodes(ImDrawList* drawList, const ImVec2& canvasPos);

        bool isNodeSelectableForDelete(int nodeId) const;

        void drawGroups(ImDrawList* drawList, const ImVec2& canvasPos);
        void drawBoxSelection(ImDrawList* drawList);
        void drawNodePins(ImDrawList* drawList, const Node& node, const ImVec2& nodePos, const ImVec2& nodeSize,
                        const ImVec2& canvasPos);
        void drawPinShape(ImDrawList* drawList, const ImVec2& center, float radius, PinShape shape, ImU32 fillColor,
                        ImU32 borderColor, float borderThickness = 1.0f, bool isHovered = false);
        void drawDragConnection(ImDrawList* drawList, const ImVec2& canvasPos);
        void drawDebugHitboxes(ImDrawList* drawList, const ImVec2& canvasPos);
        void drawContextMenu(ImDrawList* drawList);
        std::string pinTypeToString(PinType type) const;
        ImVec2 getPinPos(const Node& node, const Pin& pin, const ImVec2& canvasPos) const;
        bool isPinHovered(const Node& node, const Pin& pin, const ImVec2& canvasPos);
        bool isConnectionHovered(const Connection& connection, const ImVec2& canvasPos);
        bool doesConnectionExist(int startNodeId, int startPinId, int endNodeId, int endPinId) const;
        bool doesConnectionExistByUUID(const UUID& startNodeUuid, const UUID& startPinUuid,
                                     const UUID& endNodeUuid, const UUID& endPinUuid) const;
        bool canCreateConnection(const Pin& startPin, const Pin& endPin) const;
        void createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);
        void createConnectionByUUID(const UUID& startNodeUuid, const UUID& startPinUuid,
                                  const UUID& endNodeUuid, const UUID& endPinUuid);
        bool isPointInRect(const ImVec2& point, const ImVec2& rectMin, const ImVec2& rectMax) const;
        bool isPointNearLine(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd,
                           float threshold) const;
        bool isPointNearCubicBezier(const ImVec2& point, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3,
                                 const ImVec2& p4, float threshold) const;
        ImU32 ImLerpColor(ImU32 col_a, ImU32 col_b, float t);
        ImVec2 ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t);
        void startConnectionDrag(int nodeId, int pinId);
        void startConnectionDragByUUID(const UUID& nodeUuid, const UUID& pinUuid);
        void startNodeDrag(int nodeId, const ImVec2& mousePos);
        void startNodeDragByUUID(const UUID& nodeUuid, const ImVec2& mousePos);
        void startGroupInteraction(const ImVec2& mousePos);
        void startBoxSelect(const ImVec2& mousePos);
        void startPanCanvas();
        void endCurrentInteraction();
        void updateHoveredElements(const ImVec2& mousePos);
        void updateCurrentInteraction(const ImVec2& mousePos);
        void processGroupDragging();
        void processGroupResize();
        void processDeleteKeyPress();
        void processContextMenu();
        void processZoom(const ImVec2& mousePos);
        void duplicateNode(int nodeId);
        std::string getInteractionModeName() const;

        void updateNodeUuidMap();
        void updateConnectionUuidMap();
        void updateGroupUuidMap();
    };
}

#endif