#include "NodeEditorAPI.h"
#include <stdexcept>
#include <algorithm>

namespace NodeEditorCore {
    NodeEditorAPI::NodeEditorAPI()
        : m_editor(std::make_unique<NodeEditor>()) {
    }

    NodeEditorAPI::~NodeEditorAPI() = default;

    void NodeEditorAPI::initialize() {
        m_editor->setupCommandSystem();
    }

    void NodeEditorAPI::setupWindow(void *, void *) {
        // This function is intentionally left empty because window initialization is handled in client code, but could be extended if needed.
    }

    void NodeEditorAPI::setWindowSize(float width, float height) {
        m_editor->getViewManager().setWindowSize(Vec2(width, height));
    }

    void NodeEditorAPI::setWindowSize(const Vec2 &size) {
        m_editor->getViewManager().setWindowSize(size);
    }

    Vec2 NodeEditorAPI::getWindowSize() const {
        return m_editor->getViewManager().getWindowSize();
    }

    void NodeEditorAPI::registerNodeType(const NodeDefinition &definition) {
        m_nodeDefinitions[definition.type] = definition;

        m_editor->registerNodeType(
            definition.type,
            definition.category,
            definition.description,
            [this, definition](const Vec2 &pos) -> Node * {
                Node *node = new Node(0, definition.name, definition.type, pos);
                if (!definition.iconSymbol.empty()) {
                    node->iconSymbol = definition.iconSymbol;
                }
                return node;
            }
        );
    }

    void NodeEditorAPI::beginFrame() {
        m_editor->beginFrame();
    }

    void NodeEditorAPI::render() {
        m_editor->render();
    }

    void NodeEditorAPI::endFrame() {
        m_editor->endFrame();
    }

    UUID NodeEditorAPI::createGraph(const std::string &name) {
        return m_editor->createSubgraphWithUUID(name);
    }

    void NodeEditorAPI::switchToGraph(const UUID &graphId) {
        while (m_editor->getCurrentSubgraphId() >= 0) {
            m_editor->exitSubgraph();
        }
        m_editor->enterSubgraphByUUID(graphId);
    }

    void NodeEditorAPI::enterSubgraph(const UUID &subgraphId) {
        m_editor->enterSubgraphByUUID(subgraphId);
    }

    void NodeEditorAPI::exitSubgraph() {
        m_editor->exitSubgraph();
    }

