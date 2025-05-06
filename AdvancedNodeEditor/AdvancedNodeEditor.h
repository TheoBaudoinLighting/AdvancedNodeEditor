#ifndef NODE_EDITOR_H
#define NODE_EDITOR_H

#include <vector>
#include <string>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <imgui.h>

namespace ANE {
    struct Vec2 {
        float x, y;
        Vec2() : x(0.0f), y(0.0f) {}
        Vec2(float x_, float y_) : x(x_), y(y_) {}
    };

    struct Color {
        float r, g, b, a;
        Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
        Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}
    };

    enum class PinType {
        Flow,
        Bool,
        Int,
        Float,
        String,
        Vec2,
        Vec3,
        Vec4,
        Color,
        Object,
        Custom
    };

    enum class PinShape {
        Circle,
        Square,
        Triangle,
        Diamond
    };

    enum class NodeLabelPosition {
        None,
        Left,
        Right
    };

    enum class GroupStyle {
        Default,
        Subtle,
        Bold,
        Wire,
        Ghost
    };

    class NodeEditor;
    struct Node;
    struct Pin;
}

namespace NodeEditorCore {

struct Vec2 {
    float x, y;
    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }

    ImVec2 toImVec2() const { return ImVec2(x, y); }
    static Vec2 fromImVec2(const ImVec2& v) { return Vec2(v.x, v.y); }
    
    static Vec2 fromANEVec2(const ANE::Vec2& v) { return Vec2(v.x, v.y); }
    ANE::Vec2 toANEVec2() const { return ANE::Vec2(x, y); }
};

struct Color {
    float r, g, b, a;
    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}

    ImVec4 toImVec4() const { return ImVec4(r, g, b, a); }
    ImU32 toImU32() const { return IM_COL32(r * 255, g * 255, b * 255, a * 255); }
    static Color fromImVec4(const ImVec4& v) { return Color(v.x, v.y, v.z, v.w); }
    
    static Color fromANEColor(const ANE::Color& c) { return Color(c.r, c.g, c.b, c.a); }
    ANE::Color toANEColor() const { return ANE::Color(r, g, b, a); }
};

enum class PinType {
    Flow,
    Bool,
    Int,
    Float,
    String,
    Vec2,
    Vec3,
    Vec4,
    Color,
    Object,
    Custom
};

static PinType fromANEPinType(ANE::PinType type) {
    return static_cast<PinType>(type);
}

static ANE::PinType toANEPinType(PinType type) {
    return static_cast<ANE::PinType>(type);
}

enum class PinShape {
    Circle,
    Square,
    Triangle,
    Diamond
};

static PinShape fromANEPinShape(ANE::PinShape shape) {
    return static_cast<PinShape>(shape);
}

static ANE::PinShape toANEPinShape(PinShape shape) {
    return static_cast<ANE::PinShape>(shape);
}

enum class GroupStyle {
    Default,
    Subtle,
    Bold,
    Wire,
    Ghost
};

static GroupStyle fromANEGroupStyle(ANE::GroupStyle style) {
    return static_cast<GroupStyle>(style);
}

static ANE::GroupStyle toANEGroupStyle(GroupStyle style) {
    return static_cast<ANE::GroupStyle>(style);
}

enum class NodeLabelPosition {
    None,
    Left,
    Right
};

static NodeLabelPosition fromANENodeLabelPosition(ANE::NodeLabelPosition pos) {
    return static_cast<NodeLabelPosition>(pos);
}

static ANE::NodeLabelPosition toANENodeLabelPosition(NodeLabelPosition pos) {
    return static_cast<ANE::NodeLabelPosition>(pos);
}

namespace internal {

struct NodeColors {
    Color baseColor;
    Color headerColor;
    Color accentColor;
    Color borderColor;
    Color selectedColor;
    Color hoveredColor;
    Color glowColor;
};

struct PinColors {
    Color color;
    Color hoverColor;
    Color connectedColor;
};

struct ConnectionColors {
    Color baseColor;
    Color selectedColor;
    Color hoveredColor;
    Color validColor;
    Color invalidColor;
};

struct UIColors {
    Color background;
    Color grid;
    Color selection;
    Color menuBar;
    Color tooltipBg;
    Color tooltipText;
};

struct ImUtils {
    static ImU32 ImLerpColor(ImU32 col_a, ImU32 col_b, float t);
    static ImVec2 ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t);
    static bool isPointInRect(const ImVec2& point, const ImVec2& rectMin, const ImVec2& rectMax);
    static bool isPointNearLine(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd, float threshold);
    static bool isPointNearCubicBezier(const ImVec2& point, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float threshold);
};

} // namespace internal

