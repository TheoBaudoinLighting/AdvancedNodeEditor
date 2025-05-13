#ifndef SERIALIZED_TYPES_H
#define SERIALIZED_TYPES_H

#include "../../Core/Types/CoreTypes.h"
#include "../../Core/Types/UuidTypes.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <any>

namespace ANE {

struct SerializedPin {
    int id;
    UUID uuid;
    std::string name;
    bool isInput;
    PinType type;
    PinShape shape;
    std::unordered_map<std::string, std::any> metadata;
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
    std::unordered_map<std::string, std::any> metadata;
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
    std::unordered_map<std::string, std::any> metadata;
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
    std::unordered_map<std::string, std::any> metadata;
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
    Vec2 viewPosition;
    float viewScale;
    std::unordered_map<std::string, std::any> metadata;
};

struct SerializedState {
    std::vector<SerializedNode> nodes;
    std::vector<SerializedConnection> connections;
    std::vector<SerializedGroup> groups;
    std::vector<SerializedSubgraph> subgraphs;
};

} // namespace ANE

#endif // SERIALIZED_TYPES_H