    std::vector<UUID> NodeEditorAPI::traceConnectionPath(const UUID &startNodeId, const std::string &outputPinName,
                                                         const UUID &endNodeId, const std::string &inputPinName) {
        std::vector<UUID> connectionPath;
        Node *startNode = m_editor->getNodeByUUID(startNodeId);
        Node *endNode = m_editor->getNodeByUUID(endNodeId);
        if (!startNode || !endNode) {
            return connectionPath;
        }

        struct PathSegment {
            UUID nodeUUID;
            std::string pinName;
            bool isInput;
            int subgraphLevel;
            UUID parentNodeUUID;
        };

        std::queue<PathSegment> queue;
        std::unordered_map<std::string, std::vector<UUID> > visitedPaths;
        std::unordered_map<std::string, PathSegment> previousSegment;

        auto createPathKey = [](const UUID &nodeId, const std::string &pinName, bool isInput) {
            return nodeId + ":" + pinName + ":" + (isInput ? "in" : "out");
        };

        queue.push({startNodeId, outputPinName, false, 0, ""});
        std::string startKey = createPathKey(startNodeId, outputPinName, false);
        visitedPaths[startKey] = {};

        std::string targetKey = createPathKey(endNodeId, inputPinName, true);

        bool pathFound = false;

        while (!queue.empty() && !pathFound) {
            PathSegment current = queue.front();
            queue.pop();

            std::string currentKey = createPathKey(current.nodeUUID, current.pinName, current.isInput);

            if (currentKey == targetKey) {
                std::string key = currentKey;
                while (key != startKey) {
                    for (const UUID &connUUID: visitedPaths[key]) {
                        connectionPath.push_back(connUUID);
                    }
                    key = createPathKey(previousSegment[key].nodeUUID, previousSegment[key].pinName,
                                        previousSegment[key].isInput);
                }

                std::reverse(connectionPath.begin(), connectionPath.end());
                pathFound = true;
                break;
            }

            if (m_editor->isSubgraphContainerByUUID(current.nodeUUID)) {
                int nodeId = m_editor->getNodeId(current.nodeUUID);
                int subgraphId = m_editor->getSubgraphFromNode(nodeId);
                if (subgraphId >= 0) {
                    Subgraph *subgraph = m_editor->getSubgraph(subgraphId);
                    if (subgraph) {
                        int interfaceNodeId = -1;
                        std::string interfacePinName;

                        if (current.isInput) {
                            interfaceNodeId = subgraph->metadata.getAttribute<int>("inputNodeId", -1);
                            interfacePinName = current.pinName;
                        } else {
                            interfaceNodeId = subgraph->metadata.getAttribute<int>("outputNodeId", -1);
                            interfacePinName = current.pinName;
                        }

                        if (interfaceNodeId >= 0) {
                            Node *interfaceNode = m_editor->getNode(interfaceNodeId);
                            if (interfaceNode) {
                                UUID interfaceNodeUUID = interfaceNode->uuid;
                                PathSegment nextInside = {
                                    interfaceNodeUUID,
                                    interfacePinName,
                                    !current.isInput,
                                    current.subgraphLevel + 1,
                                    current.nodeUUID
                                };

                                std::string nextKey = createPathKey(nextInside.nodeUUID, nextInside.pinName,
                                                                    nextInside.isInput);
                                if (visitedPaths.find(nextKey) == visitedPaths.end()) {
                                    UUID virtualConnUUID = generateUUID();
                                    visitedPaths[nextKey] = visitedPaths[currentKey];
                                    visitedPaths[nextKey].push_back(virtualConnUUID);
                                    previousSegment[nextKey] = current;
                                    queue.push(nextInside);
                                }
                            }
                        }
                    }
                }
            }

            if (current.isInput) {
                auto connections = m_editor->getInputConnectionsByUUID(current.nodeUUID);
                for (const auto &conn: connections) {
                    UUID nextNodeUUID = conn.sourceNodeUuid;
                    Pin *nextPin = m_editor->getPinByUUID(conn.sourceNodeUuid, conn.sourcePinUuid);
                    bool nextIsInput = false;

                    if (nextPin) {
                        std::string nextPinName = nextPin->name;

                        PathSegment next = {
                            nextNodeUUID,
                            nextPinName,
                            nextIsInput,
                            current.subgraphLevel,
                            ""
                        };

                        std::string nextKey = createPathKey(next.nodeUUID, next.pinName, next.isInput);
                        if (visitedPaths.find(nextKey) == visitedPaths.end()) {
                            visitedPaths[nextKey] = visitedPaths[currentKey];
                            visitedPaths[nextKey].push_back(conn.connectionUuid);
                            previousSegment[nextKey] = current;
                            queue.push(next);
                        }
                    }
                }
            } else {
                auto connections = m_editor->getOutputConnectionsByUUID(current.nodeUUID);
                for (const auto &conn: connections) {
                    UUID nextNodeUUID = conn.targetNodeUuid;
                    Pin *nextPin = m_editor->getPinByUUID(conn.targetNodeUuid, conn.targetPinUuid);
                    bool nextIsInput = true;

                    if (nextPin) {
                        std::string nextPinName = nextPin->name;

                        PathSegment next = {
                            nextNodeUUID,
                            nextPinName,
                            nextIsInput,
                            current.subgraphLevel,
                            ""
                        };

                        std::string nextKey = createPathKey(next.nodeUUID, next.pinName, next.isInput);
                        if (visitedPaths.find(nextKey) == visitedPaths.end()) {
                            visitedPaths[nextKey] = visitedPaths[currentKey];
                            visitedPaths[nextKey].push_back(conn.connectionUuid);
                            previousSegment[nextKey] = current;
                            queue.push(next);
                        }
                    }
                }
            }

            if (current.subgraphLevel > 0 && !current.parentNodeUUID.empty()) {
                bool isInterfaceNode = false;
                bool isInputNode = false;

                int currentSubgraphId = m_editor->getCurrentSubgraphId();
                if (currentSubgraphId >= 0) {
                    Subgraph *subgraph = m_editor->getSubgraph(currentSubgraphId);
                    if (subgraph) {
                        int currentNodeId = m_editor->getNodeId(current.nodeUUID);
                        isInputNode = (subgraph->metadata.getAttribute<int>("inputNodeId", -1) == currentNodeId);
                        bool isOutputNode = (subgraph->metadata.getAttribute<int>("outputNodeId", -1) == currentNodeId);
                        isInterfaceNode = isInputNode || isOutputNode;
                    }
                }

                if (isInterfaceNode) {
                    PathSegment parentSegment = {
                        current.parentNodeUUID,
                        current.pinName,
                        isInputNode ? false : true,
                        current.subgraphLevel - 1,
                        ""
                    };

                    std::string parentKey = createPathKey(parentSegment.nodeUUID, parentSegment.pinName,
                                                          parentSegment.isInput);
                    if (visitedPaths.find(parentKey) == visitedPaths.end()) {
                        UUID virtualConnUUID = generateUUID();
                        visitedPaths[parentKey] = visitedPaths[currentKey];
                        visitedPaths[parentKey].push_back(virtualConnUUID);
                        previousSegment[parentKey] = current;
                        queue.push(parentSegment);
                    }
                }
            }
        }

        return connectionPath;
    }