struct Pin {
    int id;
    std::string name;
    std::string label;
    bool isInput;
    PinType type;
    PinShape shape;
    Color color;
    bool connected;

    Pin(int id, const std::string& name, bool isInput, PinType type = PinType::Flow);
};

struct Node {
    int id;
    std::string name;
    std::string type;
    Vec2 position;
    Vec2 size;
    std::vector<Pin> inputs;
    std::vector<Pin> outputs;
    bool selected;
    bool disabled;
    int groupId;
    std::string iconSymbol;
    bool isTemplate;
    bool isCurrentFlag;
    NodeLabelPosition labelPosition;

    Node(int id, const std::string& name, const std::string& type, const Vec2& pos);

    Pin* findPin(int pinId);
    const Pin* findPin(int pinId) const;

    Vec2 getPosition() const;
    void setPosition(const Vec2& position);
    Vec2 getSize() const;
};

struct Connection {
    int id;
    int startNodeId;
    int startPinId;
    int endNodeId;
    int endPinId;
    bool selected;

    Connection(int id, int startNodeId, int startPinId, int endNodeId, int endPinId);
};

struct Group {
    int id;
    std::string name;
    Vec2 position;
    Vec2 size;
    std::unordered_set<int> nodes;
    bool collapsed;
    bool selected;
    Color color;
    GroupStyle style;

    Group(int id, const std::string& name, const Vec2& pos, const Vec2& size);
};

struct NodeEditorStyle {
    internal::UIColors uiColors;
    std::unordered_map<std::string, internal::NodeColors> nodeColors;
    std::unordered_map<std::string, internal::PinColors> pinColors;
    internal::ConnectionColors connectionColors;

    float gridSpacing;
    float nodeRounding;
    float pinRadius;
    float connectionThickness;

    NodeEditorStyle();
};

class NodeEditor {
public:
    using NodeCallback = std::function<void(int nodeId)>;
    using ConnectionCallback = std::function<void(int connectionId)>;
    using CanConnectCallback = std::function<bool(const Pin& startPin, const Pin& endPin)>;

    NodeEditor();
    ~NodeEditor();

    void beginFrame();
    void render();
    void endFrame();

    int addNode(const std::string& name, const std::string& type, const Vec2& pos);
    void removeNode(int nodeId);
    Node* getNode(int nodeId);
    const Node* getNode(int nodeId) const;
    const std::vector<Node>& getNodes() const;

    int addPin(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Flow, PinShape shape = PinShape::Circle);
    void removePin(int nodeId, int pinId);
    Pin* getPin(int nodeId, int pinId);
    const Pin* getPin(int nodeId, int pinId) const;

    int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);
    void removeConnection(int connectionId);
    Connection* getConnection(int connectionId);
    const Connection* getConnection(int connectionId) const;
    const std::vector<Connection>& getConnections() const;
    bool isConnected(int nodeId, int pinId) const;

    int addGroup(const std::string& name, const Vec2& pos, const Vec2& size);
    void removeGroup(int groupId);
    Group* getGroup(int groupId);
    const Group* getGroup(int groupId) const;
    void addNodeToGroup(int nodeId, int groupId);
    void removeNodeFromGroup(int nodeId, int groupId);

    void selectNode(int nodeId, bool append = false);
    void deselectNode(int nodeId);
    void selectAllNodes();
    void deselectAllNodes();
    std::vector<int> getSelectedNodes() const;

    void setViewPosition(const Vec2& position);
    Vec2 getViewPosition() const;
    void setViewScale(float scale);
    float getViewScale() const;
    void centerView();
    void centerOnNode(int nodeId);

    void setStyle(const NodeEditorStyle& style);
    const NodeEditorStyle& getStyle() const;

    void setNodeCreatedCallback(NodeCallback callback);
    void setNodeRemovedCallback(NodeCallback callback);
    void setConnectionCreatedCallback(ConnectionCallback callback);
    void setConnectionRemovedCallback(ConnectionCallback callback);
    void setCanConnectCallback(CanConnectCallback callback);

    Vec2 screenToCanvas(const Vec2& pos) const;
    Vec2 canvasToScreen(const Vec2& pos) const;

    struct ConnectionInfo {
        int id;
        int startNodeId;
        int startPinId;
        int endNodeId;
        int endPinId;
        bool selected;
    };
    
    std::vector<ConnectionInfo> getAllConnections() const;
    std::vector<ConnectionInfo> getNodeConnections(int nodeId) const;
    std::vector<ConnectionInfo> getNodeInputConnections(int nodeId) const;
    std::vector<ConnectionInfo> getNodeOutputConnections(int nodeId) const;
    std::vector<int> getSelectedConnections() const;

    int getHoveredNodeId() const;
    int getHoveredPinId() const;
    int getHoveredConnectionId() const;
    int getHoveredGroupId() const;
    
    void selectConnection(int connectionId, bool append = false);
    void deselectConnection(int connectionId);
    void deselectAllConnections();

