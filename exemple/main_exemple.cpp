#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <iostream>
#include <variant>
#include <vector>
#include <map>
#include <algorithm>
#include <concepts>
#include <ranges>
#include <span>
#include <format>
#include <memory>
#include <queue>

#include "SDL.h"
#include "../AdvancedNodeEditor/NodeEditorAPI.h"

using namespace NodeEditorCore;

struct NodeValue {
    std::variant<float, std::string> value;

    NodeValue() : value(0.0f) {
    }

    NodeValue(float v) : value(v) {
    }

    NodeValue(const std::string &v) : value(v) {
    }

    bool isNumeric() const { return std::holds_alternative<float>(value); }
    float getNumeric() const { return isNumeric() ? std::get<float>(value) : 0.0f; }
    std::string getString() const { return isNumeric() ? std::to_string(getNumeric()) : std::get<std::string>(value); }
};

struct NodeEvaluationInfo {
    UUID nodeId;
    std::string nodeName;
    std::string nodeType;
    NodeValue result;
    std::vector<NodeValue> inputs;
    int dfsOrder;
    int depth;
    bool visited = false;
};

class AdvancedDFSEvaluator {
public:
    AdvancedDFSEvaluator(NodeEditor *editor, const std::unordered_map<UUID, NodeValue> &constantValues)
        : m_editor(editor), m_constantValues(constantValues) {
    }

    std::vector<NodeEvaluationInfo> evaluateGraph(const UUID &outputNodeId = "") {
        m_evaluationOrder.clear();
        m_nodeValues.clear();
        m_evaluationInfo.clear();
        m_dfsIndex = 0;

        const auto &nodes = m_editor->getNodes();
        std::unordered_map<int, UUID> idToUUID;

        for (const auto &node: nodes) {
            m_evaluationInfo[node.uuid] = NodeEvaluationInfo{
                node.uuid,
                node.name,
                node.type,
                NodeValue(0.0f),
                {},
                -1,
                0,
                false
            };

            idToUUID[node.id] = node.uuid;
        }

        int startNodeId = -1;
        if (!outputNodeId.empty()) {
            startNodeId = m_editor->getNodeId(outputNodeId);
        } else {
            for (const auto &node: nodes) {
                if (node.type == "Default" || node.name == "Output") {
                    startNodeId = node.id;
                    break;
                }
            }
            if (startNodeId == -1 && !nodes.empty()) {
                startNodeId = nodes.back().id;
            }
        }

        if (startNodeId != -1) {
            dfs(startNodeId, 0);
        }

        std::vector<NodeEvaluationInfo> result;
        for (const auto &uuid: m_evaluationOrder) {
            if (m_evaluationInfo.count(uuid)) {
                result.push_back(m_evaluationInfo[uuid]);
            }
        }

        return result;
    }

private:
    NodeEditor *m_editor;
    const std::unordered_map<UUID, NodeValue> &m_constantValues;
    std::vector<UUID> m_evaluationOrder;
    std::unordered_map<UUID, NodeValue> m_nodeValues;
    std::unordered_map<UUID, NodeEvaluationInfo> m_evaluationInfo;
    int m_dfsIndex = 0;