    void NodeEditorAPI::centerView() {
        m_editor->centerView();
    }

    void NodeEditorAPI::centerOnNode(const UUID &nodeId) {
        m_editor->centerOnNodeByUUID(nodeId);
    }

    void NodeEditorAPI::zoomToFit(float padding) {
        try {
            if (m_editor->getViewManager().getWindowSize().x <= 0 ||
                m_editor->getViewManager().getWindowSize().y <= 0) {
                m_editor->getViewManager().setWindowSize(Vec2(1280, 720));
            }
            m_editor->zoomToFit(padding);
        } catch (const std::exception &e) {
            std::cerr << "Exception in zoomToFit: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "Unknown exception in zoomToFit" << std::endl;
        }
    }

    void NodeEditorAPI::zoomToFitSelected(float padding) {
        m_editor->zoomToFitSelected(padding);
    }

    void NodeEditorAPI::smoothCenterView(float duration) {
        m_editor->smoothCenterView(duration);
    }

    void NodeEditorAPI::smoothCenterOnNode(const UUID &nodeId, float duration) {
        m_editor->smoothCenterOnNodeByUUID(nodeId, duration);
    }

    void NodeEditorAPI::setViewPosition(const Vec2 &position) {
        m_editor->setViewPosition(position);
    }

    Vec2 NodeEditorAPI::getViewPosition() const {
        return m_editor->getViewPosition();
    }

    void NodeEditorAPI::setViewScale(float scale) {
        m_editor->setViewScale(scale);
    }

    float NodeEditorAPI::getViewScale() const {
        return m_editor->getViewScale();
    }

    UUID NodeEditorAPI::createNode(const std::string &type, const std::string &name, const Vec2 &position) {
        UUID nodeId = createNodeWithPins(type, name, position);

        int currentSubgraphId = m_editor->getCurrentSubgraphId();

        if (currentSubgraphId >= 0) {
            int nodeNumId = m_editor->getNodeId(nodeId);
            m_editor->addNodeToSubgraph(nodeNumId, currentSubgraphId);
        }

        return nodeId;
    }

