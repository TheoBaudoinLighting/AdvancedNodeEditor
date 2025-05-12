#ifndef NODE_COMPONENTS_H
#define NODE_COMPONENTS_H

#include "CoreTypes.h"
#include <vector>
#include <string>
#include <unordered_set>
#include <functional>
#include <memory>

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

    // Constructeur par défaut
    Pin()
        : id(0), name(""), label(""), isInput(false), type(PinType::Blue),
          shape(PinShape::Circle), connected(false) {}

    // Constructeur à 4 paramètres
    Pin(int id, const std::string& name, bool isInput, PinType type)
        : id(id), name(name), label(name), isInput(isInput), type(type),
          shape(PinShape::Circle), connected(false) {

        setColorByType();
    }

    // Constructeur à 5 paramètres
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
};

struct Subgraph {
    int id;
    std::string name;
    std::vector<std::shared_ptr<Node>> nodes;
    std::vector<std::shared_ptr<Connection>> connections;
    std::vector<std::shared_ptr<Group>> groups;
    Metadata metadata;

    Subgraph(int id, const std::string& name);
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

    // Constructeur par défaut
    Pin()
        : id(0), name(""), isInput(false), type(PinType::Blue), shape(PinShape::Circle) {}

    // Constructeur principal
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
    Vec2 position; // Ajouté
    Vec2 size;     // Ajouté
    bool selected; // Ajouté
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
    bool selected;        // Ajouté
    Vec2 position;        // Ajouté
    Vec2 size;            // Ajouté
    std::unordered_set<int> nodes; // Ajouté
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
    Metadata metadata;

    Subgraph(int id, const std::string& name);

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