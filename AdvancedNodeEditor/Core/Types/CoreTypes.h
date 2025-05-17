#ifndef NODE_EDITOR_TYPES_H
#define NODE_EDITOR_TYPES_H

#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <any>
#include <algorithm>
#include <memory>
#include <imgui.h>
#include "../../Utils/UuidGenerator.h"

namespace NodeEditorCore {

using UUID = std::string;

struct UUIDHash {
    std::size_t operator()(const UUID& uuid) const {
        return std::hash<std::string>{}(uuid);
    }
};

template<typename T>
using UUIDMap = std::unordered_map<UUID, T, UUIDHash>;

inline UUID generateUUID() {
    return UuidGenerator::getInstance().generateV4().toString();
}

inline int uuidToDisplayId(const UUID& uuid) {
    static std::unordered_map<UUID, int> displayIdCache;
    static int nextDisplayId = 1;

    auto it = displayIdCache.find(uuid);
    if (it != displayIdCache.end()) {
        return it->second;
    }

    int displayId = nextDisplayId++;
    displayIdCache[uuid] = displayId;
    return displayId;
}

struct Vec2 {
    float x, y;

    Vec2() : x(0.0f), y(0.0f) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2 operator/(float scalar) const { return Vec2(x / scalar, y / scalar); }

    ImVec2 toImVec2() const { return ImVec2(x, y); }
    static Vec2 fromImVec2(const ImVec2& v) { return Vec2(v.x, v.y); }
};

struct Color {
    float r, g, b, a;

    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) : r(r_), g(g_), b(b_), a(a_) {}

    ImVec4 toImVec4() const { return ImVec4(r, g, b, a); }
    ImU32 toImU32() const { return IM_COL32(r * 255, g * 255, b * 255, a * 255); }
    static Color fromImVec4(const ImVec4& v) { return Color(v.x, v.y, v.z, v.w); }
};

enum class PinType {
    Blue,
    Red,
    Green,
    Yellow,
    Purple,
    Cyan,
    Orange,
    White,
    Black,
    Gray,
    Custom
};

enum class PinShape {
    Circle,
    Square,
    Triangle,
    Diamond
};

enum class GroupStyle {
    Default,
    Subtle,
    Bold,
    Wire,
    Ghost
};

enum class NodeLabelPosition {
    None,
    Left,
    Right
};

struct Metadata {
    std::map<std::string, std::any> attributes;

    template<typename T>
    void setAttribute(const std::string& key, const T& value) {
        attributes[key] = value;
    }

    template<typename T>
    T getAttribute(const std::string& key, const T& defaultValue = T()) const {
        auto it = attributes.find(key);
        if (it != attributes.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
            }
        }
        return defaultValue;
    }

    bool hasAttribute(const std::string& key) const {
        return attributes.find(key) != attributes.end();
    }

    void removeAttribute(const std::string& key) {
        attributes.erase(key);
    }
};

enum class EventType {
    NodeCreated,
    NodeRemoved,
    NodeSelected,
    NodeDeselected,
    NodeMoved,
    NodeResized,
    NodeChanged,
    NodeDragged,
    NodeDropped,
    PinAdded,
    PinRemoved,
    PinConnected,
    PinDisconnected,
    ConnectionCreated,
    ConnectionRemoved,
    ConnectionSelected,
    ConnectionDeselected,
    GroupCreated,
    GroupRemoved,
    GroupSelected,
    GroupDeselected,
    ViewChanged,
    StateChanged,
    SubgraphCreated,
    SubgraphRemoved,
    SubgraphEntered,
    SubgraphExited,
    NodeAddedToSubgraph,
    NodeRemovedFromSubgraph,
    ConnectionAddedToSubgraph,
    ConnectionRemovedFromSubgraph,
    GroupAddedToSubgraph,
    GroupRemovedFromSubgraph,
    Custom
};

struct Event {
    EventType type;