    UUID NodeEditorAPI::createNodeWithPins(const std::string &type, const std::string &name, const Vec2 &position) {
        UUID nodeId = m_editor->addNodeWithUUID(name, type, position);

        if (m_nodeDefinitions.find(type) != m_nodeDefinitions.end()) {
            const auto &definition = m_nodeDefinitions[type];

            for (const auto &input: definition.inputs) {
                addPinToNode(nodeId, input.first, true, input.second);
            }

            for (const auto &output: definition.outputs) {
                addPinToNode(nodeId, output.first, false, output.second);
            }
        }

        return nodeId;
    }

    UUID NodeEditorAPI::addPinToNode(const UUID &nodeId, const std::string &name, bool isInput, PinType type) {
        return m_editor->addPinWithUUIDByNodeUUID(nodeId, name, isInput, type);
    }

    void NodeEditorAPI::removeNode(const UUID &nodeId) {
        m_editor->removeNodeByUUID(nodeId);
    }

    void NodeEditorAPI::selectNode(const UUID &nodeId, bool append) {
        m_editor->selectNodeByUUID(nodeId, append);
    }

    void NodeEditorAPI::deselectNode(const UUID &nodeId) {
        m_editor->deselectNode(m_editor->getNodeId(nodeId));
    }

    void NodeEditorAPI::deselectAllNodes() {
        m_editor->deselectAllNodes();
    }

    std::vector<UUID> NodeEditorAPI::getSelectedNodes() const {
        return m_editor->getSelectedNodeUUIDs();
    }

    int NodeEditorAPI::findPinIdByName(const UUID &nodeId, const std::string &pinName, bool isInput) {
        Node *node = m_editor->getNodeByUUID(nodeId);
        if (!node) {
            return -1;
        }

        const auto &pins = isInput ? node->inputs : node->outputs;

        for (const auto &pin: pins) {
            if (pin.name == pinName) {
                return pin.id;
            }
        }

        return -1;
    }

    UUID NodeEditorAPI::connectNodes(const UUID& startNodeId, const std::string& outputPinName,
                                     const UUID& endNodeId, const std::string& inputPinName) {
        int startNodeRealId = m_editor->getNodeId(startNodeId);
        int endNodeRealId = m_editor->getNodeId(endNodeId);

        if (startNodeRealId == -1 || endNodeRealId == -1) {
            return "";
        }

        int startPinId = findPinIdByName(startNodeId, outputPinName, false);
        int endPinId = findPinIdByName(endNodeId, inputPinName, true);

        if (startPinId == -1 || endPinId == -1) {
            return "";
        }

        UUID connectionUuid = m_editor->addConnectionWithUUID(startNodeRealId, startPinId, endNodeRealId, endPinId);
        return connectionUuid;
    }

    UUID NodeEditorAPI::addRerouteToConnection(const UUID& connectionId, const Vec2& position) {
        int connectionRealId = m_editor->getConnectionId(connectionId);
        if (connectionRealId == -1) {
            return "";
        }
        
        int rerouteId = m_editor->addReroute(connectionRealId, position, -1);
        if (rerouteId == -1) {
            return "";
        }
        
        const Reroute* reroute = m_editor->getReroute(rerouteId);
        if (!reroute) {
            return "";
        }
        
        return reroute->uuid;
    }

    bool NodeEditorAPI::disconnectNodes(const UUID& connectionId) {
        int connectionRealId = m_editor->getConnectionId(connectionId);
        if (connectionRealId == -1) {
            return false;
        }

        m_editor->removeConnection(connectionRealId);
        return true;
    }

    UUID NodeEditorAPI::createGroup(const std::string &name, const Vec2 &position, const Vec2 &size) {
        return m_editor->addGroupWithUUID(name, position, size);
    }

    void NodeEditorAPI::addNodeToGroup(const UUID &nodeId, const UUID &groupId) {
        m_editor->addNodeToGroupByUUID(nodeId, groupId);
    }