    void dfs(int nodeId, int depth) {
        UUID nodeUuid = m_editor->getNodeUUID(nodeId);

        if (m_evaluationInfo[nodeUuid].visited) {
            return;
        }

        m_evaluationInfo[nodeUuid].visited = true;
        m_evaluationInfo[nodeUuid].depth = depth;

        auto inputConnections = m_editor->getInputConnections(nodeId);

        for (const auto &conn: inputConnections) {
            dfs(conn.sourceNodeId, depth + 1);
        }

        Node *node = m_editor->getNode(nodeId);
        std::vector<NodeValue> inputValues;

        if (node) {
            for (const auto &pin: node->inputs) {
                bool inputFound = false;

                for (const auto &conn: inputConnections) {
                    if (conn.targetPinId == pin.id) {
                        UUID sourceNodeUuid = m_editor->getNodeUUID(conn.sourceNodeId);
                        if (m_nodeValues.count(sourceNodeUuid)) {
                            inputValues.push_back(m_nodeValues[sourceNodeUuid]);
                            inputFound = true;
                            break;
                        }
                    }
                }

                if (!inputFound) {
                    inputValues.push_back(NodeValue(0.0f));
                }
            }
        }

        m_evaluationInfo[nodeUuid].inputs = inputValues;

        NodeValue result;

        if (node) {
            if (node->type == "Math.Constant" && m_constantValues.count(nodeUuid)) {
                result = m_constantValues.at(nodeUuid);
            } else if (node->type == "Math.Add") {
                float sum = 0.0f;
                for (const auto &input: inputValues) {
                    sum += input.getNumeric();
                }
                result = NodeValue(sum);
            } else if (node->type == "Math.Multiply") {
                float product = 1.0f;
                for (const auto &input: inputValues) {
                    product *= input.getNumeric();
                }
                result = NodeValue(product);
            } else if (node->type == "Math.Subtract") {
                if (inputValues.size() >= 2) {
                    result = NodeValue(inputValues[0].getNumeric() - inputValues[1].getNumeric());
                } else if (inputValues.size() == 1) {
                    result = NodeValue(inputValues[0].getNumeric());
                } else {
                    result = NodeValue(0.0f);
                }
            } else if (node->type == "Default") {
                if (!inputValues.empty()) {
                    result = inputValues[0];
                } else {
                    result = NodeValue(0.0f);
                }
            } else {
                if (!inputValues.empty()) {
                    result = inputValues[0];
                } else {
                    result = NodeValue(0.0f);
                }
            }
        }

        m_nodeValues[nodeUuid] = result;
        m_evaluationInfo[nodeUuid].result = result;
        m_evaluationInfo[nodeUuid].dfsOrder = m_dfsIndex++;

        m_evaluationOrder.push_back(nodeUuid);
    }
};

template<typename T>
class ResourceManager {
private:
    T* resource = nullptr;
    void (*destroyer)(T*) = nullptr;

public:
    ResourceManager() = default;

    explicit ResourceManager(T* res, void (*des)(T*)) : resource(res), destroyer(des) {}

    ~ResourceManager() {
        if (resource && destroyer) {
            destroyer(resource);
        }
    }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    ResourceManager(ResourceManager&& other) noexcept : resource(other.resource), destroyer(other.destroyer) {
        other.resource = nullptr;
        other.destroyer = nullptr;
    }

    ResourceManager& operator=(ResourceManager&& other) noexcept {
        if (this != &other) {
            if (resource && destroyer) {
                destroyer(resource);
            }
            resource = other.resource;
            destroyer = other.destroyer;
            other.resource = nullptr;
            other.destroyer = nullptr;
        }
        return *this;
    }

    T* get() const { return resource; }

    operator T*() const { return resource; }

    T* operator->() const { return resource; }
};

using WindowManager = ResourceManager<SDL_Window>;
using RendererManager = ResourceManager<SDL_Renderer>;