private:
    using ImU32 = ImU32;
    using ImVec2 = ImVec2;
    using ImVec4 = ImVec4;
    using ImDrawList = ImDrawList;

    ImU32 ImLerpColor(ImU32 col_a, ImU32 col_b, float t);
    ImVec2 ImBezierCubicCalc(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float t);

    struct State {
        std::vector<Node> nodes;
        std::vector<Connection> connections;
        std::vector<Group> groups;

        Vec2 viewPosition;
        float viewScale;

        int nextNodeId;
        int nextPinId;
        int nextConnectionId;
        int nextGroupId;

        int hoveredNodeId;
        int hoveredPinId;
        int hoveredConnectionId;
        int hoveredGroupId;

        int activeNodeId;
        int activeConnectionId;
        int activeGroupId;

        bool connecting;
        int connectingNodeId;
        int connectingPinId;

        bool dragging;
        Vec2 dragOffset;

        bool boxSelecting;
        Vec2 boxSelectStart;

        NodeEditorStyle style;

        NodeCallback nodeCreatedCallback;
        NodeCallback nodeRemovedCallback;
        ConnectionCallback connectionCreatedCallback;
        ConnectionCallback connectionRemovedCallback;
        CanConnectCallback canConnectCallback;

        State();
    };

    State m_state;

    void processInteraction();
    void processBoxSelection(const ImVec2& canvasPos);
    void processNodeSelection();
    void processNodeDragging();
    void processConnectionCreation();

    void drawGrid(ImDrawList* drawList, const ImVec2& canvasPos);
    void drawConnections(ImDrawList* drawList, const ImVec2& canvasPos);
    void drawNodes(ImDrawList* drawList, const ImVec2& canvasPos);
    void drawGroups(ImDrawList* drawList, const ImVec2& canvasPos);
    void drawBoxSelection(ImDrawList* drawList);

    void drawNodePins(ImDrawList* drawList, const Node& node, const ImVec2& nodePos, const ImVec2& nodeSize, const ImVec2& canvasPos);
    void drawPinShape(ImDrawList* drawList, const ImVec2& center, float radius, PinShape shape, ImU32 fillColor, ImU32 borderColor, float borderThickness = 1.0f, bool isHovered = false);
    void drawDragConnection(ImDrawList* drawList, const ImVec2& canvasPos);
    std::string pinTypeToString(PinType type) const;

    ImVec2 getPinPos(const Node& node, const Pin& pin, const ImVec2& canvasPos) const;
    bool isPinHovered(const Node& node, const Pin& pin, const ImVec2& canvasPos);
    bool isConnectionHovered(const Connection& connection, const ImVec2& canvasPos);
    bool doesConnectionExist(int startNodeId, int startPinId, int endNodeId, int endPinId) const;
    bool canCreateConnection(const Pin& startPin, const Pin& endPin) const;
    void createConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);

    bool isPointInRect(const ImVec2& point, const ImVec2& rectMin, const ImVec2& rectMax) const;
    bool isPointNearLine(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd, float threshold) const;
    bool isPointNearCubicBezier(const ImVec2& point, const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, float threshold) const;
};

inline Vec2 convertToVec2(const ANE::Vec2& v) { return Vec2(v.x, v.y); }
inline Color convertToColor(const ANE::Color& c) { return Color(c.r, c.g, c.b, c.a); }
inline PinType convertToPinType(ANE::PinType t) { return static_cast<PinType>(t); }
inline PinShape convertToPinShape(ANE::PinShape s) { return static_cast<PinShape>(s); }
inline NodeLabelPosition convertToNodeLabelPosition(ANE::NodeLabelPosition p) { return static_cast<NodeLabelPosition>(p); }
inline GroupStyle convertToGroupStyle(ANE::GroupStyle s) { return static_cast<GroupStyle>(s); }

