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
        std::cerr << "[ERREUR] GetNodeDefByType: type inconnu: " << type << std::endl;
        throw std::runtime_error("Type de noeud inconnu: " + type);
    }
    std::cout << "[LOG] GetNodeDefByType: " << type << std::endl;
    return nodeDefs[type];
}

Node* CreateNodeOfType(const std::string& type, const Vec2& pos) {
    static int nextId = 1;
    std::cout << "[LOG] CreateNodeOfType: type=" << type << ", pos=(" << pos.x << "," << pos.y << ")" << std::endl;
    NodeDefinition def = GetNodeDefByType(type);

    Node* node = nullptr;
    try {
        node = new Node(nextId++, def.name, type, pos);
    } catch (const std::exception& e) {
        std::cerr << "[ERREUR] Exception lors de la création du Node: " << e.what() << std::endl;
        throw;
    }
    node->iconSymbol = def.iconSymbol;

    int pinId = 1;
    for (const auto& input : def.inputs) {
        std::cout << "[LOG]   Ajout input pin: " << input.first << std::endl;
        node->inputs.push_back(Pin(pinId++, input.first, true, input.second));
    }

    for (const auto& output : def.outputs) {
        std::cout << "[LOG]   Ajout output pin: " << output.first << std::endl;
        node->outputs.push_back(Pin(pinId++, output.first, false, output.second));
    }

    std::cout << "[LOG] Node créé: " << node->name << " (id=" << node->id << ")" << std::endl;
    return node;
}

