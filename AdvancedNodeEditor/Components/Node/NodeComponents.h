#ifndef NODE_COMPONENTS_H
#define NODE_COMPONENTS_H

#include "../../Core/Types/CoreTypes.h"
#include "../../Core/Types/UuidTypes.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <functional>
#include <memory>
#include <algorithm>

namespace NodeEditorCore {

struct Pin {
    int id;
    ANE::UUID uuid;
    std::string name;
    std::string label;
    bool isInput;
    PinType type;
    PinShape shape;
    Color color;
    bool connected;
    Metadata metadata;

    Pin()
        : id(0), uuid(ANE::generateUUID()), name(""), label(""), isInput(false), type(PinType::Blue),
          shape(PinShape::Circle), connected(false) {}

    Pin(int id, const std::string& name, bool isInput, PinType type)
        : id(id), uuid(ANE::generateUUID()), name(name), label(name), isInput(isInput), type(type),
          shape(PinShape::Circle), connected(false) {
        setColorByType();
    }

    Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape)
        : id(id), uuid(ANE::generateUUID()), name(name), label(name), isInput(isInput), type(type),
          shape(shape), connected(false) {
        setColorByType();
    }

    Pin(const ANE::UUID& existingUuid, int id, const std::string& name, bool isInput, PinType type, PinShape shape)
        : id(id), uuid(existingUuid.empty() ? ANE::generateUUID() : existingUuid), name(name), label(name),
          isInput(isInput), type(type), shape(shape), connected(false) {
        setColorByType();
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
    ANE::UUID uuid;
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
    Metadata metadata;

    Node(int id, const std::string& name, const std::string& type, const Vec2& pos)
        : id(id), uuid(ANE::generateUUID()), name(name), type(type), position(pos), size(140.0f, 28.0f),
          selected(false), disabled(false), groupId(-1), isTemplate(false), isCurrentFlag(false),
          labelPosition(NodeLabelPosition::Right), isSubgraph(false), subgraphId(-1) {}

    Node(const ANE::UUID& existingUuid, int id, const std::string& name, const std::string& type, const Vec2& pos)
        : id(id), uuid(existingUuid), name(name), type(type), position(pos), size(140.0f, 28.0f),
          selected(false), disabled(false), groupId(-1), isTemplate(false), isCurrentFlag(false),
          labelPosition(NodeLabelPosition::Right), isSubgraph(false), subgraphId(-1) {}

    Pin* findPin(int pinId);

    Pin* findPinByUUID(const ANE::UUID& pinUuid) {
        for (auto& pin : inputs) {
            if (pin.uuid == pinUuid) return &pin;
        }
        for (auto& pin : outputs) {
            if (pin.uuid == pinUuid) return &pin;
        }
        return nullptr;
    }

    const Pin* findPin(int pinId) const;

    const Pin* findPinByUUID(const ANE::UUID& pinUuid) const {
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
};

struct Connection {
    int id;
    ANE::UUID uuid;
    int startNodeId;
    ANE::UUID startNodeUuid;
    int startPinId;
    ANE::UUID startPinUuid;
    int endNodeId;
    ANE::UUID endNodeUuid;
    int endPinId;
    ANE::UUID endPinUuid;
    bool selected;
    Metadata metadata;

    Connection(int id, int startNodeId, int startPinId, int endNodeId, int endPinId)
        : id(id), uuid(ANE::generateUUID()), startNodeId(startNodeId), startPinId(startPinId),
          endNodeId(endNodeId), endPinId(endPinId), selected(false) {}

    Connection(const ANE::UUID& existingUuid, int id, int startNodeId, const ANE::UUID& startNodeUuid,
               int startPinId, const ANE::UUID& startPinUuid, int endNodeId, const ANE::UUID& endNodeUuid,
               int endPinId, const ANE::UUID& endPinUuid)
        : id(id), uuid(existingUuid), startNodeId(startNodeId), startNodeUuid(startNodeUuid),
          startPinId(startPinId), startPinUuid(startPinUuid), endNodeId(endNodeId),
          endNodeUuid(endNodeUuid), endPinId(endPinId), endPinUuid(endPinUuid), selected(false) {}

    void setSubgraphId(int id) {
        metadata.setAttribute("subgraphId", id);
    }

    int getSubgraphId() const {
        return metadata.getAttribute<int>("subgraphId", -1);
    }
};

struct Group {
    int id;
    ANE::UUID uuid;
    std::string name;
    Vec2 position;
    Vec2 size;
    std::unordered_set<int> nodes;
    std::unordered_set<ANE::UUID> nodeUuids;
    bool collapsed;
    bool selected;
    Color color;
    GroupStyle style;
    Metadata metadata;

    Group(int id, const std::string& name, const Vec2& pos, const Vec2& size)
        : id(id), uuid(ANE::generateUUID()), name(name), position(pos), size(size),
          collapsed(false), selected(false), style(GroupStyle::Default) {
        color = Color(0.3f, 0.3f, 0.4f, 0.4f);
    }

    Group(const ANE::UUID& existingUuid, int id, const std::string& name, const Vec2& pos, const Vec2& size)
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
};

struct Subgraph {
    int id;
    ANE::UUID uuid;
    std::string name;
    std::vector<int> nodeIds;
    std::vector<ANE::UUID> nodeUuids;
    std::vector<int> connectionIds;
    std::vector<ANE::UUID> connectionUuids;
    std::vector<int> groupIds;
    std::vector<ANE::UUID> groupUuids;
    std::vector<int> interfaceInputs;
    std::vector<int> interfaceOutputs;
    int parentSubgraphId;
    ANE::UUID parentSubgraphUuid;
    std::vector<int> childSubgraphIds;
    std::vector<ANE::UUID> childSubgraphUuids;
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

    Subgraph(int id, const std::string& name)
        : id(id), uuid(ANE::generateUUID()), name(name), parentSubgraphId(-1), isExpanded(true),
          viewPosition(0.0f, 0.0f), viewScale(1.0f), isTemplate(false) {}

    Subgraph(const ANE::UUID& existingUuid, int id, const std::string& name)
        : id(id), uuid(existingUuid), name(name), parentSubgraphId(-1), isExpanded(true),
          viewPosition(0.0f, 0.0f), viewScale(1.0f), isTemplate(false) {}

    void addNode(int nodeId, const ANE::UUID& nodeUuid) {
        if (!containsNode(nodeId)) {
            nodeIds.push_back(nodeId);
            nodeUuids.push_back(nodeUuid);
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

    void removeNodeByUUID(const ANE::UUID& nodeUuid) {
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

    bool containsNodeUUID(const ANE::UUID& nodeUuid) const {
        return std::find(nodeUuids.begin(), nodeUuids.end(), nodeUuid) != nodeUuids.end();
    }

    void addConnection(int connectionId, const ANE::UUID& connectionUuid) {
        if (!containsConnection(connectionId)) {
            connectionIds.push_back(connectionId);
            connectionUuids.push_back(connectionUuid);
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

    void removeConnectionByUUID(const ANE::UUID& connectionUuid) {
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

    bool containsConnectionUUID(const ANE::UUID& connectionUuid) const {
        return std::find(connectionUuids.begin(), connectionUuids.end(), connectionUuid) != connectionUuids.end();
    }

    void addGroup(int groupId, const ANE::UUID& groupUuid) {
        if (!containsGroup(groupId)) {
            groupIds.push_back(groupId);
            groupUuids.push_back(groupUuid);
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

    void removeGroupByUUID(const ANE::UUID& groupUuid) {
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

    bool containsGroupUUID(const ANE::UUID& groupUuid) const {
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

    void addChildSubgraph(int subgraphId, const ANE::UUID& subgraphUuid) {
        if (!containsSubgraph(subgraphId)) {
            childSubgraphIds.push_back(subgraphId);
            childSubgraphUuids.push_back(subgraphUuid);
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

    void removeChildSubgraphByUUID(const ANE::UUID& subgraphUuid) {
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

    bool containsSubgraphUUID(const ANE::UUID& subgraphUuid) const {
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
};

}

namespace ANE {

struct Pin {
    int id;
    ANE::UUID uuid;
    std::string name;
    bool isInput;
    PinType type;
    PinShape shape;
    Metadata metadata;

    Pin()
        : id(0), uuid(ANE::generateUUID()), name(""), isInput(false), type(PinType::Blue), shape(PinShape::Circle) {}

    Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape)
        : id(id), uuid(ANE::generateUUID()), name(name), isInput(isInput), type(type), shape(shape) {}

    Pin(const ANE::UUID& existingUuid, int id, const std::string& name, bool isInput, PinType type, PinShape shape)
        : id(id), uuid(existingUuid), name(name), isInput(isInput), type(type), shape(shape) {}

    template<typename T>
    void setMetadata(const std::string& key, const T& value) {
        metadata.setAttribute(key, value);
    }

    template<typename T>
    T getMetadata(const std::string& key, const T& defaultValue = T()) const {
        return metadata.getAttribute<T>(key, defaultValue);
    }
};

struct Node {
    int id;
    ANE::UUID uuid;
    std::string name;
    std::string type;
    std::string iconSymbol;
    NodeLabelPosition labelPosition;
    bool disabled;
    bool isTemplate;
    bool isCurrentFlag;
    bool isSubgraph;
    int subgraphId;
    ANE::UUID subgraphUuid;
    Vec2 position;
    Vec2 size;
    bool selected;
    Metadata metadata;

    Node(int id, const std::string& name, const std::string& type)
        : id(id), uuid(ANE::generateUUID()), name(name), type(type), labelPosition(NodeLabelPosition::Right),
          disabled(false), isTemplate(false), isCurrentFlag(false), isSubgraph(false),
          subgraphId(-1), position(0.0f, 0.0f), size(140.0f, 28.0f), selected(false) {}

    Node(const ANE::UUID& existingUuid, int id, const std::string& name, const std::string& type)
        : id(id), uuid(existingUuid), name(name), type(type), labelPosition(NodeLabelPosition::Right),
          disabled(false), isTemplate(false), isCurrentFlag(false), isSubgraph(false),
          subgraphId(-1), position(0.0f, 0.0f), size(140.0f, 28.0f), selected(false) {}

    void setIconSymbol(const std::string& symbol);
    void setLabelPosition(NodeLabelPosition position);
    void setDisabled(bool value);
    void setAsTemplate(bool value);
    void setCurrentFlag(bool value);
    void setAsSubgraph(bool value, int id = -1, const ANE::UUID& uuid = "");

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
    ANE::UUID uuid;
    std::string name;
    Color color;
    GroupStyle style;
    bool collapsed;
    bool selected;
    Vec2 position;
    Vec2 size;
    std::unordered_set<int> nodes;
    std::unordered_set<ANE::UUID> nodeUuids;
    Metadata metadata;

    Group(int id, const std::string& name)
        : id(id), uuid(ANE::generateUUID()), name(name), collapsed(false), style(GroupStyle::Default),
          selected(false), position(0.0f, 0.0f), size(200.0f, 150.0f) {
        color = Color(0.2f, 0.2f, 0.25f, 0.25f);
    }

    Group(const ANE::UUID& existingUuid, int id, const std::string& name)
        : id(id), uuid(existingUuid), name(name), collapsed(false), style(GroupStyle::Default),
          selected(false), position(0.0f, 0.0f), size(200.0f, 150.0f) {
        color = Color(0.2f, 0.2f, 0.25f, 0.25f);
    }

    void setColor(const Color& newColor);

    void setStyle(GroupStyle newStyle);

    void setCollapsed(bool value);

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
    ANE::UUID uuid;
    std::string name;
    std::vector<int> nodeIds;
    std::vector<ANE::UUID> nodeUuids;
    std::vector<int> connectionIds;
    std::vector<ANE::UUID> connectionUuids;
    std::vector<int> groupIds;
    std::vector<ANE::UUID> groupUuids;
    std::vector<int> interfaceInputs;
    std::vector<int> interfaceOutputs;
    int parentSubgraphId;
    ANE::UUID parentSubgraphUuid;
    std::vector<int> childSubgraphIds;
    std::vector<ANE::UUID> childSubgraphUuids;
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

    Subgraph(int id, const std::string& name)
        : id(id), uuid(ANE::generateUUID()), name(name), parentSubgraphId(-1), isExpanded(true),
          viewPosition(0.0f, 0.0f), viewScale(1.0f), isTemplate(false) {
        nodeIds.clear();
        nodeUuids.clear();
        connectionIds.clear();
        connectionUuids.clear();
        groupIds.clear();
        groupUuids.clear();
        interfaceInputs.clear();
        interfaceOutputs.clear();
        childSubgraphIds.clear();
        childSubgraphUuids.clear();

        description = "";
        category = "Default";
        iconSymbol = "";
        accentColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
    }

    Subgraph(const ANE::UUID& existingUuid, int id, const std::string& name)
        : id(id), uuid(existingUuid), name(name), parentSubgraphId(-1), isExpanded(true),
          viewPosition(0.0f, 0.0f), viewScale(1.0f), isTemplate(false) {
        nodeIds.clear();
        nodeUuids.clear();
        connectionIds.clear();
        connectionUuids.clear();
        groupIds.clear();
        groupUuids.clear();
        interfaceInputs.clear();
        interfaceOutputs.clear();
        childSubgraphIds.clear();
        childSubgraphUuids.clear();

        description = "";
        category = "Default";
        iconSymbol = "";
        accentColor = Color(0.4f, 0.6f, 0.8f, 1.0f);
    }

    void addNode(int nodeId, const ANE::UUID& nodeUuid = "") {
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

    void removeNodeByUUID(const ANE::UUID& nodeUuid) {
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

    bool containsNodeUUID(const ANE::UUID& nodeUuid) const {
        return std::find(nodeUuids.begin(), nodeUuids.end(), nodeUuid) != nodeUuids.end();
    }

    void addConnection(int connectionId, const ANE::UUID& connectionUuid = "") {
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

    void removeConnectionByUUID(const ANE::UUID& connectionUuid) {
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

    bool containsConnectionUUID(const ANE::UUID& connectionUuid) const {
        return std::find(connectionUuids.begin(), connectionUuids.end(), connectionUuid) != connectionUuids.end();
    }

    void addGroup(int groupId, const ANE::UUID& groupUuid = "") {
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

    void removeGroupByUUID(const ANE::UUID& groupUuid) {
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

    bool containsGroupUUID(const ANE::UUID& groupUuid) const {
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

    void addChildSubgraph(int subgraphId, const ANE::UUID& subgraphUuid = "") {
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

    void removeChildSubgraphByUUID(const ANE::UUID& subgraphUuid) {
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

    bool containsSubgraphUUID(const ANE::UUID& subgraphUuid) const {
        return std::find(childSubgraphUuids.begin(), childSubgraphUuids.end(), subgraphUuid) != childSubgraphUuids.end();
    }

    void setIconSymbol(const std::string& symbol) {
        iconSymbol = symbol;
    }

    void setAccentColor(const Color& color) {
        accentColor = color;
    }

    void setIsTemplate(bool isTemplate) {
        this->isTemplate = isTemplate;
    }

    void setDescription(const std::string& desc) {
        description = desc;
    }

    void setCategory(const std::string& category) {
        this->category = category;
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

}

#endif