inline ANE::Vec2 convertToANEVec2(const Vec2& v) { return ANE::Vec2(v.x, v.y); }
inline ANE::Color convertToANEColor(const Color& c) { return ANE::Color(c.r, c.g, c.b, c.a); }
inline ANE::PinType convertToANEPinType(PinType t) { return static_cast<ANE::PinType>(t); }
inline ANE::PinShape convertToANEPinShape(PinShape s) { return static_cast<ANE::PinShape>(s); }
inline ANE::NodeLabelPosition convertToANENodeLabelPosition(NodeLabelPosition p) { return static_cast<ANE::NodeLabelPosition>(p); }
inline ANE::GroupStyle convertToANEGroupStyle(GroupStyle s) { return static_cast<ANE::GroupStyle>(s); }

} // namespace NodeEditorCore

namespace ANE {

struct NodeStyle {
    Color baseColor;
    Color headerColor;
    Color accentColor;
    Color borderColor;
    Color selectedColor;
    Color hoveredColor;
    Color glowColor;
};

struct PinStyle {
    Color color;
    Color hoverColor;
    Color connectedColor;
};

struct ConnectionStyle {
    Color baseColor;
    Color selectedColor;
    Color hoveredColor;
    Color validColor;
    Color invalidColor;
};

struct EditorStyle {
    Color backgroundColor;
    Color gridColor;
    Color selectionColor;
    
    std::unordered_map<std::string, NodeStyle> nodeStyles;
    std::unordered_map<std::string, PinStyle> pinStyles;
    ConnectionStyle connectionStyle;
    
    float gridSpacing;
    float nodeRounding;
    float pinRadius;
    float connectionThickness;

    EditorStyle();
};

struct Node {
    int id;
    std::string name;
    std::string type;
    std::string iconSymbol;
    NodeLabelPosition labelPosition;
    bool disabled;
    bool isTemplate;
    bool isCurrentFlag;
    
    Node(int id, const std::string& name, const std::string& type);
    void setIconSymbol(const std::string& symbol);
    void setLabelPosition(NodeLabelPosition position);
    void setDisabled(bool disabled);
    void setAsTemplate(bool isTemplate);
    void setCurrentFlag(bool isCurrentFlag);
    
    Vec2 getPosition() const;
    void setPosition(const Vec2& position);
    Vec2 getSize() const;
};

struct Pin {
    int id;
    std::string name;
    bool isInput;
    PinType type;
    PinShape shape;
    
    Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape);
};

struct Group {
    int id;
    std::string name;
    Color color;
    GroupStyle style;
    bool collapsed;
    
    Group(int id, const std::string& name);
    void setColor(const Color& color);
    void setStyle(GroupStyle style);
    void setCollapsed(bool collapsed);
};

class NodeEditor {
public:
    using CanConnectCallback = std::function<bool(const Pin& startPin, const Pin& endPin)>;
    
    NodeEditor();
    ~NodeEditor();
    
    void beginFrame();
    void render();
    void endFrame();
    
    int addNode(const std::string& name, const std::string& type, const Vec2& position);
    Node* getNode(int nodeId);
    void removeNode(int nodeId);
    
    int addPin(int nodeId, const std::string& name, bool isInput, PinType type = PinType::Flow, PinShape shape = PinShape::Circle);
    Pin* getPin(int nodeId, int pinId);
    
    int addConnection(int startNodeId, int startPinId, int endNodeId, int endPinId);
    void removeConnection(int connectionId);
    
    int addGroup(const std::string& name, const Vec2& position, const Vec2& size);
    Group* getGroup(int groupId);
    void addNodeToGroup(int nodeId, int groupId);
    
    void selectNode(int nodeId, bool append = false);
    std::vector<int> getSelectedNodes() const;
    
    int getHoveredNodeId() const;
    int getHoveredPinId() const;
    int getHoveredConnectionId() const;
    int getHoveredGroupId() const;
    
    void selectConnection(int connectionId, bool append = false);
    void deselectConnection(int connectionId);
    void deselectAllConnections();
    
    void centerView();
    void setViewScale(float scale);
    float getViewScale() const;
    
    void setStyle(const EditorStyle& style);
    EditorStyle getStyle() const;
    
    void setCanConnectCallback(CanConnectCallback callback);
    
private:
    NodeEditorCore::NodeEditor m_editor;
    
    NodeEditorCore::NodeEditorStyle convertToInternalStyle(const EditorStyle& style) const;
    EditorStyle convertToAPIStyle(const NodeEditorCore::NodeEditorStyle& style) const;
};

} // namespace ANE

#endif // NODE_EDITOR_H