    int nodeId = -1;
    int pinId = -1;
    int connectionId = -1;
    int groupId = -1;
    int subgraphId = -1;
    std::unordered_map<std::string, std::any> data;

    Event(EventType type) : type(type) {}

    template<typename T>
    void setData(const std::string& key, const T& value) {
        data[key] = value;
    }

    template<typename T>
    T getData(const std::string& key, const T& defaultValue = T()) const {
        auto it = data.find(key);
        if (it != data.end()) {
            try {
                return std::any_cast<T>(it->second);
            } catch (const std::bad_any_cast&) {
            }
        }
        return defaultValue;
    }
};

using EventCallback = std::function<void(const Event&)>;

namespace UI {
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

    struct Layer {
        int zOrder;
        bool visible;
        std::string name;
        std::function<void(ImDrawList*, const ImVec2&)> drawCallback;

        Layer() : zOrder(0), visible(true), name("") {}
        Layer(const std::string& name, int zOrder, std::function<void(ImDrawList*, const ImVec2&)> callback)
            : name(name), zOrder(zOrder), visible(true), drawCallback(callback) {}
    };
}

struct Pin {
    int id;
    UUID uuid;
    std::string name;
    std::string label;
    bool isInput;
    PinType type;
    PinShape shape;
    Color color;
    bool connected;
    Metadata metadata;

    Pin() : id(0), uuid(generateUUID()), name(""), label(""), isInput(false),
            type(PinType::Blue), shape(PinShape::Circle), connected(false) {}

    Pin(int id, const std::string& name, bool isInput, PinType type)
        : id(id), uuid(generateUUID()), name(name), label(name), isInput(isInput),
          type(type), shape(PinShape::Circle), connected(false) {
        setColorByType();
    }

    Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape)
        : id(id), uuid(generateUUID()), name(name), label(name), isInput(isInput),
          type(type), shape(shape), connected(false) {
        setColorByType();
    }

    Pin(const UUID& existingUuid, int id, const std::string& name, bool isInput, PinType type, PinShape shape)
        : id(id), uuid(existingUuid.empty() ? generateUUID() : existingUuid), name(name), label(name),
          isInput(isInput), type(type), shape(shape), connected(false) {
        setColorByType();
    }

    template<typename T>
    void setMetadata(const std::string& key, const T& value) {
        metadata.setAttribute(key, value);
    }

    template<typename T>
    T getMetadata(const std::string& key, const T& defaultValue = T()) const {
        return metadata.getAttribute<T>(key, defaultValue);
    }

private:
    void setColorByType() {
        switch (type) {
            case PinType::Blue:    color = Color(0.2f, 0.4f, 0.9f); break;
            case PinType::Red:     color = Color(0.9f, 0.3f, 0.3f); break;
            case PinType::Green:   color = Color(0.3f, 0.8f, 0.3f); break;
            case PinType::Yellow:  color = Color(0.95f, 0.95f, 0.3f); break;
            case PinType::Purple:  color = Color(0.8f, 0.3f, 0.8f); break;
            case PinType::Cyan:    color = Color(0.3f, 0.8f, 0.9f); break;
            case PinType::Orange:  color = Color(0.9f, 0.6f, 0.3f); break;
            case PinType::White:   color = Color(0.9f, 0.9f, 0.9f); break;
            case PinType::Black:   color = Color(0.2f, 0.2f, 0.2f); break;
            case PinType::Gray:    color = Color(0.7f, 0.7f, 0.7f); break;
            default:               color = Color(0.7f, 0.7f, 0.7f); break;
        }
    }
};

struct Node {
    int id;
    UUID uuid;
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
    bool isSubgraph;
    int subgraphId;
    UUID subgraphUuid;
    Metadata metadata;
    bool isProtected;

    Node() : id(-1), uuid(generateUUID()), position(0.0f, 0.0f), size(140.0f, 28.0f),
             selected(false), disabled(false), groupId(-1), isTemplate(false), isCurrentFlag(false),
             labelPosition(NodeLabelPosition::Right), isSubgraph(false), subgraphId(-1), isProtected(false) {}