int main(int argc, char *argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        std::cerr << "Error initializing SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    int windowWidth = 1280;
    int windowHeight = 720;

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    WindowManager window(
        SDL_CreateWindow(
            "Node Editor Framework Demo",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            windowWidth, windowHeight, window_flags
        ),
        SDL_DestroyWindow
    );

    if (!window.get()) {
        std::cerr << "Error creating window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -2;
    }

    RendererManager renderer(
        SDL_CreateRenderer(
            window.get(), -1,
            SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED
        ),
        SDL_DestroyRenderer
    );

    if (!renderer.get()) {
        std::cerr << "Error creating renderer: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -3;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window.get(), renderer.get());
    ImGui_ImplSDLRenderer2_Init(renderer.get());

    auto cleanup = [&]() {
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        SDL_Quit();
    };

    try {
        NodeEditorAPI editor;
        editor.initialize();

        editor.setWindowSize(windowWidth, windowHeight);

        editor.registerNodeType({
            "Math.Add", "Add", "Math", "Addition node", "+",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}
        });

        editor.registerNodeType({
            "Math.Multiply", "Multiply", "Math", "Multiplication node", "*",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}
        });

        editor.registerNodeType({
            "Math.Subtract", "Subtract", "Math", "Subtraction node", "-",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}
        });

        editor.registerNodeType({
            "Math.Constant", "Constant", "Math", "Constant value", "C",
            {},
            {{"Value", PinType::Blue}}
        });

        editor.registerNodeType({
            "Default", "Output", "Utility", "Output node", "O",
            {{"Value", PinType::Blue}},
            {}
        });

        NodeEditor *rawEditor = editor.getUnderlyingEditor();

        // Assurons-nous de sortir de tous les sous-graphes existants
        while (rawEditor->getCurrentSubgraphId() >= 0) {
            rawEditor->exitSubgraph();
        }

        // Créer un nouveau graphe principal ou utiliser l'existant
        UUID mainGraphUuid = editor.createGraph("Main Graph");
        std::cout << "Main graph created: " << mainGraphUuid << std::endl;
        editor.switchToGraph(mainGraphUuid);

        // Forcer à nouveau la sortie de tout sous-graphe après le switch
        while (rawEditor->getCurrentSubgraphId() >= 0) {
            rawEditor->exitSubgraph();
        }

        // Création des nœuds mathématiques directement dans le graphe principal
        const auto &existingNodes = rawEditor->getNodes();
        bool hasExistingOutputNode = false;

        UUID const1NodeId, const2NodeId, const3NodeId, addNodeId, multiplyNodeId, subtractNodeId, outputNodeId;
        std::unordered_map<UUID, NodeValue> constantValues;

        // Vérifier si les nœuds existent déjà
        bool hasAllRequiredNodes = true;

        for (const auto &node: existingNodes) {
            if (node.type == "Default" || node.name == "Output") {
                outputNodeId = node.uuid;
                hasExistingOutputNode = true;
                std::cout << "Existing output node found: " << outputNodeId << std::endl;
            }

            if (node.type == "Math.Constant") {
                if (node.name == "Value A") const1NodeId = node.uuid;
                else if (node.name == "Value B") const2NodeId = node.uuid;
                else if (node.name == "Value C") const3NodeId = node.uuid;

                if (!node.uuid.empty()) {
                    auto constValue = editor.getConstantValue(node.uuid);
                    if (constValue.has_value()) {
                        try {
                            NodeValue value = std::any_cast<NodeValue>(constValue);
                            constantValues[node.uuid] = value;
                        } catch (...) {
                            constantValues[node.uuid] = NodeValue(1.0f);
                        }
                    } else {
                        constantValues[node.uuid] = NodeValue(1.0f);
                    }
                }
            } else if (node.type == "Math.Add") addNodeId = node.uuid;
            else if (node.type == "Math.Multiply") multiplyNodeId = node.uuid;
            else if (node.type == "Math.Subtract") subtractNodeId = node.uuid;
        }

        // Vérifier si tous les nœuds requis existent
        if (const1NodeId.empty() || const2NodeId.empty() || const3NodeId.empty() ||
            addNodeId.empty() || multiplyNodeId.empty() || subtractNodeId.empty() ||
            outputNodeId.empty()) {
            hasAllRequiredNodes = false;
        }

        // Recréer tous les nœuds si le nœud de sortie ou tout autre nœud requis n'existe pas
        if (!hasAllRequiredNodes) {
            // Supprimer les nœuds existants potentiellement incomplets
            std::vector<UUID> nodesToRemove;
            for (const auto &node: existingNodes) {
                nodesToRemove.push_back(node.uuid);
            }

            for (const auto &nodeId: nodesToRemove) {
                try {
                    editor.removeNode(nodeId);
                } catch (...) {
                    // Ignorer les erreurs de suppression
                }
            }

            std::cout << "Creating math nodes in main graph..." << std::endl;

            const1NodeId = editor.createNode("Math.Constant", "Value A", Vec2(300, 100));
            const2NodeId = editor.createNode("Math.Constant", "Value B", Vec2(300, 200));
            const3NodeId = editor.createNode("Math.Constant", "Value C", Vec2(300, 300));

            addNodeId = editor.createNode("Math.Add", "Add", Vec2(500, 150));
            multiplyNodeId = editor.createNode("Math.Multiply", "Multiply", Vec2(500, 300));
            subtractNodeId = editor.createNode("Math.Subtract", "Subtract", Vec2(700, 200));

            // Création explicite du nœud de sortie
            outputNodeId = editor.createNode("Default", "Output", Vec2(900, 200));
            std::cout << "Output node created with UUID: " << outputNodeId << std::endl;

            constantValues[const1NodeId] = NodeValue(5.0f);
            constantValues[const2NodeId] = NodeValue(3.0f);
            constantValues[const3NodeId] = NodeValue(2.0f);

            for (const auto &[id, value]: constantValues) {
                editor.setConstantValue(id, value);
            }

            try {
                std::cout << "Creating connections..." << std::endl;
                editor.connectNodes(const1NodeId, "Value", addNodeId, "A");
                std::cout << "Connected Const1 to Add" << std::endl;

                editor.connectNodes(const2NodeId, "Value", addNodeId, "B");
                std::cout << "Connected Const2 to Add" << std::endl;

                editor.connectNodes(addNodeId, "Result", multiplyNodeId, "A");
                std::cout << "Connected Add to Multiply" << std::endl;

                editor.connectNodes(const3NodeId, "Value", multiplyNodeId, "B");
                std::cout << "Connected Const3 to Multiply" << std::endl;

                editor.connectNodes(multiplyNodeId, "Result", subtractNodeId, "A");
                std::cout << "Connected Multiply to Subtract" << std::endl;

                editor.connectNodes(const1NodeId, "Value", subtractNodeId, "B");
                std::cout << "Connected Const1 to Subtract" << std::endl;

                editor.connectNodes(subtractNodeId, "Result", outputNodeId, "Value");
                std::cout << "Connected Subtract to Output" << std::endl;

                std::cout << "All connections created successfully" << std::endl;
            } catch (const std::exception &e) {
                std::cerr << "Error creating connections: " << e.what() << std::endl;
            }
        }

        // Évaluer le graphe initial et zoomer pour voir tous les nœuds
        AdvancedDFSEvaluator dfsEvaluator(rawEditor, constantValues);
        std::vector<NodeEvaluationInfo> evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);

        NodeValue finalResult(0.0f);
        if (!evaluationResults.empty()) {
            finalResult = evaluationResults.back().result;
            std::cout << "Evaluation result: " << finalResult.getString() << std::endl;
        }

        // S'assurer que tous les nœuds sont visibles
        editor.zoomToFit(50.0f);

        bool showNodeCreationMenu = false;
        ImVec2 nodeCreationPos(0, 0);
        bool showEvaluationDetails = true;
        bool autoEvaluate = true;
        int selectedNodeIndex = -1;

        bool done = false;
        while (!done) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);

                if (event.type == SDL_QUIT) {
                    done = true;
                }
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(window.get())) {
                    done = true;
                }

                if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_RIGHT) {
                    ImGuiIO &io = ImGui::GetIO();
                    if (io.WantCaptureMouse) {
                        continue;
                    }

                    showNodeCreationMenu = true;
                    nodeCreationPos = ImVec2(event.button.x, event.button.y);

                    Vec2 canvasPos = rawEditor->screenToCanvas(Vec2(nodeCreationPos.x, nodeCreationPos.y));
                    nodeCreationPos = ImVec2(canvasPos.x, canvasPos.y);
                }

                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    windowWidth = event.window.data1;
                    windowHeight = event.window.data2;
                    editor.setWindowSize(windowWidth, windowHeight);
                }
            }

            ImGui_ImplSDLRenderer2_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();

            int currentSubgraphId = -1;
            std::string currentGraphName = "Main Graph";

            std::vector<UUID> selectedNodeUUIDs = rawEditor->getSelectedNodeUUIDs();
            Node *selectedNode = nullptr;
            if (!selectedNodeUUIDs.empty()) {
                selectedNode = rawEditor->getNodeByUUID(selectedNodeUUIDs[0]);
            }

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Exit")) done = true;
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View")) {
                    if (ImGui::MenuItem("Zoom to Fit", "F")) {
                        editor.zoomToFit();
                    }
                    if (ImGui::MenuItem("Center View", "C")) {
                        editor.centerView();
                    }
                    if (ImGui::MenuItem("Reset View")) {
                        // Sortir de tout sous-graphe éventuel et revenir au graphe principal
                        while (rawEditor->getCurrentSubgraphId() >= 0) {
                            rawEditor->exitSubgraph();
                        }
                        editor.switchToGraph(mainGraphUuid);
                        editor.zoomToFit();
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Evaluate")) {
                    if (ImGui::MenuItem("Evaluate Graph")) {
                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);

                        if (!evaluationResults.empty()) {
                            finalResult = evaluationResults.back().result;
                        }
                    }

                    ImGui::MenuItem("Auto-Evaluate on Changes", nullptr, &autoEvaluate);
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Navigation")) {
                    if (ImGui::MenuItem("Zoom to Fit")) editor.zoomToFit();
                    if (ImGui::MenuItem("Center View")) editor.centerView();

                    ImGui::EndMenu();
                }

                ImGui::SameLine(ImGui::GetWindowWidth() - 200);
                ImGui::Text("Main Graph");

                ImGui::EndMainMenuBar();
            }

            ImVec2 totalSize = ImGui::GetIO().DisplaySize;
            ImVec2 menuBarSize = ImGui::GetItemRectSize();
            float panelWidth = 350.0f;
            float editorWidth = totalSize.x - panelWidth;
            float panelHeight = totalSize.y - menuBarSize.y;

            ImGui::SetNextWindowPos(ImVec2(0, menuBarSize.y));
            ImGui::SetNextWindowSize(ImVec2(editorWidth, 30));
            ImGui::Begin("Navigation", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            if (ImGui::Button("Zoom to Fit")) {
                editor.zoomToFit();
            }

            ImGui::SameLine();
            if (ImGui::Button("Center View")) {
                editor.centerView();
            }

            ImGui::SameLine();
            ImGui::Text("Main Graph");

            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(0, menuBarSize.y + 30));
            ImGui::SetNextWindowSize(ImVec2(editorWidth, panelHeight - 30));
            ImGui::Begin("Node Editor", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
                                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

            ImVec2 editorSize = ImGui::GetWindowSize();
            editor.setWindowSize(Vec2(editorSize.x, editorSize.y));

            editor.beginFrame();
            editor.render();
            editor.endFrame();

            if (showNodeCreationMenu) {
                ImGui::OpenPopup("NodeCreationMenu");
                showNodeCreationMenu = false;
            }

            if (ImGui::BeginPopup("NodeCreationMenu")) {
                ImGui::Text("Add Node:");
                ImGui::Separator();

                int popupSubgraphId = rawEditor->getCurrentSubgraphId();

                if (ImGui::MenuItem("Add Constant")) {
                    UUID nodeId = editor.createNode("Math.Constant", "New Constant",
                                                    Vec2(nodeCreationPos.x, nodeCreationPos.y));

                    editor.setConstantValue(nodeId, NodeValue(1.0f));
                    constantValues[nodeId] = NodeValue(1.0f);

                    if (autoEvaluate) {
                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                        if (!evaluationResults.empty()) {
                            finalResult = evaluationResults.back().result;
                        }
                    }
                }

                if (ImGui::MenuItem("Add Add")) {
                    editor.createNode("Math.Add", "New Add", Vec2(nodeCreationPos.x, nodeCreationPos.y));

                    if (autoEvaluate) {
                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                        if (!evaluationResults.empty()) {
                            finalResult = evaluationResults.back().result;
                        }
                    }
                }

                if (ImGui::MenuItem("Add Multiply")) {
                    editor.createNode("Math.Multiply", "New Multiply", Vec2(nodeCreationPos.x, nodeCreationPos.y));

                    if (autoEvaluate) {
                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                        if (!evaluationResults.empty()) {
                            finalResult = evaluationResults.back().result;
                        }
                    }
                }

                if (ImGui::MenuItem("Add Subtract")) {
                    editor.createNode("Math.Subtract", "New Subtract", Vec2(nodeCreationPos.x, nodeCreationPos.y));

                    if (autoEvaluate) {
                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                        if (!evaluationResults.empty()) {
                            finalResult = evaluationResults.back().result;
                        }
                    }
                }

                if (ImGui::MenuItem("Add Output")) {
                    editor.createNode("Default", "Output", Vec2(nodeCreationPos.x, nodeCreationPos.y));

                    if (autoEvaluate) {
                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                        if (!evaluationResults.empty()) {
                            finalResult = evaluationResults.back().result;
                        }
                    }
                }

                ImGui::EndPopup();
            }

            ImGui::End();

            ImGui::SetNextWindowPos(ImVec2(editorWidth, menuBarSize.y));
            ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));
            ImGui::Begin("Properties", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

            if (ImGui::BeginTabBar("PropertiesTabBar")) {
                if (ImGui::BeginTabItem("Add Nodes")) {
                    if (ImGui::Button("Add Constant")) {
                        UUID nodeId = editor.createNode("Math.Constant", "New Constant", Vec2(400, 400));

                        editor.setConstantValue(nodeId, NodeValue(1.0f));
                        constantValues[nodeId] = NodeValue(1.0f);

                        if (autoEvaluate) {
                            evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                            if (!evaluationResults.empty()) {
                                finalResult = evaluationResults.back().result;
                            }
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Add Add")) {
                        editor.createNode("Math.Add", "New Add", Vec2(600, 400));

                        if (autoEvaluate) {
                            evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                            if (!evaluationResults.empty()) {
                                finalResult = evaluationResults.back().result;
                            }
                        }
                    }

                    if (ImGui::Button("Add Multiply")) {
                        editor.createNode("Math.Multiply", "New Multiply", Vec2(600, 500));

                        if (autoEvaluate) {
                            evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                            if (!evaluationResults.empty()) {
                                finalResult = evaluationResults.back().result;
                            }
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Add Subtract")) {
                        editor.createNode("Math.Subtract", "New Subtract", Vec2(600, 600));

                        if (autoEvaluate) {
                            evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                            if (!evaluationResults.empty()) {
                                finalResult = evaluationResults.back().result;
                            }
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Selected Node")) {
                    if (selectedNode) {
                        ImGui::Text("Node: %s", selectedNode->name.c_str());
                        ImGui::Text("Type: %s", selectedNode->type.c_str());
                        ImGui::Text("UUID: %s", selectedNode->uuid.c_str());
                        ImGui::Separator();

                        if (selectedNode->type == "Math.Constant") {
                            if (constantValues.find(selectedNode->uuid) != constantValues.end()) {
                                float value = constantValues[selectedNode->uuid].getNumeric();
                                if (ImGui::SliderFloat("Value", &value, 0.0f, 10.0f)) {
                                    constantValues[selectedNode->uuid] = NodeValue(value);
                                    editor.setConstantValue(selectedNode->uuid, NodeValue(value));

                                    if (autoEvaluate) {
                                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                                        if (!evaluationResults.empty()) {
                                            finalResult = evaluationResults.back().result;
                                        }
                                    }
                                }
                            } else {
                                float value = 1.0f;
                                if (ImGui::SliderFloat("Value", &value, 0.0f, 10.0f)) {
                                    constantValues[selectedNode->uuid] = NodeValue(value);
                                    editor.setConstantValue(selectedNode->uuid, NodeValue(value));

                                    if (autoEvaluate) {
                                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                                        if (!evaluationResults.empty()) {
                                            finalResult = evaluationResults.back().result;
                                        }
                                    }
                                }
                            }
                        }

                        char nameBuf[64];
                        strncpy(nameBuf, selectedNode->name.c_str(), sizeof(nameBuf) - 1);
                        nameBuf[sizeof(nameBuf) - 1] = '\0';

                        if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
                        }

                        ImGui::Separator();

                        if (!selectedNode->inputs.empty()) {
                            ImGui::Text("Input Pins:");
                            for (const auto &pin: selectedNode->inputs) {
                                ImGui::Text("  %s (ID: %d)", pin.name.c_str(), pin.id);
                            }
                        }

                        if (!selectedNode->outputs.empty()) {
                            ImGui::Text("Output Pins:");
                            for (const auto &pin: selectedNode->outputs) {
                                ImGui::Text("  %s (ID: %d)", pin.name.c_str(), pin.id);
                            }
                        }

                        ImGui::Separator();

                        if (ImGui::Button("Delete Node")) {
                            editor.removeNode(selectedNode->uuid);

                            if (autoEvaluate) {
                                evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                                if (!evaluationResults.empty()) {
                                    finalResult = evaluationResults.back().result;
                                } else {
                                    finalResult = NodeValue(0.0f);
                                }
                            }
                        }
                    } else {
                        ImGui::Text("No node selected");
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Navigation")) {
                    if (ImGui::Button("Zoom To Fit")) {
                        editor.zoomToFit();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Center View")) {
                        editor.centerView();
                    }

                    if (ImGui::Button("Zoom To Fit Selected")) {
                        editor.zoomToFitSelected();
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Smooth Center")) {
                        editor.smoothCenterView(0.5f);
                    }

                    if (ImGui::Button("Center on Add Node")) {
                        if (!addNodeId.empty()) {
                            editor.centerOnNode(addNodeId);
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Smooth Center on Add")) {
                        if (!addNodeId.empty()) {
                            editor.smoothCenterOnNode(addNodeId, 0.5f);
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Evaluation")) {
                    ImGui::Text("Final Result: %s", finalResult.getString().c_str());

                    ImGui::Separator();

                    if (ImGui::Button("Evaluate Graph Now")) {
                        int currentSubgraph = rawEditor->getCurrentSubgraphId();

                        while (rawEditor->getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }

                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);

                        if (!evaluationResults.empty()) {
                            finalResult = evaluationResults.back().result;
                        }

                        while (rawEditor->getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        if (currentSubgraph >= 0) {
                            editor.enterSubgraph(rawEditor->getSubgraphUUID(currentSubgraph));
                        }
                    }

                    ImGui::SameLine();
                    ImGui::Checkbox("Auto-Evaluate", &autoEvaluate);

                    ImGui::Separator();

                    if (!addNodeId.empty()) {
                        ImGui::Text("Constant Values in Math Graph:");

                        bool valueChanged = false;
                        for (auto &[nodeId, value]: constantValues) {
                            Node *node = rawEditor->getNodeByUUID(nodeId);
                            if (!node || node->type != "Math.Constant") continue;

                            float fValue = value.getNumeric();
                            if (ImGui::SliderFloat(node->name.c_str(), &fValue, 0.0f, 10.0f)) {
                                value = NodeValue(fValue);
                                editor.setConstantValue(nodeId, value);
                                valueChanged = true;
                            }
                        }

                    if (valueChanged && autoEvaluate) {
                        evaluationResults = dfsEvaluator.evaluateGraph(outputNodeId);
                        if (!evaluationResults.empty()) {
                            finalResult = evaluationResults.back().result;
                        }
                    }
                    } else {
                        ImGui::Text("Navigate to Math Graph to edit constants");
                    }

                    ImGui::Separator();
                    ImGui::Checkbox("Show Evaluation Details", &showEvaluationDetails);

                    if (showEvaluationDetails && !evaluationResults.empty()) {
                        ImGui::Text("DFS Evaluation Order:");

                        auto sortedResults = evaluationResults;
                        std::ranges::sort(sortedResults, [](const NodeEvaluationInfo &a, const NodeEvaluationInfo &b) {
                            return a.dfsOrder > b.dfsOrder;
                        });

                        ImGui::BeginChild("EvaluationOrder", ImVec2(0, 150), true,
                                          ImGuiWindowFlags_HorizontalScrollbar);
                        for (size_t i = 0; i < sortedResults.size(); i++) {
                            const auto &info = sortedResults[i];

                            std::string indent(info.depth * 2, ' ');
                            std::string label = indent + std::to_string(i + 1) + ". " + info.nodeName + " (" + info.
                                                nodeType + ")";

                            ImVec4 color;
                            if (info.nodeType == "Math.Constant") {
                                color = ImVec4(0.7f, 1.0f, 0.7f, 1.0f);
                            } else if (info.nodeType == "Math.Add") {
                                color = ImVec4(0.7f, 0.7f, 1.0f, 1.0f);
                            } else if (info.nodeType == "Math.Multiply") {
                                color = ImVec4(1.0f, 0.7f, 0.7f, 1.0f);
                            } else if (info.nodeType == "Math.Subtract") {
                                color = ImVec4(1.0f, 1.0f, 0.7f, 1.0f);
                            } else if (info.nodeType == "Default") {
                                color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
                            } else {
                                color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
                            }

                            ImGui::PushStyleColor(ImGuiCol_Text, color);
                            bool isSelected = (selectedNodeIndex == static_cast<int>(i));
                            if (ImGui::Selectable(label.c_str(), isSelected)) {
                                selectedNodeIndex = i;
                            }
                            ImGui::PopStyleColor();
                        }
                        ImGui::EndChild();

                        if (selectedNodeIndex >= 0 && selectedNodeIndex < static_cast<int>(sortedResults.size())) {
                            const auto &selectedNode = sortedResults[selectedNodeIndex];
                            ImGui::Separator();
                            ImGui::Text("Node Details:");
                            ImGui::Text("Name: %s", selectedNode.nodeName.c_str());
                            ImGui::Text("Type: %s", selectedNode.nodeType.c_str());
                            ImGui::Text("DFS Order: %d", selectedNode.dfsOrder);
                            ImGui::Text("Depth: %d", selectedNode.depth);

                            if (!selectedNode.inputs.empty()) {
                                ImGui::Text("Inputs:");
                                for (size_t i = 0; i < selectedNode.inputs.size(); i++) {
                                    ImGui::Text("  %zu: %s", i + 1, selectedNode.inputs[i].getString().c_str());
                                }
                            } else {
                                ImGui::Text("Inputs: None");
                            }

                            ImGui::Text("Result: %s", selectedNode.result.getString().c_str());
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Graph Info")) {
                    ImGui::Text("Main Graph");

                    const auto &nodes = rawEditor->getNodes();
                    ImGui::Text("Nodes in graph: %zu", nodes.size());

                    if (ImGui::TreeNode("Nodes List")) {
                        for (const auto &node: nodes) {
                            ImGui::Text("%s (Type: %s, ID: %d)", node.name.c_str(), node.type.c_str(), node.id);
                        }
                        ImGui::TreePop();
                    }

                    const auto &connections = rawEditor->getConnections();
                    ImGui::Text("Connections in current graph: %zu", connections.size());

                    if (ImGui::TreeNode("Connections List")) {
                        for (const auto &conn: connections) {
                            Node *sourceNode = rawEditor->getNode(conn.startNodeId);
                            Node *targetNode = rawEditor->getNode(conn.endNodeId);

                            if (sourceNode && targetNode) {
                                ImGui::Text("%s -> %s", sourceNode->name.c_str(), targetNode->name.c_str());
                            }
                        }
                        ImGui::TreePop();
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();

            ImGui::Render();
            SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
            SDL_RenderClear(renderer.get());
            ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer.get());
            SDL_RenderPresent(renderer.get());
        }
    } catch (const std::exception &e) {
        std::cerr << "Unhandled exception: " << e.what() << std::endl;
        cleanup();
        return -1;
    }

    cleanup();
    return 0;
}