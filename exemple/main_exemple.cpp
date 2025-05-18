#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <iostream>
#include <exception>
#include <unordered_map>
#include <stack>
#include <queue>
#include <any>
#include <variant>

#include "../AdvancedNodeEditor/NodeEditor.h"

using namespace NodeEditorCore;

struct NodeValue {
    std::variant<float, std::string> value;

    NodeValue() : value(0.0f) {}
    NodeValue(float v) : value(v) {}
    NodeValue(const std::string& v) : value(v) {}

    bool isNumeric() const { return std::holds_alternative<float>(value); }
    float getNumeric() const { return isNumeric() ? std::get<float>(value) : 0.0f; }
    std::string getString() const { return isNumeric() ? std::to_string(getNumeric()) : std::get<std::string>(value); }
};

struct NodeDefinition {
    std::string type;
    std::string name;
    std::vector<std::pair<std::string, PinType>> inputs;
    std::vector<std::pair<std::string, PinType>> outputs;
    std::string iconSymbol;
    std::function<NodeValue(const std::vector<NodeValue>&)> evaluator;
};

static NodeDefinition GetNodeDefByType(const std::string& type) {
    static std::unordered_map<std::string, NodeDefinition> nodeDefs = {
        {"Math.Add", {
            "Math.Add", "Add",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}, "+",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                if (inputs.size() < 2) return 0.0f;
                return inputs[0].getNumeric() + inputs[1].getNumeric();
            }
        }},
        {"Math.Multiply", {
            "Math.Multiply", "Multiply",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}, "*",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                if (inputs.size() < 2) return 0.0f;
                return inputs[0].getNumeric() * inputs[1].getNumeric();
            }
        }},
        {"Math.Subtract", {
            "Math.Subtract", "Subtract",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}, "-",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                if (inputs.size() < 2) return 0.0f;
                return inputs[0].getNumeric() - inputs[1].getNumeric();
            }
        }},
        {"Math.Divide", {
            "Math.Divide", "Divide",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}, "/",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                if (inputs.size() < 2) return 0.0f;
                if (inputs[1].getNumeric() == 0.0f) return 0.0f;
                return inputs[0].getNumeric() / inputs[1].getNumeric();
            }
        }},
        {"Math.Constant", {
            "Math.Constant", "Constant",
            {},
            {{"Value", PinType::Blue}}, "C",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                return 5.0f;
            }
        }},
        {"Geometry.Box", {
            "Geometry.Box", "Box",
            {{"Size", PinType::Green}, {"Center", PinType::Green}},
            {{"Geometry", PinType::Red}}, "■",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                return NodeValue("Box");
            }
        }},
        {"Geometry.Sphere", {
            "Geometry.Sphere", "Sphere",
            {{"Radius", PinType::Green}, {"Center", PinType::Green}},
            {{"Geometry", PinType::Red}}, "●",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                return NodeValue("Sphere");
            }
        }},
        {"Material.Basic", {
            "Material.Basic", "Material",
            {{"Color", PinType::Purple}, {"Roughness", PinType::Blue}},
            {{"Material", PinType::Yellow}}, "M",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                return NodeValue("Material");
            }
        }},
        {"Render.MeshRenderer", {
            "Render.MeshRenderer", "Renderer",
            {{"Geometry", PinType::Red}, {"Material", PinType::Yellow}},
            {{"Output", PinType::Cyan}}, "R",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                return NodeValue("Rendered");
            }
        }},
        {"Subgraph.Input", {
            "Subgraph.Input", "Input",
            {},
            {{"Value", PinType::Blue}}, "I",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                return inputs.empty() ? NodeValue(1.0f) : inputs[0];
            }
        }},
        {"Subgraph.Output", {
            "Subgraph.Output", "Output",
            {{"Value", PinType::Blue}},
            {}, "O",
            [](const std::vector<NodeValue>& inputs) -> NodeValue {
                return inputs.empty() ? NodeValue(0.0f) : inputs[0];
            }
        }}
    };

    if (nodeDefs.find(type) == nodeDefs.end()) {
        throw std::runtime_error("Unknown node type: " + type);
    }
    return nodeDefs[type];
}