    Node(int id, const std::string& name, const std::string& type, const Vec2& pos)
        : id(id), uuid(generateUUID()), name(name), type(type), position(pos), size(140.0f, 28.0f),
          selected(false), disabled(false), groupId(-1), isTemplate(false), isCurrentFlag(false),
          labelPosition(NodeLabelPosition::Right), isSubgraph(false), subgraphId(-1), isProtected(false) {}

    Node(const UUID& existingUuid, int id, const std::string& name, const std::string& type, const Vec2& pos)
        : id(id), uuid(existingUuid), name(name), type(type), position(pos), size(140.0f, 28.0f),
          selected(false), disabled(false), groupId(-1), isTemplate(false), isCurrentFlag(false),
          labelPosition(NodeLabelPosition::Right), isSubgraph(false), subgraphId(-1), isProtected(false) {}

    Pin* findPin(int pinId) {
        for (auto& pin : inputs) {
            if (pin.id == pinId) {
                return &pin;
            }
        }
        for (auto& pin : outputs) {
            if (pin.id == pinId) {
                return &pin;
            }
        }
        return nullptr;
    }

    Pin* findPinByUUID(const UUID& pinUuid) {
        for (auto& pin : inputs) {
            if (pin.uuid == pinUuid) return &pin;
        }
        for (auto& pin : outputs) {
            if (pin.uuid == pinUuid) return &pin;
        }
        return nullptr;
    }

    const Pin* findPin(int pinId) const {
        for (const auto& pin : inputs) {
            if (pin.id == pinId) return &pin;
        }
        for (const auto& pin : outputs) {
            if (pin.id == pinId) return &pin;
        }
        return nullptr;
    }

    const Pin* findPinByUUID(const UUID& pinUuid) const {
        for (const auto& pin : inputs) {
            if (pin.uuid == pinUuid) return &pin;
        }
        for (const auto& pin : outputs) {
            if (pin.uuid == pinUuid) return &pin;
        }
        return nullptr;
    }

    void setSubgraphId(int id) {
        metadata.setAttribute("subgraphId", id);
    }

    int getSubgraphId() const {
        return metadata.getAttribute<int>("subgraphId", -1);
    }

    void setIconSymbol(const std::string& symbol) {
        iconSymbol = symbol;
    }

    void setLabelPosition(NodeLabelPosition position) {
        labelPosition = position;
    }

    void setDisabled(bool value) {
        disabled = value;
    }

    void setAsTemplate(bool value) {
        isTemplate = value;
    }

    void setCurrentFlag(bool value) {
        isCurrentFlag = value;
    }

    void setAsSubgraph(bool value, int id = -1, const UUID& uuid = "") {
        isSubgraph = value;
        subgraphId = id;
        subgraphUuid = uuid;
    }

    template<typename T>
    void setMetadata(const std::string& key, const T& value) {
        metadata.setAttribute(key, value);
    }

    template<typename T>
    T getMetadata(const std::string& key, const T& defaultValue = T()) const {
        return metadata.getAttribute<T>(key, defaultValue);
    }
};

struct Connection {
    int id;
    UUID uuid;
    int startNodeId;
    UUID startNodeUuid;
    int startPinId;
    UUID startPinUuid;
    int endNodeId;
    UUID endNodeUuid;
    int endPinId;
    UUID endPinUuid;
    bool selected;
    Metadata metadata;
    int subgraphId = -1;

    Connection() : id(-1), uuid(generateUUID()), startNodeId(-1), startPinId(-1),
                   endNodeId(-1), endPinId(-1), selected(false), subgraphId(-1) {}

    Connection(int id, int startNodeId, int startPinId, int endNodeId, int endPinId)
        : id(id), uuid(generateUUID()), startNodeId(startNodeId), startPinId(startPinId),
          endNodeId(endNodeId), endPinId(endPinId), selected(false) {}

