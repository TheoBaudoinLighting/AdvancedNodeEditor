#ifndef NODE_COMPONENTS_H
#define NODE_COMPONENTS_H

#include "CoreTypes.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <functional>
#include <memory>
#include <algorithm>

namespace NodeEditorCore {

struct Pin {
    int id;
    std::string name;
    std::string label;
    bool isInput;
    PinType type;
    PinShape shape;
    Color color;
    bool connected;
    Metadata metadata;

    Pin()
        : id(0), name(""), label(""), isInput(false), type(PinType::Blue),
          shape(PinShape::Circle), connected(false) {}

    Pin(int id, const std::string& name, bool isInput, PinType type)
        : id(id), name(name), label(name), isInput(isInput), type(type),
          shape(PinShape::Circle), connected(false) {

        setColorByType();
    }

    Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape)
        : id(id), name(name), label(name), isInput(isInput), type(type),
          shape(shape), connected(false) {

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

    Node(int id, const std::string& name, const std::string& type, const Vec2& pos);

    Pin* findPin(int pinId);
    const Pin* findPin(int pinId) const;
    
    void setSubgraphId(int id) {
        metadata.setAttribute("subgraphId", id);
    }
    
    int getSubgraphId() const {
        return metadata.getAttribute<int>("subgraphId", -1);
    }
};

struct Connection {
    int id;
    int startNodeId;
    int startPinId;
    int endNodeId;
    int endPinId;
    bool selected;
    Metadata metadata;

    Connection(int id, int startNodeId, int startPinId, int endNodeId, int endPinId);
    
    void setSubgraphId(int id) {
        metadata.setAttribute("subgraphId", id);
    }
    
    int getSubgraphId() const {
        return metadata.getAttribute<int>("subgraphId", -1);
    }
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
    Metadata metadata;

    Group(int id, const std::string& name, const Vec2& pos, const Vec2& size);
    
    void setSubgraphId(int id) {
        metadata.setAttribute("subgraphId", id);
    }
    
    int getSubgraphId() const {
        return metadata.getAttribute<int>("subgraphId", -1);
    }
};

struct Subgraph {
    int id;
    std::string name;
    std::vector<int> nodeIds;
    std::vector<int> connectionIds;
    std::vector<int> groupIds;
    std::vector<int> interfaceInputs;
    std::vector<int> interfaceOutputs;
    int parentSubgraphId;
    std::vector<int> childSubgraphIds;
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

    Subgraph(int id, const std::string& name);

    void addNode(int nodeId);
    void removeNode(int nodeId);
    bool containsNode(int nodeId) const;
    
    void addConnection(int connectionId);
    void removeConnection(int connectionId);
    bool containsConnection(int connectionId) const;
    
    void addGroup(int groupId);
    void removeGroup(int groupId);
    bool containsGroup(int groupId) const;
    
    void exposeInput(int nodeId, int pinId);
    void exposeOutput(int nodeId, int pinId);
    void unexposeInput(int nodeId, int pinId);
    void unexposeOutput(int nodeId, int pinId);
    bool isInputExposed(int nodeId, int pinId) const;
    bool isOutputExposed(int nodeId, int pinId) const;
    
    void addChildSubgraph(int subgraphId);
    void removeChildSubgraph(int subgraphId);
    bool containsSubgraph(int subgraphId) const;
    
    void setIconSymbol(const std::string& symbol);
    void setAccentColor(const Color& color);
    void setIsTemplate(bool isTemplate);
    void setDescription(const std::string& desc);
    void setCategory(const std::string& category);
    void setViewState(const Vec2& position, float scale);
    
    bool validate() const;
    std::vector<std::string> getValidationErrors() const;

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

namespace ANE {

struct Pin {
    int id;
    std::string name;
    bool isInput;
    PinType type;
    PinShape shape;
    Metadata metadata;

    Pin()
        : id(0), name(""), isInput(false), type(PinType::Blue), shape(PinShape::Circle) {}

    Pin(int id, const std::string& name, bool isInput, PinType type, PinShape shape)
        : id(id), name(name), isInput(isInput), type(type), shape(shape) {}

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
    std::string name;
    std::string type;
    std::string iconSymbol;
    NodeLabelPosition labelPosition;
    bool disabled;
    bool isTemplate;
    bool isCurrentFlag;
    bool isSubgraph;
    int subgraphId;
    Vec2 position;
    Vec2 size;
    bool selected;
    Metadata metadata;

    Node(int id, const std::string& name, const std::string& type);
    void setIconSymbol(const std::string& symbol);
    void setLabelPosition(NodeLabelPosition position);
    void setDisabled(bool value);
    void setAsTemplate(bool value);
    void setCurrentFlag(bool value);
    void setAsSubgraph(bool value, int subgraphId = -1);

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
    std::string name;
    Color color;
    GroupStyle style;
    bool collapsed;
    bool selected;
    Vec2 position;
    Vec2 size;
    std::unordered_set<int> nodes;
    Metadata metadata;

    Group(int id, const std::string& name);
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
    std::string name;
    std::vector<int> nodeIds;
    std::vector<int> connectionIds;
    std::vector<int> groupIds;
    std::vector<int> interfaceInputs;
    std::vector<int> interfaceOutputs;
    int parentSubgraphId;
    std::vector<int> childSubgraphIds;
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

    Subgraph(int id, const std::string& name);

    void addNode(int nodeId);
    void removeNode(int nodeId);
    bool containsNode(int nodeId) const;
    
    void addConnection(int connectionId);
    void removeConnection(int connectionId);
    bool containsConnection(int connectionId) const;
    
    void addGroup(int groupId);
    void removeGroup(int groupId);
    bool containsGroup(int groupId) const;
    
    void exposeInput(int nodeId, int pinId);
    void exposeOutput(int nodeId, int pinId);
    void unexposeInput(int nodeId, int pinId);
    void unexposeOutput(int nodeId, int pinId);
    bool isInputExposed(int nodeId, int pinId) const;
    bool isOutputExposed(int nodeId, int pinId) const;
    
    void addChildSubgraph(int subgraphId);
    void removeChildSubgraph(int subgraphId);
    bool containsSubgraph(int subgraphId) const;
    
    void setIconSymbol(const std::string& symbol);
    void setAccentColor(const Color& color);
    void setIsTemplate(bool isTemplate);
    void setDescription(const std::string& desc);
    void setCategory(const std::string& category);
    void setViewState(const Vec2& position, float scale);
    
    bool validate() const;
    std::vector<std::string> getValidationErrors() const;

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