Node* CreateNodeOfType(const std::string& type, const Vec2& pos) {
    static int nextId = 1;
    NodeDefinition def = GetNodeDefByType(type);

    Node* node = nullptr;
    try {
        node = new Node(nextId++, def.name, type, pos);
    } catch (const std::exception& e) {
        throw;
    }
    node->iconSymbol = def.iconSymbol;

    static int globalPinId = 1;

    for (const auto& input : def.inputs) {
        node->inputs.push_back(Pin(globalPinId++, input.first, true, input.second));
    }

    for (const auto& output : def.outputs) {
        node->outputs.push_back(Pin(globalPinId++, output.first, false, output.second));
    }

    std::cout << "Node created: " << node->name << " (Type: " << node->type << ")" << std::endl;
    std::cout << "Input pins: " << node->inputs.size() << ", Output pins: " << node->outputs.size() << std::endl;

    return node;
}

class GraphEvaluator {
public:
    GraphEvaluator(const NodeEditor& editor) : m_editor(editor) {}

    std::vector<int> computeEvaluationOrder(int outputNodeId) {
        const auto& nodes = m_editor.getNodes();
        const auto& connections = m_editor.getConnections();

        std::unordered_map<int, std::vector<int>> dependencyGraph;
        std::unordered_set<int> visited;
        std::vector<int> evaluationOrder;

        for (const auto& connection : connections) {
            dependencyGraph[connection.endNodeId].push_back(connection.startNodeId);
        }

        std::function<void(int)> dfs = [&](int nodeId) {
            if (visited.count(nodeId)) {
                return;
            }

            visited.insert(nodeId);

            if (dependencyGraph.count(nodeId)) {
                for (int dependentNodeId : dependencyGraph[nodeId]) {
                    dfs(dependentNodeId);
                }
            }

            evaluationOrder.push_back(nodeId);
        };

        if (outputNodeId != -1) {
            dfs(outputNodeId);
        } else {
            std::unordered_set<int> hasOutput;
            for (const auto& connection : connections) {
                hasOutput.insert(connection.startNodeId);
            }

            for (const auto& node : nodes) {
                if (!hasOutput.count(node.id)) {
                    dfs(node.id);
                }
            }
        }

        return evaluationOrder;
    }

    NodeValue evaluateGraph(int outputNodeId = -1) {
        const auto& nodes = m_editor.getNodes();
        const auto& connections = m_editor.getConnections();

        std::vector<int> evaluationOrder = computeEvaluationOrder(outputNodeId);
        std::unordered_map<int, NodeValue> nodeValues;
        std::unordered_map<int, std::unordered_map<int, int>> connectionMap;

        for (const auto& connection : connections) {
            connectionMap[connection.endNodeId][connection.endPinId] =
                (connection.startNodeId << 16) | connection.startPinId;
        }

        for (int nodeId : evaluationOrder) {
            const Node* node = m_editor.getNode(nodeId);
            if (!node) continue;

            std::vector<NodeValue> inputValues;

            for (const auto& pin : node->inputs) {
                if (connectionMap.count(nodeId) && connectionMap[nodeId].count(pin.id)) {
                    int sourceInfo = connectionMap[nodeId][pin.id];
                    int sourceNodeId = sourceInfo >> 16;
                    int sourcePinId = sourceInfo & 0xFFFF;

                    if (nodeValues.count(sourceNodeId)) {
                        inputValues.push_back(nodeValues[sourceNodeId]);
                    } else {
                        inputValues.push_back(NodeValue(0.0f));
                    }
                } else {
                    inputValues.push_back(NodeValue(0.0f));
                }
            }

            NodeDefinition def = GetNodeDefByType(node->type);
            NodeValue result = def.evaluator(inputValues);
            nodeValues[nodeId] = result;
        }

        if (outputNodeId != -1 && nodeValues.count(outputNodeId)) {
            return nodeValues[outputNodeId];
        } else if (!evaluationOrder.empty() && nodeValues.count(evaluationOrder.back())) {
            return nodeValues[evaluationOrder.back()];
        }

        return NodeValue(0.0f);
    }

private:
    const NodeEditor& m_editor;
};