    Connection(const UUID& existingUuid, int id, int startNodeId, const UUID& startNodeUuid,
               int startPinId, const UUID& startPinUuid, int endNodeId, const UUID& endNodeUuid,
               int endPinId, const UUID& endPinUuid)
        : id(id), uuid(existingUuid), startNodeId(startNodeId), startNodeUuid(startNodeUuid),
          startPinId(startPinId), startPinUuid(startPinUuid), endNodeId(endNodeId),
          endNodeUuid(endNodeUuid), endPinId(endPinId), endPinUuid(endPinUuid), selected(false), subgraphId(-1) {}

    void setSubgraphId(int id) {
        subgraphId = id;
        metadata.setAttribute("subgraphId", id);
    }

    int getSubgraphId() const {
        if (subgraphId >= 0) return subgraphId;
        return metadata.getAttribute<int>("subgraphId", -1);
    }

    template<typename T>
    void setMetadata(const std::string& key, const T& value) {
        metadata.setAttribute(key, value);
    }

    template<typename T>
    T getMetadata(const std::string& key, const T& defaultValue = T()) const {
        return metadata.getAttribute<T>(key, defaultValue);
    }
};

struct Group {
    int id;
    UUID uuid;
    std::string name;
    Vec2 position;
    Vec2 size;
    std::unordered_set<int> nodes;
    std::unordered_set<UUID> nodeUuids;
    bool collapsed;
    bool selected;
    Color color;
    GroupStyle style;
    Metadata metadata;

    Group() : id(-1), uuid(generateUUID()), position(0.0f, 0.0f), size(200.0f, 150.0f),
              collapsed(false), selected(false), style(GroupStyle::Default) {
        color = Color(0.3f, 0.3f, 0.4f, 0.4f);
    }

    Group(int id, const std::string& name, const Vec2& pos, const Vec2& size)
        : id(id), uuid(generateUUID()), name(name), position(pos), size(size),
          collapsed(false), selected(false), style(GroupStyle::Default) {
        color = Color(0.3f, 0.3f, 0.4f, 0.4f);
    }

    Group(const UUID& existingUuid, int id, const std::string& name, const Vec2& pos, const Vec2& size)
        : id(id), uuid(existingUuid), name(name), position(pos), size(size),
          collapsed(false), selected(false), style(GroupStyle::Default) {
        color = Color(0.3f, 0.3f, 0.4f, 0.4f);
    }

    void setSubgraphId(int id) {
        metadata.setAttribute("subgraphId", id);
    }

    int getSubgraphId() const {
        return metadata.getAttribute<int>("subgraphId", -1);
    }

    void setColor(const Color& newColor) {
        color = newColor;
    }

    void setStyle(GroupStyle newStyle) {
        style = newStyle;
    }

    void setCollapsed(bool value) {
        collapsed = value;
    }

    template<typename T>
    void setMetadata(const std::string& key, const T& value) {
        metadata.setAttribute(key, value);
    }

    template<typename T>
    T getMetadata(const std::string& key, const T& defaultValue = T()) const {
        return metadata.getAttribute<T>(key, defaultValue);
    }
};

struct Subgraph {
    int id;
    UUID uuid;
    std::string name;
    std::vector<int> nodeIds;
    std::vector<UUID> nodeUuids;
    std::vector<int> connectionIds;
    std::vector<UUID> connectionUuids;
    std::vector<int> groupIds;
    std::vector<UUID> groupUuids;
    std::vector<int> interfaceInputs;
    std::vector<int> interfaceOutputs;
    int parentSubgraphId;
    UUID parentSubgraphUuid;
    std::vector<int> childSubgraphIds;
    std::vector<UUID> childSubgraphUuids;
    bool isExpanded;
    Vec2 viewPosition;
    float viewScale;
    std::string description;
    std::string category;
    bool isTemplate;
    std::string iconSymbol;
    Color accentColor;
    Metadata metadata;

    static int nextId;

