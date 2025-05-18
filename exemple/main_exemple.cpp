#include <SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_sdlrenderer2.h>
#include <iostream>
#include <exception>

#include "../AdvancedNodeEditor/NodeEditor.h"

using namespace NodeEditorCore;

struct NodeDefinition {
    std::string type;
    std::string name;
    std::vector<std::pair<std::string, PinType>> inputs;
    std::vector<std::pair<std::string, PinType>> outputs;
    std::string iconSymbol;
};

static NodeDefinition GetNodeDefByType(const std::string& type) {
    static std::unordered_map<std::string, NodeDefinition> nodeDefs = {
        {"Math.Add", {
            "Math.Add", "Add",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}, "+"
        }},
        {"Math.Multiply", {
            "Math.Multiply", "Multiply",
            {{"A", PinType::Blue}, {"B", PinType::Blue}},
            {{"Result", PinType::Blue}}, "*"
        }},
        {"Geometry.Box", {
            "Geometry.Box", "Box",
            {{"Size", PinType::Green}, {"Center", PinType::Green}},
            {{"Geometry", PinType::Red}}, "■"
        }},
        {"Geometry.Sphere", {
            "Geometry.Sphere", "Sphere",
            {{"Radius", PinType::Green}, {"Center", PinType::Green}},
            {{"Geometry", PinType::Red}}, "●"
        }},
        {"Material.Basic", {
            "Material.Basic", "Material",
            {{"Color", PinType::Purple}, {"Roughness", PinType::Blue}},
            {{"Material", PinType::Yellow}}, "M"
        }},
        {"Render.MeshRenderer", {
            "Render.MeshRenderer", "Renderer",
            {{"Geometry", PinType::Red}, {"Material", PinType::Yellow}},
            {{"Output", PinType::Cyan}}, "R"
        }},
        {"Subgraph.Input", {
            "Subgraph.Input", "Input",
            {},
            {{"Value", PinType::Blue}}, "I"
        }},
        {"Subgraph.Output", {
            "Subgraph.Output", "Output",
            {{"Value", PinType::Blue}},
            {}, "O"
        }}
    };

    if (nodeDefs.find(type) == nodeDefs.end()) {
        throw std::runtime_error("Type de noeud inconnu: " + type);
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

    return node;
}