    void NodeEditorAPI::removeNodeFromGroup(const UUID &nodeId, const UUID &groupId) {
        m_editor->removeNodeFromGroup(m_editor->getNodeId(nodeId), m_editor->getGroupId(groupId));
    }

    void NodeEditorAPI::registerEvaluator(const std::string &nodeType,
                                          std::function<std::any(const std::vector<std::any> &)> evaluator) {
        m_evaluators[nodeType] = std::move(evaluator);
    }

    NodeEditorAPI::EvaluationResult NodeEditorAPI::evaluateGraph(const UUID &outputNodeId) {
        EvaluationResult result;

        std::vector<UUID> evaluationOrderUUIDs = m_editor->getEvaluationOrderUUIDs();
        result.evaluationOrder = evaluationOrderUUIDs;

        std::unordered_map<UUID, std::any> nodeValues;

        for (const auto &nodeUUID: evaluationOrderUUIDs) {
            Node *node = m_editor->getNodeByUUID(nodeUUID);
            if (!node) continue;

            std::vector<std::any> inputValues;

            for (size_t i = 0; i < node->inputs.size(); i++) {
                const Pin &pin = node->inputs[i];
                std::any pinValue;
                bool valueFound = false;

                auto connections = m_editor->getInputConnectionsByUUID(nodeUUID);

                for (const auto &conn: connections) {
                    if (conn.targetPinId == pin.id) {
                        if (nodeValues.count(conn.sourceNodeUuid)) {
                            pinValue = nodeValues[conn.sourceNodeUuid];
                            valueFound = true;
                        }
                        break;
                    }
                }

                inputValues.push_back(pinValue);
            }

            std::any nodeResult;

            if (m_constantValues.count(nodeUUID)) {
                nodeResult = m_constantValues[nodeUUID];
            } else if (m_evaluators.count(node->type)) {
                nodeResult = m_evaluators[node->type](inputValues);
            } else {
                nodeResult = std::any();
            }

            nodeValues[nodeUUID] = nodeResult;
        }

        if (!outputNodeId.empty() && nodeValues.count(outputNodeId)) {
            result.value = nodeValues[outputNodeId];
        } else if (!evaluationOrderUUIDs.empty()) {
            result.value = nodeValues[evaluationOrderUUIDs.back()];
        }

        return result;
    }

    void NodeEditorAPI::setConstantValue(const UUID &nodeId, const std::any &value) {
        m_constantValues[nodeId] = value;
    }

    std::any NodeEditorAPI::getConstantValue(const UUID &nodeId) const {
        if (m_constantValues.count(nodeId)) {
            return m_constantValues.at(nodeId);
        }
        return std::any();
    }

    void NodeEditorAPI::setNodeCreatedCallback(std::function<void(const UUID &)> callback) {
        m_editor->setNodeCreatedCallback([callback](int, const UUID &uuid) {
            callback(uuid);
        });
    }

    void NodeEditorAPI::setNodeRemovedCallback(std::function<void(const UUID &)> callback) {
        m_editor->setNodeRemovedCallback([callback](int, const UUID &uuid) {
            callback(uuid);
        });
    }

    void NodeEditorAPI::setConnectionCreatedCallback(std::function<void(const UUID &)> callback) {
        m_editor->setConnectionCreatedCallback([callback](int, const UUID &uuid) {
            callback(uuid);
        });
    }

    void NodeEditorAPI::setConnectionRemovedCallback(std::function<void(const UUID &)> callback) {
        m_editor->setConnectionRemovedCallback([callback](int, const UUID &uuid) {
            callback(uuid);
        });
    }

    void NodeEditorAPI::executeCommand(const std::string &command, const std::any &data) {
        m_editor->dispatchToBackend(command, data);
    }

    NodeEditor *NodeEditorAPI::getUnderlyingEditor() {
        return m_editor.get();
    }
}