    Subgraph() : id(-1), uuid(generateUUID()), parentSubgraphId(-1), isExpanded(true),
                 viewPosition(0.0f, 0.0f), viewScale(1.0f), isTemplate(false) {
        description = "";
        category = "Default";
        iconSymbol = "";
        accentColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
    }

    Subgraph(int id, const std::string& name)
        : id(id), uuid(generateUUID()), name(name), parentSubgraphId(-1), isExpanded(true),
          viewPosition(0.0f, 0.0f), viewScale(1.0f), isTemplate(false) {
        description = "";
        category = "Default";
        iconSymbol = "";
        accentColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
    }

    Subgraph(const UUID& existingUuid, int id, const std::string& name)
        : id(id), uuid(existingUuid), name(name), parentSubgraphId(-1), isExpanded(true),
          viewPosition(0.0f, 0.0f), viewScale(1.0f), isTemplate(false) {
        description = "";
        category = "Default";
        iconSymbol = "";
        accentColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
    }

    void addNode(int nodeId, const UUID& nodeUuid = "") {
        if (!containsNode(nodeId)) {
            nodeIds.push_back(nodeId);
            if (!nodeUuid.empty()) {
                nodeUuids.push_back(nodeUuid);
            }
        }
    }

    void removeNode(int nodeId) {
        auto it = std::find(nodeIds.begin(), nodeIds.end(), nodeId);
        if (it != nodeIds.end()) {
            size_t index = std::distance(nodeIds.begin(), it);
            nodeIds.erase(it);
            if (index < nodeUuids.size()) {
                nodeUuids.erase(nodeUuids.begin() + index);
            }
        }
    }

    void removeNodeByUUID(const UUID& nodeUuid) {
        auto it = std::find(nodeUuids.begin(), nodeUuids.end(), nodeUuid);
        if (it != nodeUuids.end()) {
            size_t index = std::distance(nodeUuids.begin(), it);
            nodeUuids.erase(it);
            if (index < nodeIds.size()) {
                nodeIds.erase(nodeIds.begin() + index);
            }
        }
    }

    bool containsNode(int nodeId) const {
        return std::find(nodeIds.begin(), nodeIds.end(), nodeId) != nodeIds.end();
    }

    bool containsNodeUUID(const UUID& nodeUuid) const {
        return std::find(nodeUuids.begin(), nodeUuids.end(), nodeUuid) != nodeUuids.end();
    }

    void addConnection(int connectionId, const UUID& connectionUuid = "") {
        if (!containsConnection(connectionId)) {
            connectionIds.push_back(connectionId);
            if (!connectionUuid.empty()) {
                connectionUuids.push_back(connectionUuid);
            }
        }
    }

    void removeConnection(int connectionId) {
        auto it = std::find(connectionIds.begin(), connectionIds.end(), connectionId);
        if (it != connectionIds.end()) {
            size_t index = std::distance(connectionIds.begin(), it);
            connectionIds.erase(it);
            if (index < connectionUuids.size()) {
                connectionUuids.erase(connectionUuids.begin() + index);
            }
        }
    }

    void removeConnectionByUUID(const UUID& connectionUuid) {
        auto it = std::find(connectionUuids.begin(), connectionUuids.end(), connectionUuid);
        if (it != connectionUuids.end()) {
            size_t index = std::distance(connectionUuids.begin(), it);
            connectionUuids.erase(it);
            if (index < connectionIds.size()) {
                connectionIds.erase(connectionIds.begin() + index);
            }
        }
    }

    bool containsConnection(int connectionId) const {
        return std::find(connectionIds.begin(), connectionIds.end(), connectionId) != connectionIds.end();
    }

    bool containsConnectionUUID(const UUID& connectionUuid) const {
        return std::find(connectionUuids.begin(), connectionUuids.end(), connectionUuid) != connectionUuids.end();
    }

    void addGroup(int groupId, const UUID& groupUuid = "") {
        if (!containsGroup(groupId)) {
            groupIds.push_back(groupId);
            if (!groupUuid.empty()) {
                groupUuids.push_back(groupUuid);
            }
        }
    }