int main(int argc, char* argv[]) {
    try {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            return -1;
        }

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        SDL_Window* window = SDL_CreateWindow("Advanced Node Editor - Subgraphs Demo",
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

        editor.setGraphTitle("Houdini-Style Node Graph");
        editor.setGraphTitleStyle(NodeEditor::TitleStyle::Houdini);
        editor.setConnectionStyle(NodeEditor::ConnectionStyle::Bezier);
        editor.setConnectionThickness(2.5f);
        editor.enableNodeAvoidance(true);
        editor.enableMinimap(true);
        editor.activateAllConnectionFlows(false, 0.0f);
        editor.setShowSubgraphBreadcrumbs(true);

        UUID mainGraphUuid = editor.createSubgraphWithUUID("Main");
        UUID mathSubgraphUuid = editor.createSubgraphWithUUID("Math Utilities");
        UUID renderSubgraphUuid = editor.createSubgraphWithUUID("Rendering");

        int boxNodeId = editor.addNode("Box", "Geometry.Box", Vec2(100, 100));
        int sphereNodeId = editor.addNode("Sphere", "Geometry.Sphere", Vec2(100, 250));

        editor.enterSubgraphByUUID(mathSubgraphUuid);
        int addNodeId = editor.addNode("Add", "Math.Add", Vec2(300, 150));
        int multiplyNodeId = editor.addNode("Multiply", "Math.Multiply", Vec2(600, 150));
        int inputNodeId = editor.addNode("Input A", "Subgraph.Input", Vec2(50, 100));
        int input2NodeId = editor.addNode("Input B", "Subgraph.Input", Vec2(50, 200));
        int outputNodeId = editor.addNode("Output", "Subgraph.Output", Vec2(900, 150));

        Node* inputNode = editor.getNode(inputNodeId);
        Node* addNode = editor.getNode(addNodeId);
        if (inputNode && !inputNode->outputs.empty() && addNode && !addNode->inputs.empty()) {
            editor.addConnection(inputNodeId, inputNode->outputs[0].id, addNodeId, addNode->inputs[0].id);
        }

        Node* input2Node = editor.getNode(input2NodeId);
        if (input2Node && !input2Node->outputs.empty() && addNode && addNode->inputs.size() >= 2) {
            editor.addConnection(input2NodeId, input2Node->outputs[0].id, addNodeId, addNode->inputs[1].id);
        }

        Node* multiplyNode = editor.getNode(multiplyNodeId);
        if (addNode && !addNode->outputs.empty() && multiplyNode && !multiplyNode->inputs.empty()) {
            editor.addConnection(addNodeId, addNode->outputs[0].id, multiplyNodeId, multiplyNode->inputs[0].id);
        }
        if (addNode && !addNode->outputs.empty() && multiplyNode && multiplyNode->inputs.size() >= 2) {
            editor.addConnection(addNodeId, addNode->outputs[0].id, multiplyNodeId, multiplyNode->inputs[1].id);
        }

        Node* outputNode = editor.getNode(outputNodeId);
        if (multiplyNode && !multiplyNode->outputs.empty() && outputNode && !outputNode->inputs.empty()) {
            editor.addConnection(multiplyNodeId, multiplyNode->outputs[0].id, outputNodeId, outputNode->inputs[0].id);
        }

        auto mathSubgraph = editor.getSubgraph(editor.getSubgraphId(mathSubgraphUuid));
        if (mathSubgraph) {
            mathSubgraph->interfaceInputs.push_back((inputNodeId << 16) | 1);
            mathSubgraph->interfaceInputs.push_back((input2NodeId << 16) | 1);
            mathSubgraph->interfaceOutputs.push_back((outputNodeId << 16) | 1);
        }
        editor.exitSubgraph();

        editor.enterSubgraphByUUID(renderSubgraphUuid);
        int materialNodeId = editor.addNode("Material", "Material.Basic", Vec2(300, 150));
        int rendererNodeId = editor.addNode("Renderer", "Render.MeshRenderer", Vec2(600, 150));
        int geomInputNodeId = editor.addNode("Geometry Input", "Subgraph.Input", Vec2(100, 100));
        int renderOutputNodeId = editor.addNode("Output", "Subgraph.Output", Vec2(900, 150));

        Node* geomInputNode = editor.getNode(geomInputNodeId);
        Node* rendererNode = editor.getNode(rendererNodeId);
        if (geomInputNode && !geomInputNode->outputs.empty() && rendererNode && !rendererNode->inputs.empty()) {
            editor.addConnection(geomInputNodeId, geomInputNode->outputs[0].id, rendererNodeId, rendererNode->inputs[0].id);
        }

        Node* materialNode = editor.getNode(materialNodeId);
        if (materialNode && !materialNode->outputs.empty() && rendererNode && rendererNode->inputs.size() >= 2) {
            editor.addConnection(materialNodeId, materialNode->outputs[0].id, rendererNodeId, rendererNode->inputs[1].id);
        }

        Node* renderOutputNode = editor.getNode(renderOutputNodeId);
        if (rendererNode && !rendererNode->outputs.empty() && renderOutputNode && !renderOutputNode->inputs.empty()) {
            editor.addConnection(rendererNodeId, rendererNode->outputs[0].id, renderOutputNodeId, renderOutputNode->inputs[0].id);
        }

        auto renderSubgraph = editor.getSubgraph(editor.getSubgraphId(renderSubgraphUuid));
        if (renderSubgraph) {
            renderSubgraph->interfaceInputs.push_back((geomInputNodeId << 16) | 1);
            renderSubgraph->interfaceOutputs.push_back((renderOutputNodeId << 16) | 1);
        }
        editor.exitSubgraph();

        Node* mathSubgraphNode = editor.createSubgraphNode(editor.getSubgraphId(mathSubgraphUuid),
                                                         "Math Utilities",
                                                         Vec2(300, 175));
        int mathSubgraphNodeId = mathSubgraphNode ? mathSubgraphNode->id : -1;

        Node* renderSubgraphNode = editor.createSubgraphNode(editor.getSubgraphId(renderSubgraphUuid),
                                                           "Rendering",
                                                           Vec2(500, 175));
        int renderSubgraphNodeId = renderSubgraphNode ? renderSubgraphNode->id : -1;

        Node* boxNode = editor.getNode(boxNodeId);
        if (mathSubgraphNode && boxNode) {
            if (!boxNode->outputs.empty() && !mathSubgraphNode->inputs.empty()) {
                editor.addConnection(boxNodeId, boxNode->outputs[0].id, mathSubgraphNodeId, mathSubgraphNode->inputs[0].id);
            }
        }

        Node* sphereNode = editor.getNode(sphereNodeId);
        if (sphereNode && mathSubgraphNode) {
            if (!sphereNode->outputs.empty() && mathSubgraphNode->inputs.size() >= 2) {
                editor.addConnection(sphereNodeId, sphereNode->outputs[0].id, mathSubgraphNodeId, mathSubgraphNode->inputs[1].id);
            }
        }

        if (mathSubgraphNode && renderSubgraphNode) {
            if (!mathSubgraphNode->outputs.empty() && !renderSubgraphNode->inputs.empty()) {
                editor.addConnection(mathSubgraphNodeId, mathSubgraphNode->outputs[0].id, renderSubgraphNodeId, renderSubgraphNode->inputs[0].id);
            }
        }

        enum class CurrentPanel {
            NodeEditor,
            NodeProperties,
            SubgraphNavigator
        };

        CurrentPanel currentSidePanel = CurrentPanel::SubgraphNavigator;
        std::vector<std::string> subgraphHistory;
        bool showProperties = true;

        bool done = false;
        int frameCount = 0;
        while (!done) {
            frameCount++;
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
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Subgraph")) {
                    if (ImGui::MenuItem("Main Graph")) {
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                    }
                    if (ImGui::MenuItem("Math Utilities")) {
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        editor.enterSubgraphByUUID(mathSubgraphUuid);
                    }
                    if (ImGui::MenuItem("Rendering")) {
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        editor.enterSubgraphByUUID(renderSubgraphUuid);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Create New Subgraph...")) {
                        UUID newGraphUuid = editor.createSubgraphWithUUID("New Subgraph");
                        editor.enterSubgraphByUUID(newGraphUuid);
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
                    ImGui::Text("Current: [ERREUR: subgraph nul]");
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

                        if (ImGui::Button("Math Utilities")) {
                            while (editor.getCurrentSubgraphId() >= 0) {
                                editor.exitSubgraph();
                            }
                            editor.enterSubgraphByUUID(mathSubgraphUuid);
                        }

                        if (ImGui::Button("Rendering")) {
                            while (editor.getCurrentSubgraphId() >= 0) {
                                editor.exitSubgraph();
                            }
                            editor.enterSubgraphByUUID(renderSubgraphUuid);
                        }
                    }

                    if (ImGui::CollapsingHeader("Create New", ImGuiTreeNodeFlags_DefaultOpen)) {
                        static char newSubgraphName[128] = "New Subgraph";
                        ImGui::InputText("Name", newSubgraphName, IM_ARRAYSIZE(newSubgraphName));

                        if (ImGui::Button("Create Subgraph")) {
                            UUID newGraphUuid = editor.createSubgraphWithUUID(newSubgraphName);
                            editor.enterSubgraphByUUID(newGraphUuid);
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

                            if (node->type == "Geometry.Box") {
                                static float size[3] = { 1.0f, 1.0f, 1.0f };
                                ImGui::Text("Size");
                                ImGui::InputFloat3("##size", size);

                                static float center[3] = { 0.0f, 0.0f, 0.0f };
                                ImGui::Text("Center");
                                ImGui::InputFloat3("##center", center);
                            }
                            else if (node->type == "Material.Basic") {
                                static float color[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
                                ImGui::Text("Color");
                                ImGui::ColorEdit4("##color", color);

                                static float roughness = 0.5f;
                                ImGui::Text("Roughness");
                                ImGui::SliderFloat("##roughness", &roughness, 0.0f, 1.0f);
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

                if (ImGui::BeginTabItem("Add Nodes")) {
                    ImGui::Text("Node Library");
                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Math", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Add")) {
                            editor.createNodeOfType("Math.Add", Vec2(300, 200));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Multiply")) {
                            editor.createNodeOfType("Math.Multiply", Vec2(300, 300));
                        }
                    }

                    if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Box")) {
                            editor.createNodeOfType("Geometry.Box", Vec2(300, 200));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Sphere")) {
                            editor.createNodeOfType("Geometry.Sphere", Vec2(300, 300));
                        }
                    }

                    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Basic Material")) {
                            editor.createNodeOfType("Material.Basic", Vec2(300, 200));
                        }
                    }

                    if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Mesh Renderer")) {
                            editor.createNodeOfType("Render.MeshRenderer", Vec2(300, 200));
                        }
                    }

                    if (ImGui::CollapsingHeader("Subgraph", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Input")) {
                            editor.createNodeOfType("Subgraph.Input", Vec2(100, 200));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Output")) {
                            editor.createNodeOfType("Subgraph.Output", Vec2(500, 200));
                        }
                    }

                    if (editor.getCurrentSubgraphId() >= 0) {
                        ImGui::Separator();
                        ImGui::Text("Create interface node for current subgraph:");

                        static char interfaceName[128] = "Interface";
                        ImGui::InputText("Name", interfaceName, IM_ARRAYSIZE(interfaceName));

                        if (ImGui::Button("Add Input Node")) {
                            editor.addNode(interfaceName, "Subgraph.Input", Vec2(100, 200));
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Add Output Node")) {
                            editor.addNode(interfaceName, "Subgraph.Output", Vec2(500, 200));
                        }
                    }

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();

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