int main(int argc, char* argv[]) {
    try {
        std::cout << "[LOG] Initialisation de SDL..." << std::endl;
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
            std::cerr << "[ERREUR] SDL_Init a échoué ! SDL_Error: " << SDL_GetError() << std::endl;
            return -1;
        }

        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        SDL_Window* window = SDL_CreateWindow("Advanced Node Editor - Subgraphs Demo",
                                             SDL_WINDOWPOS_CENTERED,
                                             SDL_WINDOWPOS_CENTERED,
                                             1280, 720,
                                             window_flags);
        if (!window) {
            std::cerr << "[ERREUR] SDL_CreateWindow a échoué ! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_Quit();
            return -2;
        }

        SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
        if (!renderer) {
            std::cerr << "[ERREUR] SDL_CreateRenderer a échoué ! SDL_Error: " << SDL_GetError() << std::endl;
            SDL_DestroyWindow(window);
            SDL_Quit();
            return -3;
        }

        std::cout << "[LOG] Initialisation de ImGui..." << std::endl;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

        ImGui::StyleColorsDark();
        ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
        ImGui_ImplSDLRenderer2_Init(renderer);

        NodeEditor editor;
        std::cout << "[LOG] Enregistrement des types de noeuds..." << std::endl;

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

        std::cout << "[LOG] Configuration de l'éditeur..." << std::endl;
        editor.setGraphTitle("Houdini-Style Node Graph");
        editor.setGraphTitleStyle(NodeEditor::TitleStyle::Houdini);
        editor.setConnectionStyle(NodeEditor::ConnectionStyle::Bezier);
        editor.setConnectionThickness(2.5f);
        editor.enableNodeAvoidance(true);
        editor.enableMinimap(true);
        editor.setShowSubgraphBreadcrumbs(true);

        std::cout << "[LOG] Création des subgraphs..." << std::endl;
        UUID mainGraphUuid = editor.createSubgraphWithUUID("Main");
        std::cout << "[LOG]   mainGraphUuid = " << mainGraphUuid << std::endl;
        UUID mathSubgraphUuid = editor.createSubgraphWithUUID("Math Utilities");
        std::cout << "[LOG]   mathSubgraphUuid = " << mathSubgraphUuid << std::endl;
        UUID renderSubgraphUuid = editor.createSubgraphWithUUID("Rendering");
        std::cout << "[LOG]   renderSubgraphUuid = " << renderSubgraphUuid << std::endl;

        std::cout << "[LOG] Création des noeuds du graphe principal..." << std::endl;
        int boxNodeId = editor.addNode("Box", "Geometry.Box", Vec2(100, 100));
        std::cout << "[LOG]   boxNodeId = " << boxNodeId << std::endl;
        int sphereNodeId = editor.addNode("Sphere", "Geometry.Sphere", Vec2(100, 250));
        std::cout << "[LOG]   sphereNodeId = " << sphereNodeId << std::endl;

        std::cout << "[LOG] Création et configuration du subgraph de math..." << std::endl;
        editor.enterSubgraphByUUID(mathSubgraphUuid);
        std::cout << "[LOG]   Entré dans le subgraph mathSubgraphUuid" << std::endl;
        int addNodeId = editor.addNode("Add", "Math.Add", Vec2(300, 150));
        std::cout << "[LOG]   addNodeId = " << addNodeId << std::endl;
        int multiplyNodeId = editor.addNode("Multiply", "Math.Multiply", Vec2(600, 150));
        std::cout << "[LOG]   multiplyNodeId = " << multiplyNodeId << std::endl;
        int inputNodeId = editor.addNode("Input A", "Subgraph.Input", Vec2(50, 100));
        std::cout << "[LOG]   inputNodeId = " << inputNodeId << std::endl;
        int input2NodeId = editor.addNode("Input B", "Subgraph.Input", Vec2(50, 200));
        std::cout << "[LOG]   input2NodeId = " << input2NodeId << std::endl;
        int outputNodeId = editor.addNode("Output", "Subgraph.Output", Vec2(900, 150));
        std::cout << "[LOG]   outputNodeId = " << outputNodeId << std::endl;

        std::cout << "[LOG] Connexion des noeuds du subgraph de math..." << std::endl;
        std::cout << "[LOG]   inputNodeId->addNodeId" << std::endl;
        editor.addConnection(inputNodeId, 1, addNodeId, 1);
        std::cout << "[LOG]   input2NodeId->addNodeId" << std::endl;
        editor.addConnection(input2NodeId, 1, addNodeId, 2);
        std::cout << "[LOG]   addNodeId->multiplyNodeId (1)" << std::endl;
        editor.addConnection(addNodeId, 3, multiplyNodeId, 1);
        std::cout << "[LOG]   addNodeId->multiplyNodeId (2)" << std::endl;
        editor.addConnection(addNodeId, 3, multiplyNodeId, 2);
        std::cout << "[LOG]   multiplyNodeId->outputNodeId" << std::endl;
        editor.addConnection(multiplyNodeId, 3, outputNodeId, 1);

        std::cout << "[LOG]   Récupération du subgraph mathSubgraph" << std::endl;
        auto mathSubgraph = editor.getSubgraph(editor.getSubgraphId(mathSubgraphUuid));
        if (mathSubgraph) {
            std::cout << "[LOG]   Ajout interfaceInputs/Outputs à mathSubgraph" << std::endl;
            mathSubgraph->interfaceInputs.push_back((inputNodeId << 16) | 1);
            mathSubgraph->interfaceInputs.push_back((input2NodeId << 16) | 1);
            mathSubgraph->interfaceOutputs.push_back((outputNodeId << 16) | 1);
        } else {
            std::cerr << "[ERREUR] mathSubgraph est nul !" << std::endl;
        }
        editor.exitSubgraph();
        std::cout << "[LOG]   Sorti du subgraph mathSubgraphUuid" << std::endl;

        std::cout << "[LOG] Création et configuration du subgraph de rendu..." << std::endl;
        editor.enterSubgraphByUUID(renderSubgraphUuid);
        std::cout << "[LOG]   Entré dans le subgraph renderSubgraphUuid" << std::endl;
        int materialNodeId = editor.addNode("Material", "Material.Basic", Vec2(300, 150));
        std::cout << "[LOG]   materialNodeId = " << materialNodeId << std::endl;
        int rendererNodeId = editor.addNode("Renderer", "Render.MeshRenderer", Vec2(600, 150));
        std::cout << "[LOG]   rendererNodeId = " << rendererNodeId << std::endl;
        int geomInputNodeId = editor.addNode("Geometry Input", "Subgraph.Input", Vec2(100, 100));
        std::cout << "[LOG]   geomInputNodeId = " << geomInputNodeId << std::endl;
        int renderOutputNodeId = editor.addNode("Output", "Subgraph.Output", Vec2(900, 150));
        std::cout << "[LOG]   renderOutputNodeId = " << renderOutputNodeId << std::endl;

        std::cout << "[LOG] Connexion des noeuds du subgraph de rendu..." << std::endl;
        std::cout << "[LOG]   geomInputNodeId->rendererNodeId" << std::endl;
        editor.addConnection(geomInputNodeId, 1, rendererNodeId, 1);
        std::cout << "[LOG]   materialNodeId->rendererNodeId" << std::endl;
        editor.addConnection(materialNodeId, 2, rendererNodeId, 2);
        std::cout << "[LOG]   rendererNodeId->renderOutputNodeId" << std::endl;
        editor.addConnection(rendererNodeId, 3, renderOutputNodeId, 1);

        std::cout << "[LOG]   Récupération du subgraph renderSubgraph" << std::endl;
        auto renderSubgraph = editor.getSubgraph(editor.getSubgraphId(renderSubgraphUuid));
        if (renderSubgraph) {
            std::cout << "[LOG]   Ajout interfaceInputs/Outputs à renderSubgraph" << std::endl;
            renderSubgraph->interfaceInputs.push_back((geomInputNodeId << 16) | 1);
            renderSubgraph->interfaceOutputs.push_back((renderOutputNodeId << 16) | 1);
        } else {
            std::cerr << "[ERREUR] renderSubgraph est nul !" << std::endl;
        }
        editor.exitSubgraph();
        std::cout << "[LOG]   Sorti du subgraph renderSubgraphUuid" << std::endl;

        std::cout << "[LOG] Création des noeuds de subgraph dans le graphe principal..." << std::endl;
        Node* mathSubgraphNode = editor.createSubgraphNode(editor.getSubgraphId(mathSubgraphUuid),
                                                         "Math Utilities",
                                                         Vec2(300, 175));
        if (!mathSubgraphNode) {
            std::cerr << "[ERREUR] mathSubgraphNode est nul !" << std::endl;
        }
        int mathSubgraphNodeId = mathSubgraphNode ? mathSubgraphNode->id : -1;
        std::cout << "[LOG]   mathSubgraphNodeId = " << mathSubgraphNodeId << std::endl;

        Node* renderSubgraphNode = editor.createSubgraphNode(editor.getSubgraphId(renderSubgraphUuid),
                                                           "Rendering",
                                                           Vec2(500, 175));
        if (!renderSubgraphNode) {
            std::cerr << "[ERREUR] renderSubgraphNode est nul !" << std::endl;
        }
        int renderSubgraphNodeId = renderSubgraphNode ? renderSubgraphNode->id : -1;
        std::cout << "[LOG]   renderSubgraphNodeId = " << renderSubgraphNodeId << std::endl;

        std::cout << "[LOG] Connexion des noeuds dans le graphe principal..." << std::endl;
        std::cout << "[LOG]   boxNodeId->mathSubgraphNodeId" << std::endl;
        editor.addConnection(boxNodeId, 2, mathSubgraphNodeId, 1);
        std::cout << "[LOG]   sphereNodeId->mathSubgraphNodeId" << std::endl;
        editor.addConnection(sphereNodeId, 2, mathSubgraphNodeId, 2);
        std::cout << "[LOG]   mathSubgraphNodeId->renderSubgraphNodeId" << std::endl;
        editor.addConnection(mathSubgraphNodeId, 3, renderSubgraphNodeId, 1);


        enum class CurrentPanel {
            NodeEditor,
            NodeProperties,
            SubgraphNavigator
        };

        CurrentPanel currentSidePanel = CurrentPanel::SubgraphNavigator;
        std::vector<std::string> subgraphHistory;
        bool showProperties = true;

        std::cout << "[LOG] Entrée dans la boucle principale..." << std::endl;
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
                    if (ImGui::MenuItem("New Graph")) { std::cout << "[LOG] New Graph demandé" << std::endl; }
                    if (ImGui::MenuItem("Open Graph...")) { std::cout << "[LOG] Open Graph demandé" << std::endl; }
                    if (ImGui::MenuItem("Save Graph")) { std::cout << "[LOG] Save Graph demandé" << std::endl; }
                    if (ImGui::MenuItem("Save Graph As...")) { std::cout << "[LOG] Save Graph As demandé" << std::endl; }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Exit")) { std::cout << "[LOG] Exit demandé" << std::endl; done = true; }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit")) {
                    if (ImGui::MenuItem("Undo", "CTRL+Z")) { std::cout << "[LOG] Undo" << std::endl; }
                    if (ImGui::MenuItem("Redo", "CTRL+Y")) { std::cout << "[LOG] Redo" << std::endl; }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "CTRL+X")) { std::cout << "[LOG] Cut" << std::endl; }
                    if (ImGui::MenuItem("Copy", "CTRL+C")) { std::cout << "[LOG] Copy" << std::endl; }
                    if (ImGui::MenuItem("Paste", "CTRL+V")) { std::cout << "[LOG] Paste" << std::endl; }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("View")) {
                    if (ImGui::MenuItem("Zoom to Fit", "F")) { std::cout << "[LOG] Zoom to Fit" << std::endl; editor.zoomToFit(); }
                    if (ImGui::MenuItem("Center View", "C")) { std::cout << "[LOG] Center View" << std::endl; editor.centerView(); }
                    ImGui::Separator();
                    bool showMinimap = editor.isMinimapEnabled();
                    if (ImGui::MenuItem("Minimap", nullptr, &showMinimap)) {
                        std::cout << "[LOG] Minimap toggled: " << (showMinimap ? "ON" : "OFF") << std::endl;
                        editor.enableMinimap(showMinimap);
                    }
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Subgraph")) {
                    if (ImGui::MenuItem("Main Graph")) {
                        std::cout << "[LOG] Navigation vers Main Graph" << std::endl;
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                    }
                    if (ImGui::MenuItem("Math Utilities")) {
                        std::cout << "[LOG] Navigation vers Math Utilities" << std::endl;
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        editor.enterSubgraphByUUID(mathSubgraphUuid);
                    }
                    if (ImGui::MenuItem("Rendering")) {
                        std::cout << "[LOG] Navigation vers Rendering" << std::endl;
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        editor.enterSubgraphByUUID(renderSubgraphUuid);
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Create New Subgraph...")) {
                        std::cout << "[LOG] Création d'un nouveau subgraph" << std::endl;
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
                std::cout << "[LOG] Bouton Main Graph cliqué" << std::endl;
                while (editor.getCurrentSubgraphId() >= 0) {
                    editor.exitSubgraph();
                }
            }

            ImGui::SameLine();
            if (editor.getCurrentSubgraphId() >= 0) {
                if (ImGui::Button("Exit Subgraph")) {
                    std::cout << "[LOG] Bouton Exit Subgraph cliqué" << std::endl;
                    editor.exitSubgraph();
                }

                ImGui::SameLine();
                auto currentSubgraph = editor.getSubgraph(editor.getCurrentSubgraphId());
                if (currentSubgraph) {
                    ImGui::Text("Current: %s", currentSubgraph->name.c_str());
                } else {
                    ImGui::Text("Current: [ERREUR: subgraph nul]");
                    std::cerr << "[ERREUR] getSubgraph retourne nullptr pour id=" << editor.getCurrentSubgraphId() << std::endl;
                }
            }

            ImGui::EndChild();

            try {
                editor.beginFrame();
                editor.render();
                editor.endFrame();
            } catch (const std::exception& e) {
                std::cerr << "[ERREUR] Exception dans editor.beginFrame/render/endFrame: " << e.what() << std::endl;
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
                            std::cout << "[LOG] Navigation via panneau latéral vers Main Graph" << std::endl;
                            while (editor.getCurrentSubgraphId() >= 0) {
                                editor.exitSubgraph();
                            }
                        }

                        if (ImGui::Button("Math Utilities")) {
                            std::cout << "[LOG] Navigation via panneau latéral vers Math Utilities" << std::endl;
                            while (editor.getCurrentSubgraphId() >= 0) {
                                editor.exitSubgraph();
                            }
                            editor.enterSubgraphByUUID(mathSubgraphUuid);
                        }

                        if (ImGui::Button("Rendering")) {
                            std::cout << "[LOG] Navigation via panneau latéral vers Rendering" << std::endl;
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
                            std::cout << "[LOG] Création d'un subgraph nommé : " << newSubgraphName << std::endl;
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
                                            std::cout << "[LOG] Entrée dans le subgraph depuis propriétés" << std::endl;
                                            editor.enterSubgraph(subgraphId);
                                        }
                                    } else {
                                        std::cerr << "[ERREUR] subgraph est nul dans propriétés (id=" << subgraphId << ")" << std::endl;
                                    }
                                } else {
                                    std::cerr << "[ERREUR] subgraphId < 0 dans propriétés" << std::endl;
                                }
                            }
                        } else {
                            std::cerr << "[ERREUR] node est nul dans propriétés" << std::endl;
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
                            std::cout << "[LOG] Ajout d'un noeud Math.Add" << std::endl;
                            editor.createNodeOfType("Math.Add", Vec2(300, 200));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Multiply")) {
                            std::cout << "[LOG] Ajout d'un noeud Math.Multiply" << std::endl;
                            editor.createNodeOfType("Math.Multiply", Vec2(300, 300));
                        }
                    }

                    if (ImGui::CollapsingHeader("Geometry", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Box")) {
                            std::cout << "[LOG] Ajout d'un noeud Geometry.Box" << std::endl;
                            editor.createNodeOfType("Geometry.Box", Vec2(300, 200));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Sphere")) {
                            std::cout << "[LOG] Ajout d'un noeud Geometry.Sphere" << std::endl;
                            editor.createNodeOfType("Geometry.Sphere", Vec2(300, 300));
                        }
                    }

                    if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Basic Material")) {
                            std::cout << "[LOG] Ajout d'un noeud Material.Basic" << std::endl;
                            editor.createNodeOfType("Material.Basic", Vec2(300, 200));
                        }
                    }

                    if (ImGui::CollapsingHeader("Render", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Mesh Renderer")) {
                            std::cout << "[LOG] Ajout d'un noeud Render.MeshRenderer" << std::endl;
                            editor.createNodeOfType("Render.MeshRenderer", Vec2(300, 200));
                        }
                    }

                    if (ImGui::CollapsingHeader("Subgraph", ImGuiTreeNodeFlags_DefaultOpen)) {
                        if (ImGui::Button("Input")) {
                            std::cout << "[LOG] Ajout d'un noeud Subgraph.Input" << std::endl;
                            editor.createNodeOfType("Subgraph.Input", Vec2(100, 200));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Output")) {
                            std::cout << "[LOG] Ajout d'un noeud Subgraph.Output" << std::endl;
                            editor.createNodeOfType("Subgraph.Output", Vec2(500, 200));
                        }
                    }

                    if (editor.getCurrentSubgraphId() >= 0) {
                        ImGui::Separator();
                        ImGui::Text("Create interface node for current subgraph:");

                        static char interfaceName[128] = "Interface";
                        ImGui::InputText("Name", interfaceName, IM_ARRAYSIZE(interfaceName));

                        if (ImGui::Button("Add Input Node")) {
                            std::cout << "[LOG] Ajout d'un noeud d'interface Input dans le subgraph courant" << std::endl;
                            editor.addNode(interfaceName, "Subgraph.Input", Vec2(100, 200));
                        }

                        ImGui::SameLine();
                        if (ImGui::Button("Add Output Node")) {
                            std::cout << "[LOG] Ajout d'un noeud d'interface Output dans le subgraph courant" << std::endl;
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

        std::cout << "[LOG] Fermeture de l'application..." << std::endl;
        ImGui_ImplSDLRenderer2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        std::cout << "[LOG] Application terminée." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "[ERREUR] Exception attrapée dans main: " << e.what() << std::endl;
        return -100;
    } catch (...) {
        std::cerr << "[ERREUR] Exception inconnue attrapée dans main." << std::endl;
        return -101;
    }
}