    void removeGroup(int groupId) {
        auto it = std::find(groupIds.begin(), groupIds.end(), groupId);
        if (it != groupIds.end()) {
            size_t index = std::distance(groupIds.begin(), it);
            groupIds.erase(it);
            if (index < groupUuids.size()) {
                groupUuids.erase(groupUuids.begin() + index);
            }
        }
    }

    void removeGroupByUUID(const UUID& groupUuid) {
        auto it = std::find(groupUuids.begin(), groupUuids.end(), groupUuid);
        if (it != groupUuids.end()) {
            size_t index = std::distance(groupUuids.begin(), it);
            groupUuids.erase(it);
            if (index < groupIds.size()) {
                groupIds.erase(groupIds.begin() + index);
            }
        }
    }

    bool containsGroup(int groupId) const {
        return std::find(groupIds.begin(), groupIds.end(), groupId) != groupIds.end();
    }

    bool containsGroupUUID(const UUID& groupUuid) const {
        return std::find(groupUuids.begin(), groupUuids.end(), groupUuid) != groupUuids.end();
    }

    void exposeInput(int nodeId, int pinId) {
        int interfaceId = (nodeId << 16) | pinId;
        if (std::find(interfaceInputs.begin(), interfaceInputs.end(), interfaceId) == interfaceInputs.end()) {
            interfaceInputs.push_back(interfaceId);
        }
    }

    void exposeOutput(int nodeId, int pinId) {
        int interfaceId = (nodeId << 16) | pinId;
        if (std::find(interfaceOutputs.begin(), interfaceOutputs.end(), interfaceId) == interfaceOutputs.end()) {
            interfaceOutputs.push_back(interfaceId);
        }
    }

    void unexposeInput(int nodeId, int pinId) {
        int interfaceId = (nodeId << 16) | pinId;
        auto it = std::find(interfaceInputs.begin(), interfaceInputs.end(), interfaceId);
        if (it != interfaceInputs.end()) {
            interfaceInputs.erase(it);
        }
    }

    void unexposeOutput(int nodeId, int pinId) {
        int interfaceId = (nodeId << 16) | pinId;
        auto it = std::find(interfaceOutputs.begin(), interfaceOutputs.end(), interfaceId);
        if (it != interfaceOutputs.end()) {
            interfaceOutputs.erase(it);
        }
    }

    bool isInputExposed(int nodeId, int pinId) const {
        int interfaceId = (nodeId << 16) | pinId;
        return std::find(interfaceInputs.begin(), interfaceInputs.end(), interfaceId) != interfaceInputs.end();
    }

    bool isOutputExposed(int nodeId, int pinId) const {
        int interfaceId = (nodeId << 16) | pinId;
        return std::find(interfaceOutputs.begin(), interfaceOutputs.end(), interfaceId) != interfaceOutputs.end();
    }

    void addChildSubgraph(int subgraphId, const UUID& subgraphUuid = "") {
        if (!containsSubgraph(subgraphId)) {
            childSubgraphIds.push_back(subgraphId);
            if (!subgraphUuid.empty()) {
                childSubgraphUuids.push_back(subgraphUuid);
            }
        }
    }

    void removeChildSubgraph(int subgraphId) {
        auto it = std::find(childSubgraphIds.begin(), childSubgraphIds.end(), subgraphId);
        if (it != childSubgraphIds.end()) {
            size_t index = std::distance(childSubgraphIds.begin(), it);
            childSubgraphIds.erase(it);
            if (index < childSubgraphUuids.size()) {
                childSubgraphUuids.erase(childSubgraphUuids.begin() + index);
            }
        }
    }

    void removeChildSubgraphByUUID(const UUID& subgraphUuid) {
        auto it = std::find(childSubgraphUuids.begin(), childSubgraphUuids.end(), subgraphUuid);
        if (it != childSubgraphUuids.end()) {
            size_t index = std::distance(childSubgraphUuids.begin(), it);
            childSubgraphUuids.erase(it);
            if (index < childSubgraphIds.size()) {
                childSubgraphIds.erase(childSubgraphIds.begin() + index);
            }
        }
    }