int main(int argc, char* argv[]) {
    try {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            return -1;
        }

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        SDL_Window* window = SDL_CreateWindow("Advanced Node Editor - Math Graph Evaluator",
                                             SDL_WINDOWPOS_CENTERED,
                                             SDL_WINDOWPOS_CENTERED,
                                             1280, 720,
                                             window_flags);
        if (!window) {
            SDL_Quit();
            return -2;
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -3;
        }

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        NodeEditor editor;

        editor.registerNodeType("Math.Add", "Math", "Addition node",
                               [](const Vec2& pos) { return CreateNodeOfType("Math.Add", pos); });

        editor.registerNodeType("Math.Multiply", "Math", "Multiplication node",
                               [](const Vec2& pos) { return CreateNodeOfType("Math.Multiply", pos); });

        editor.registerNodeType("Math.Subtract", "Math", "Subtraction node",
                               [](const Vec2& pos) { return CreateNodeOfType("Math.Subtract", pos); });

        editor.registerNodeType("Math.Divide", "Math", "Division node",
                               [](const Vec2& pos) { return CreateNodeOfType("Math.Divide", pos); });

        editor.registerNodeType("Math.Constant", "Math", "Constant value",
                               [](const Vec2& pos) { return CreateNodeOfType("Math.Constant", pos); });

        editor.registerNodeType("Geometry.Box", "Geometry", "Create a box geometry",
                               [](const Vec2& pos) { return CreateNodeOfType("Geometry.Box", pos); });

        editor.registerNodeType("Geometry.Sphere", "Geometry", "Create a sphere geometry",
                               [](const Vec2& pos) { return CreateNodeOfType("Geometry.Sphere", pos); });

        editor.registerNodeType("Material.Basic", "Material", "Create a basic material",
                               [](const Vec2& pos) { return CreateNodeOfType("Material.Basic", pos); });

        editor.registerNodeType("Render.MeshRenderer", "Render", "Render a mesh with material",
                               [](const Vec2& pos) { return CreateNodeOfType("Render.MeshRenderer", pos); });

        editor.registerNodeType("Subgraph.Input", "Subgraph", "Subgraph input port",
                               [](const Vec2& pos) { return CreateNodeOfType("Subgraph.Input", pos); });

        editor.registerNodeType("Subgraph.Output", "Subgraph", "Subgraph output port",
                               [](const Vec2& pos) { return CreateNodeOfType("Subgraph.Output", pos); });

        editor.setGraphTitle("Math Node Graph Evaluator");
        editor.setGraphTitleStyle(NodeEditor::TitleStyle::Houdini);
        editor.setConnectionStyle(NodeEditor::ConnectionStyle::Bezier);
        editor.setConnectionThickness(2.5f);
        editor.enableNodeAvoidance(true);
        editor.enableMinimap(true);
        editor.activateAllConnectionFlows(false, 0.0f);
        editor.setShowSubgraphBreadcrumbs(true);

        UUID mainGraphUuid = editor.createSubgraphWithUUID("Main");
        UUID mathGraphUuid = editor.createSubgraphWithUUID("Math Graph");

        editor.enterSubgraphByUUID(mathGraphUuid);

        int const1NodeId = editor.addNode("Value A", "Math.Constant", Vec2(300, 100));
        int const1OutputPinId = editor.addPin(const1NodeId, "Value", false, PinType::Blue);
        Node* const1Node = editor.getNode(const1NodeId);
        std::cout << "Constant1 node created: " << const1NodeId << ", output pin added: " << const1OutputPinId << std::endl;

        int const2NodeId = editor.addNode("Value B", "Math.Constant", Vec2(300, 200));
        int const2OutputPinId = editor.addPin(const2NodeId, "Value", false, PinType::Blue);
        Node* const2Node = editor.getNode(const2NodeId);
        std::cout << "Constant2 node created: " << const2NodeId << ", output pin added: " << const2OutputPinId << std::endl;

        int const3NodeId = editor.addNode("Value C", "Math.Constant", Vec2(300, 300));
        int const3OutputPinId = editor.addPin(const3NodeId, "Value", false, PinType::Blue);
        Node* const3Node = editor.getNode(const3NodeId);
        std::cout << "Constant3 node created: " << const3NodeId << ", output pin added: " << const3OutputPinId << std::endl;

        int addNodeId = editor.addNode("Add", "Math.Add", Vec2(500, 150));
        int addInputPin1Id = editor.addPin(addNodeId, "A", true, PinType::Blue);
        int addInputPin2Id = editor.addPin(addNodeId, "B", true, PinType::Blue);
        int addOutputPinId = editor.addPin(addNodeId, "Result", false, PinType::Blue);
        Node* addNode = editor.getNode(addNodeId);
        std::cout << "Add node created: " << addNodeId << ", input pins added: " << addInputPin1Id << ", "
                 << addInputPin2Id << ", output pin added: " << addOutputPinId << std::endl;

        int multiplyNodeId = editor.addNode("Multiply", "Math.Multiply", Vec2(500, 300));
        int multiplyInputPin1Id = editor.addPin(multiplyNodeId, "A", true, PinType::Blue);
        int multiplyInputPin2Id = editor.addPin(multiplyNodeId, "B", true, PinType::Blue);
        int multiplyOutputPinId = editor.addPin(multiplyNodeId, "Result", false, PinType::Blue);
        Node* multiplyNode = editor.getNode(multiplyNodeId);
        std::cout << "Multiply node created: " << multiplyNodeId << ", input pins added: " << multiplyInputPin1Id
                 << ", " << multiplyInputPin2Id << ", output pin added: " << multiplyOutputPinId << std::endl;

        int subtractNodeId = editor.addNode("Subtract", "Math.Subtract", Vec2(700, 200));
        int subtractInputPin1Id = editor.addPin(subtractNodeId, "A", true, PinType::Blue);
        int subtractInputPin2Id = editor.addPin(subtractNodeId, "B", true, PinType::Blue);
        int subtractOutputPinId = editor.addPin(subtractNodeId, "Result", false, PinType::Blue);
        Node* subtractNode = editor.getNode(subtractNodeId);
        std::cout << "Subtract node created: " << subtractNodeId << ", input pins added: " << subtractInputPin1Id
                 << ", " << subtractInputPin2Id << ", output pin added: " << subtractOutputPinId << std::endl;

        int outputNodeId = editor.addNode("Output", "Default", Vec2(900, 200));
        int outputPinId = editor.addPin(outputNodeId, "Value", true, PinType::Blue);
        Node* outputNode = editor.getNode(outputNodeId);
        std::cout << "Output node created: " << outputNodeId << ", input pin added: " << outputPinId << std::endl;

        if (outputNode) {
            outputNode->metadata.setAttribute("protected", true);
            std::cout << "Output node marked as protected" << std::endl;
        }

        if (const1Node && !const1Node->outputs.empty() && addNode && !addNode->inputs.empty()) {
            editor.addConnection(const1NodeId, const1Node->outputs[0].id, addNodeId, addNode->inputs[0].id);
        }
        if (const2Node && !const2Node->outputs.empty() && addNode && addNode->inputs.size() >= 2) {
            editor.addConnection(const2NodeId, const2Node->outputs[0].id, addNodeId, addNode->inputs[1].id);
        }

        try {
            editor.addConnection(addNodeId, addOutputPinId, multiplyNodeId, multiplyInputPin1Id);
            std::cout << "Connection created between Add (" << addOutputPinId << ") and Multiply.A (" << multiplyInputPin1Id << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error connecting Add->Multiply: " << e.what() << std::endl;
        }

        try {
            editor.addConnection(const3NodeId, const3OutputPinId, multiplyNodeId, multiplyInputPin2Id);
            std::cout << "Connection created between Constant3 (" << const3OutputPinId << ") and Multiply.B (" << multiplyInputPin2Id << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error connecting Constant3->Multiply: " << e.what() << std::endl;
        }

        try {
            editor.addConnection(multiplyNodeId, multiplyOutputPinId, subtractNodeId, subtractInputPin1Id);
            std::cout << "Connection created between Multiply (" << multiplyOutputPinId << ") and Subtract.A (" << subtractInputPin1Id << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error connecting Multiply->Subtract: " << e.what() << std::endl;
        }

        try {
            editor.addConnection(const1NodeId, const1OutputPinId, subtractNodeId, subtractInputPin2Id);
            std::cout << "Connection created between Constant1 (" << const1OutputPinId << ") and Subtract.B (" << subtractInputPin2Id << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error connecting Constant1->Subtract: " << e.what() << std::endl;
        }

        try {
            editor.addConnection(subtractNodeId, subtractOutputPinId, outputNodeId, outputPinId);
            std::cout << "Connection created between Subtract (" << subtractOutputPinId << ") and Output (" << outputPinId << ")" << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error connecting Subtract->Output: " << e.what() << std::endl;
        }

        auto mathSubgraph = editor.getSubgraph(editor.getSubgraphId(mathGraphUuid));
        if (mathSubgraph) {
            mathSubgraph->interfaceOutputs.push_back((outputNodeId << 16) | outputPinId);
        }

        editor.exitSubgraph();

        Node* mathGraphNode = editor.createSubgraphNode(editor.getSubgraphId(mathGraphUuid),
                                                      "Math Calculation",
                                                      Vec2(300, 175));
        int mathGraphNodeId = mathGraphNode ? mathGraphNode->id : -1;

        bool showEvaluationWindow = true;
        NodeValue evaluationResult;
        std::vector<int> evaluationOrder;
        std::unordered_map<int, std::string> nodeNames;

        std::unordered_map<int, float> constantValues;
        constantValues[const1NodeId] = 5.0f;
        constantValues[const2NodeId] = 3.0f;
        constantValues[const3NodeId] = 2.0f;

        class CustomizedGraphEvaluator {
        public:
            CustomizedGraphEvaluator(const NodeEditor& editor,
                                  const std::unordered_map<int, float>& constantValues)
                : m_editor(editor), m_constantValues(constantValues), m_baseEvaluator(editor) {}

            std::vector<int> computeEvaluationOrder(int outputNodeId) {
                return m_baseEvaluator.computeEvaluationOrder(outputNodeId);
            }

            NodeValue evaluateGraph(int outputNodeId = -1) {
                try {
                    const auto& nodes = m_editor.getNodes();
                    const auto& connections = m_editor.getConnections();

                    std::vector<int> evaluationOrder = computeEvaluationOrder(outputNodeId);
                    std::unordered_map<int, NodeValue> nodeValues;
                    std::unordered_map<int, std::unordered_map<int, int>> connectionMap;

                    std::cout << "Evaluation order: ";
                    for (int id : evaluationOrder) {
                        std::cout << id << " ";
                    }
                    std::cout << std::endl;

                    for (const auto& connection : connections) {
                        connectionMap[connection.endNodeId][connection.endPinId] =
                            (connection.startNodeId << 16) | connection.startPinId;
                        std::cout << "Connection: " << connection.startNodeId << ":" << connection.startPinId
                                << " -> " << connection.endNodeId << ":" << connection.endPinId << std::endl;
                    }

                    for (int nodeId : evaluationOrder) {
                        const Node* node = m_editor.getNode(nodeId);
                        if (!node) {
                            std::cout << "Node " << nodeId << " not found, skipped" << std::endl;
                            continue;
                        }

                        std::cout << "Begin evaluation of node " << nodeId << " (" << node->name << ", type: " << node->type << ")" << std::endl;

                        std::vector<NodeValue> inputValues;

                        for (const auto& pin : node->inputs) {
                            if (connectionMap.count(nodeId) && connectionMap[nodeId].count(pin.id)) {
                                int sourceInfo = connectionMap[nodeId][pin.id];
                                int sourceNodeId = sourceInfo >> 16;
                                int sourcePinId = sourceInfo & 0xFFFF;

                                std::cout << "  Pin " << pin.id << " connected to node " << sourceNodeId << " pin " << sourcePinId << std::endl;

                                if (nodeValues.count(sourceNodeId)) {
                                    inputValues.push_back(nodeValues[sourceNodeId]);
                                    std::cout << "  Input value: " << nodeValues[sourceNodeId].getString() << std::endl;
                                } else {
                                    inputValues.push_back(NodeValue(0.0f));
                                    std::cout << "  Source node not evaluated, using default 0.0" << std::endl;
                                }
                            } else {
                                inputValues.push_back(NodeValue(0.0f));
                                std::cout << "  Pin " << pin.id << " not connected, using default 0.0" << std::endl;
                            }
                        }

                        NodeValue result;

                        if (node->type == "Math.Constant" && m_constantValues.count(nodeId)) {
                            result = NodeValue(m_constantValues.at(nodeId));
                            std::cout << "  Constant node, set value: " << m_constantValues.at(nodeId) << std::endl;
                        }
                        else if (node->type == "Default") {
                            if (!inputValues.empty()) {
                                result = inputValues[0];
                                std::cout << "  Default node, passing input value: " << inputValues[0].getString() << std::endl;
                            } else {
                                result = NodeValue(0.0f);
                                std::cout << "  Default node with no inputs, default value: 0" << std::endl;
                            }
                        }
                        else {
                            try {
                                NodeDefinition def = GetNodeDefByType(node->type);
                                result = def.evaluator(inputValues);
                                std::cout << "  Standard evaluation" << std::endl;
                            } catch (const std::exception& e) {
                                std::cout << "  Evaluation ERROR: " << e.what() << std::endl;
                                result = NodeValue(0.0f);
                            }
                        }

                        nodeValues[nodeId] = result;

                        std::cout << "Evaluation of node " << nodeId << " (" << node->name << "): ";
                        if (result.isNumeric()) {
                            std::cout << result.getNumeric() << std::endl;
                        } else {
                            std::cout << result.getString() << std::endl;
                        }
                    }

                    if (outputNodeId != -1 && nodeValues.count(outputNodeId)) {
                        std::cout << "Returning value of specified output node: " << outputNodeId << std::endl;
                        return nodeValues[outputNodeId];
                    } else if (!evaluationOrder.empty() && nodeValues.count(evaluationOrder.back())) {
                        std::cout << "Returning value of last evaluated node: " << evaluationOrder.back() << std::endl;
                        return nodeValues[evaluationOrder.back()];
                    }

                    std::cout << "No value found, returning 0.0" << std::endl;
                    return NodeValue(0.0f);
                } catch (const std::exception& e) {
                    std::cerr << "Exception in evaluateGraph: " << e.what() << std::endl;
                    return NodeValue(0.0f);
                } catch (...) {
                    std::cerr << "Unknown exception in evaluateGraph" << std::endl;
                    return NodeValue(0.0f);
                }
            }

        private:
            const NodeEditor& m_editor;
            const std::unordered_map<int, float>& m_constantValues;
            GraphEvaluator m_baseEvaluator;
        };

        CustomizedGraphEvaluator evaluator(editor, constantValues);

        bool done = false;
        while (!done) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT)
                    done = true;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(window))
                    done = true;
            }

            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("New Graph")) { }
                    if (ImGui::MenuItem("Open Graph...")) { }
                    if (ImGui::MenuItem("Save Graph")) { }
                    if (ImGui::MenuItem("Save Graph As...")) { }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit")) { done = true; }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit")) {
                    if (ImGui::MenuItem("Undo", "CTRL+Z")) { }
                    if (ImGui::MenuItem("Redo", "CTRL+Y")) { }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "CTRL+X")) { }
                    if (ImGui::MenuItem("Copy", "CTRL+C")) { }
                    if (ImGui::MenuItem("Paste", "CTRL+V")) { }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View")) {
                    if (ImGui::MenuItem("Zoom to Fit", "F")) { editor.zoomToFit(); }
                    if (ImGui::MenuItem("Center View", "C")) { editor.centerView(); }
                    ImGui::Separator();
                    bool showMinimap = editor.isMinimapEnabled();
                    if (ImGui::MenuItem("Minimap", nullptr, &showMinimap)) {
                        editor.enableMinimap(showMinimap);
                    }
                    ImGui::MenuItem("Show Evaluation Window", nullptr, &showEvaluationWindow);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Evaluate")) {
                    if (ImGui::MenuItem("Evaluate Graph")) {
                        editor.enterSubgraphByUUID(mathGraphUuid);

                        evaluationResult = evaluator.evaluateGraph(outputNodeId);
                        evaluationOrder = evaluator.computeEvaluationOrder(outputNodeId);

                        nodeNames.clear();
                        for (int id : evaluationOrder) {
                            const Node* node = editor.getNode(id);
                            if (node) {
                                nodeNames[id] = node->name;
                            }
                        }

                        editor.exitSubgraph();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Subgraph")) {
                    if (ImGui::MenuItem("Main Graph")) {
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                    }
                    if (ImGui::MenuItem("Math Graph")) {
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        editor.enterSubgraphByUUID(mathGraphUuid);
                    }
                    ImGui::EndMenu();
                }

                ImGui::EndMainMenuBar();
            }

            ImVec2 totalSize = ImGui::GetIO().DisplaySize;
            ImVec2 menuBarSize = ImGui::GetItemRectSize();
            float panelWidth = 300.0f;
            float editorWidth = totalSize.x - panelWidth;
            float panelHeight = totalSize.y - menuBarSize.y;

            ImGui::SetNextWindowPos(ImVec2(0, menuBarSize.y));
            ImGui::SetNextWindowSize(ImVec2(editorWidth, panelHeight));
            ImGui::Begin("Node Editor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImGui::BeginChild("SubgraphNavigation", ImVec2(ImGui::GetContentRegionAvail().x, 30), true);

            if (ImGui::Button("Main Graph")) {
                while (editor.getCurrentSubgraphId() >= 0) {
                    editor.exitSubgraph();
                }
            }

            ImGui::SameLine();
            if (editor.getCurrentSubgraphId() >= 0) {
                if (ImGui::Button("Exit Subgraph")) {
                    editor.exitSubgraph();
                }

                ImGui::SameLine();
                auto currentSubgraph = editor.getSubgraph(editor.getCurrentSubgraphId());
                if (currentSubgraph) {
                    ImGui::Text("Current: %s", currentSubgraph->name.c_str());
                } else {
                    ImGui::Text("Current: [ERROR: null subgraph]");
                }
            }

            ImGui::EndChild();

            try {
                editor.beginFrame();
                editor.render();
                editor.endFrame();
            } catch (const std::exception& e) {
            }

            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(editorWidth, menuBarSize.y));
            ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));
            ImGui::Begin("Side Panel", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            if (ImGui::BeginTabBar("SidePanelTabs")) {
                if (ImGui::BeginTabItem("Subgraphs")) {
                    ImGui::Text("Subgraph Navigator");
                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Available Subgraphs", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Main Graph")) {
                            while (editor.getCurrentSubgraphId() >= 0) {
                                editor.exitSubgraph();
                            }
                        }

                        if (ImGui::Button("Math Graph")) {
                            while (editor.getCurrentSubgraphId() >= 0) {
                                editor.exitSubgraph();
                            }
                            editor.enterSubgraphByUUID(mathGraphUuid);
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Properties")) {
                    ImGui::Text("Node Properties");
                    ImGui::Separator();

                    std::vector<int> selectedNodes = editor.getSelectedNodes();
                    if (!selectedNodes.empty()) {
                        const Node* node = editor.getNode(selectedNodes[0]);
                        if (node) {
                            ImGui::Text("Node: %s", node->name.c_str());
                            ImGui::Text("Type: %s", node->type.c_str());
                            ImGui::Separator();

                            if (node->type == "Math.Constant") {
                                if (constantValues.find(selectedNodes[0]) == constantValues.end()) {
                                    constantValues[selectedNodes[0]] = 0.0f;
                                }

                                float& value = constantValues[selectedNodes[0]];
                                if (ImGui::InputFloat("Value", &value, 0.1f, 1.0f, "%.2f")) {
                                }
                            }
                            else if (editor.isSubgraphContainer(*node)) {
                                ImGui::Text("Subgraph Container");
                                int subgraphId = editor.getSubgraphFromNode(node->id);
                                if (subgraphId >= 0) {
                                    Subgraph* subgraph = editor.getSubgraph(subgraphId);
                                    if (subgraph) {
                                        ImGui::Text("Name: %s", subgraph->name.c_str());
                                        ImGui::Text("Nodes: %zu", subgraph->nodeIds.size());
                                        ImGui::Text("Connections: %zu", subgraph->connectionIds.size());

                                        if (ImGui::Button("Enter Subgraph")) {
                                            editor.enterSubgraph(subgraphId);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else {
                        ImGui::Text("No node selected");
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::CollapsingHeader("Add Nodes")) {
                    ImGui::Text("Node Library");
                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Math", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Add")) {
                            Node* node = editor.createNodeOfType("Math.Add", Vec2(400, 300));
                            if (node && node->inputs.empty()) {
                                editor.addPin(node->id, "A", true, PinType::Blue);
                                editor.addPin(node->id, "B", true, PinType::Blue);
                            }
                            if (node && node->outputs.empty()) {
                                editor.addPin(node->id, "Result", false, PinType::Blue);
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Multiply")) {
                            Node* node = editor.createNodeOfType("Math.Multiply", Vec2(400, 400));
                            if (node && node->inputs.empty()) {
                                editor.addPin(node->id, "A", true, PinType::Blue);
                                editor.addPin(node->id, "B", true, PinType::Blue);
                            }
                            if (node && node->outputs.empty()) {
                                editor.addPin(node->id, "Result", false, PinType::Blue);
                            }
                        }

                        if (ImGui::Button("Subtract")) {
                            Node* node = editor.createNodeOfType("Math.Subtract", Vec2(400, 500));
                            if (node && node->inputs.empty()) {
                                editor.addPin(node->id, "A", true, PinType::Blue);
                                editor.addPin(node->id, "B", true, PinType::Blue);
                            }
                            if (node && node->outputs.empty()) {
                                editor.addPin(node->id, "Result", false, PinType::Blue);
                            }
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Divide")) {
                            Node* node = editor.createNodeOfType("Math.Divide", Vec2(400, 600));
                            if (node && node->inputs.empty()) {
                                editor.addPin(node->id, "A", true, PinType::Blue);
                                editor.addPin(node->id, "B", true, PinType::Blue);
                            }
                            if (node && node->outputs.empty()) {
                                editor.addPin(node->id, "Result", false, PinType::Blue);
                            }
                        }

                        if (ImGui::Button("Constant")) {
                            Node* node = editor.createNodeOfType("Math.Constant", Vec2(300, 400));
                            if (node && node->outputs.empty()) {
                                editor.addPin(node->id, "Value", false, PinType::Blue);
                            }
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Evaluation")) {
                    ImGui::Text("Graph Evaluation");
                    ImGui::Separator();

                    if (ImGui::Button("Evaluate Graph")) {
                        int currentSubgraphId = editor.getCurrentSubgraphId();

                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        editor.enterSubgraphByUUID(mathGraphUuid);

                        evaluationResult = evaluator.evaluateGraph(outputNodeId);
                        evaluationOrder = evaluator.computeEvaluationOrder(outputNodeId);

                        nodeNames.clear();
                        for (int id : evaluationOrder) {
                            const Node* node = editor.getNode(id);
                            if (node) {
                                nodeNames[id] = node->name;
                            }
                        }

                        if (currentSubgraphId >= 0) {
                            while (editor.getCurrentSubgraphId() >= 0) {
                                editor.exitSubgraph();
                            }
                            editor.enterSubgraph(currentSubgraphId);
                        } else {
                            editor.exitSubgraph();
                        }
                    }

                    ImGui::Separator();
                    ImGui::Text("Evaluation Result: %s", evaluationResult.getString().c_str());

                    ImGui::Separator();
                    ImGui::Text("Evaluation Order (DFS):");
                    for (size_t i = 0; i < evaluationOrder.size(); i++) {
                        int nodeId = evaluationOrder[i];
                        ImGui::Text("%zu. %s (ID: %d)", i + 1, nodeNames[nodeId].c_str(), nodeId);
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();

            if (showEvaluationWindow) {
                ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
                ImGui::Begin("Graph Evaluation Results", &showEvaluationWindow);

                if (ImGui::Button("Evaluate Graph Now")) {
                    int currentSubgraphId = editor.getCurrentSubgraphId();

                    while (editor.getCurrentSubgraphId() >= 0) {
                        editor.exitSubgraph();
                    }
                    editor.enterSubgraphByUUID(mathGraphUuid);

                    auto originalEval = GetNodeDefByType("Math.Constant").evaluator;
                    auto modifiedConstantNodeDefs = GetNodeDefByType("Math.Constant");

                    evaluationResult = evaluator.evaluateGraph(outputNodeId);
                    evaluationOrder = evaluator.computeEvaluationOrder(outputNodeId);

                    nodeNames.clear();
                    for (int id : evaluationOrder) {
                        const Node* node = editor.getNode(id);
                        if (node) {
                            nodeNames[id] = node->name;
                        }
                    }

                    if (currentSubgraphId >= 0) {
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        editor.enterSubgraph(currentSubgraphId);
                    } else {
                        editor.exitSubgraph();
                    }
                }

                ImGui::Separator();

                if (evaluationResult.isNumeric()) {
                    ImGui::Text("Final Result: %.2f", evaluationResult.getNumeric());
                } else {
                    ImGui::Text("Final Result: %s", evaluationResult.getString().c_str());
                }

                ImGui::Separator();
                ImGui::Text("Constant Values:");

                for (auto& [nodeId, value] : constantValues) {
                    if (nodeNames.find(nodeId) != nodeNames.end()) {
                        char label[64];
                        sprintf(label, "%s##%d", nodeNames[nodeId].c_str(), nodeId);
                        ImGui::InputFloat(label, &value, 0.1f, 1.0f, "%.2f");
                    }
                }

                ImGui::Separator();
                ImGui::Text("Evaluation Order (DFS):");
                for (size_t i = 0; i < evaluationOrder.size(); i++) {
                    int nodeId = evaluationOrder[i];
                    ImGui::Text("%zu. %s (ID: %d)", i + 1, nodeNames[nodeId].c_str(), nodeId);
                }

                ImGui::End();
            }

            ImGui::Render();
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
            SDL_RenderPresent(renderer);
        }

        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
    } catch (const std::exception& e) {
        return -100;
    } catch (...) {
        return -101;
    }
}