    bool containsSubgraph(int subgraphId) const {
        return std::find(childSubgraphIds.begin(), childSubgraphIds.end(), subgraphId) != childSubgraphIds.end();
    }

    bool containsSubgraphUUID(const UUID& subgraphUuid) const {
        return std::find(childSubgraphUuids.begin(), childSubgraphUuids.end(), subgraphUuid) != childSubgraphUuids.end();
    }

    void setIconSymbol(const std::string& symbol) {
        iconSymbol = symbol;
    }

    void setAccentColor(const Color& color) {
        accentColor = color;
    }

    void setIsTemplate(bool template_value) {
        isTemplate = template_value;
    }

    void setDescription(const std::string& desc) {
        description = desc;
    }

    void setCategory(const std::string& cat) {
        category = cat;
    }

    void setViewState(const Vec2& position, float scale) {
        viewPosition = position;
        viewScale = scale;
    }

    bool validate() const {
        return !name.empty() && id >= 0;
    }

    std::vector<std::string> getValidationErrors() const {
        std::vector<std::string> errors;

        if (name.empty()) {
            errors.push_back("Le nom du sous-graphe ne peut pas être vide");
        }

        if (id < 0) {
            errors.push_back("L'ID du sous-graphe doit être positif");
        }

        return errors;
    }

    template<typename T>
    void setMetadata(const std::string& key, const T& value) {
        metadata.setAttribute(key, value);
    }

    template<typename T>
    T getMetadata(const std::string& key, const T& defaultValue = T()) const {
        return metadata.getAttribute<T>(key, defaultValue);
    }
};

struct EditorState {
    std::vector<Node> nodes;
    std::vector<Connection> connections;
    std::vector<Group> groups;
    std::vector<Subgraph> subgraphs;
    Vec2 viewPosition;
    float viewScale;
    std::vector<int> selectedNodes;
    std::vector<int> selectedConnections;
    std::vector<int> selectedGroups;
    int currentSubgraphId;

    EditorState() : viewPosition(0.0f, 0.0f), viewScale(1.0f), currentSubgraphId(-1) {}
};

struct SerializedPin {
    int id;
    UUID uuid;
    std::string name;
    bool isInput;
    PinType type;
    PinShape shape;
    Metadata metadata;

    SerializedPin() = default;

    SerializedPin(const Pin& pin)
        : id(pin.id), uuid(pin.uuid), name(pin.name), isInput(pin.isInput),
          type(pin.type), shape(pin.shape), metadata(pin.metadata) {}
};

struct SerializedNode {
    int id;
    UUID uuid;
    std::string name;
    std::string type;
    Vec2 position;
    Vec2 size;
    bool isSubgraph;
    int subgraphId;
    UUID subgraphUuid;
    std::vector<SerializedPin> inputs;
    std::vector<SerializedPin> outputs;
    Metadata metadata;

    SerializedNode() = default;

    SerializedNode(const Node& node)
        : id(node.id), uuid(node.uuid), name(node.name), type(node.type),
          position(node.position), size(node.size), isSubgraph(node.isSubgraph),
          subgraphId(node.subgraphId), subgraphUuid(node.subgraphUuid), metadata(node.metadata) {
        for (const auto& pin : node.inputs) {
            inputs.emplace_back(pin);
        }
        for (const auto& pin : node.outputs) {
            outputs.emplace_back(pin);
        }
    }
};

struct SerializedConnection {
    int id;
    UUID uuid;
    int startNodeId;
    UUID startNodeUuid;
    int startPinId;
    UUID startPinUuid;
    int endNodeId;
    UUID endNodeUuid;
    int endPinId;
    UUID endPinUuid;
    Metadata metadata;

    SerializedConnection() = default;

    SerializedConnection(const Connection& conn)
        : id(conn.id), uuid(conn.uuid), startNodeId(conn.startNodeId), startNodeUuid(conn.startNodeUuid),
          startPinId(conn.startPinId), startPinUuid(conn.startPinUuid), endNodeId(conn.endNodeId),
          endNodeUuid(conn.endNodeUuid), endPinId(conn.endPinId), endPinUuid(conn.endPinUuid),
          metadata(conn.metadata) {}
};

struct SerializedGroup {
    int id;
    UUID uuid;
    std::string name;
    Vec2 position;
    Vec2 size;
    Color color;
    GroupStyle style;
    bool collapsed;
    std::vector<int> nodeIds;
    std::vector<UUID> nodeUuids;
    Metadata metadata;

    SerializedGroup() = default;

    SerializedGroup(const Group& group)
        : id(group.id), uuid(group.uuid), name(group.name), position(group.position),
          size(group.size), color(group.color), style(group.style), collapsed(group.collapsed),
          metadata(group.metadata) {
        nodeIds.assign(group.nodes.begin(), group.nodes.end());
        nodeUuids.assign(group.nodeUuids.begin(), group.nodeUuids.end());
    }
};

struct SerializedSubgraph {
    int id;
    UUID uuid;
    std::string name;
    std::vector<int> nodeIds;
    std::vector<UUID> nodeUuids;
    std::vector<int> connectionIds;
    std::vector<UUID> connectionUuids;
    std::vector<int> groupIds;
    std::vector<UUID> groupUuids;
    std::vector<int> interfaceInputs;
    std::vector<int> interfaceOutputs;
    int parentSubgraphId;
    UUID parentSubgraphUuid;
    std::vector<int> childSubgraphIds;
    std::vector<UUID> childSubgraphUuids;
    Vec2 viewPosition;
    float viewScale;
    std::string description;
    std::string category;
    bool isTemplate;
    std::string iconSymbol;
    Color accentColor;
    Metadata metadata;

    SerializedSubgraph() = default;

    SerializedSubgraph(const Subgraph& subgraph)
        : id(subgraph.id), uuid(subgraph.uuid), name(subgraph.name),
          nodeIds(subgraph.nodeIds), nodeUuids(subgraph.nodeUuids),
          connectionIds(subgraph.connectionIds), connectionUuids(subgraph.connectionUuids),
          groupIds(subgraph.groupIds), groupUuids(subgraph.groupUuids),
          interfaceInputs(subgraph.interfaceInputs), interfaceOutputs(subgraph.interfaceOutputs),
          parentSubgraphId(subgraph.parentSubgraphId), parentSubgraphUuid(subgraph.parentSubgraphUuid),
          childSubgraphIds(subgraph.childSubgraphIds), childSubgraphUuids(subgraph.childSubgraphUuids),
          viewPosition(subgraph.viewPosition), viewScale(subgraph.viewScale),
          description(subgraph.description), category(subgraph.category),
          isTemplate(subgraph.isTemplate), iconSymbol(subgraph.iconSymbol),
          accentColor(subgraph.accentColor), metadata(subgraph.metadata) {}
};

struct SerializedState {
    std::vector<SerializedNode> nodes;
    std::vector<SerializedConnection> connections;
    std::vector<SerializedGroup> groups;
    std::vector<SerializedSubgraph> subgraphs;
    Vec2 viewPosition;
    float viewScale;

    SerializedState() : viewPosition(0.0f, 0.0f), viewScale(1.0f) {}

    SerializedState(const EditorState& state)
        : viewPosition(state.viewPosition), viewScale(state.viewScale) {
        for (const auto& node : state.nodes) {
            nodes.emplace_back(node);
        }
        for (const auto& conn : state.connections) {
            connections.emplace_back(conn);
        }
        for (const auto& group : state.groups) {
            groups.emplace_back(group);
        }
        for (const auto& subgraph : state.subgraphs) {
            subgraphs.emplace_back(subgraph);
        }
    }
};

} // namespace NodeEditorCore

#endif // NODE_EDITOR_TYPES_H