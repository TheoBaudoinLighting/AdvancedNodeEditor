#include <SDL.h>
#include <SDL_opengl.h>
#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "../AdvancedNodeEditor/NodeEditor.h"

struct NodeTypeDefinition {
    std::string name;
    std::string category;
    ANE::Color color;
    std::vector<std::pair<std::string, ANE::PinType>> inputs;
    std::vector<std::pair<std::string, ANE::PinType>> outputs;
};

class Application {
private:
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    bool m_running;
    
    ANE::NodeEditor m_nodeEditor;
    std::vector<ANE::UUID> m_nodeUuids;
    
    std::vector<NodeTypeDefinition> m_nodeTypes;
    
    void setupNodeTypes() {
        m_nodeTypes = {
            {
                "Nombre", "Entrée", ANE::Color(0.2f, 0.4f, 0.8f),
                {},
                {{"Valeur", ANE::PinType::Blue}}
            },
            {
                "Addition", "Math", ANE::Color(0.8f, 0.4f, 0.2f),
                {{"A", ANE::PinType::Blue}, {"B", ANE::PinType::Blue}},
                {{"Résultat", ANE::PinType::Blue}}
            },
            {
                "Multiplication", "Math", ANE::Color(0.8f, 0.4f, 0.2f),
                {{"A", ANE::PinType::Blue}, {"B", ANE::PinType::Blue}},
                {{"Résultat", ANE::PinType::Blue}}
            },
            {
                "Vecteur", "Géométrie", ANE::Color(0.2f, 0.8f, 0.4f),
                {{"X", ANE::PinType::Blue}, {"Y", ANE::PinType::Blue}, {"Z", ANE::PinType::Blue}},
                {{"Vec", ANE::PinType::Green}}
            },
            {
                "Affichage", "Sortie", ANE::Color(0.8f, 0.2f, 0.4f),
                {{"Valeur", ANE::PinType::Blue}},
                {}
            }
        };
        
        for (const auto& nodeType : m_nodeTypes) {
            m_nodeEditor.registerNodeType(
                nodeType.name,
                nodeType.category,
                "Description de " + nodeType.name,
                [this, nodeType](const ANE::Vec2& pos) -> ANE::Node* {
                    ANE::UUID uuid;
                    int nodeId = m_nodeEditor.addNode(nodeType.name, nodeType.name, pos);
                    ANE::Node* node = m_nodeEditor.getNode(nodeId);
                    
                    if (node) {
                        for (const auto& input : nodeType.inputs) {
                            m_nodeEditor.addPin(nodeId, input.first, true, input.second);
                        }
                        
                        for (const auto& output : nodeType.outputs) {
                            m_nodeEditor.addPin(nodeId, output.first, false, output.second);
                        }
                        
                        m_nodeUuids.push_back(node->uuid);
                    }
                    
                    return node;
                }
            );
        }
    }
    
    void createExampleNodes() {
        ANE::UUID node1 = m_nodeEditor.addNodeWithUUID("Nombre", "Nombre", ANE::Vec2(100, 100));
        ANE::UUID node2 = m_nodeEditor.addNodeWithUUID("Nombre", "Nombre", ANE::Vec2(100, 200));
        ANE::UUID node3 = m_nodeEditor.addNodeWithUUID("Addition", "Addition", ANE::Vec2(300, 150));
        ANE::UUID node4 = m_nodeEditor.addNodeWithUUID("Affichage", "Affichage", ANE::Vec2(500, 150));
        
        ANE::Node* node1Ptr = m_nodeEditor.getNodeByUUID(node1);
        ANE::Node* node2Ptr = m_nodeEditor.getNodeByUUID(node2);
        ANE::Node* node3Ptr = m_nodeEditor.getNodeByUUID(node3);
        ANE::Node* node4Ptr = m_nodeEditor.getNodeByUUID(node4);
        
        if (node1Ptr && node2Ptr && node3Ptr && node4Ptr) {
            ANE::UUID node1OutPin = m_nodeEditor.addPinWithUUID(node1Ptr->id, "Valeur", false, ANE::PinType::Blue);
            ANE::UUID node2OutPin = m_nodeEditor.addPinWithUUID(node2Ptr->id, "Valeur", false, ANE::PinType::Blue);
            
            ANE::UUID node3InPin1 = m_nodeEditor.addPinWithUUID(node3Ptr->id, "A", true, ANE::PinType::Blue);
            ANE::UUID node3InPin2 = m_nodeEditor.addPinWithUUID(node3Ptr->id, "B", true, ANE::PinType::Blue);
            ANE::UUID node3OutPin = m_nodeEditor.addPinWithUUID(node3Ptr->id, "Résultat", false, ANE::PinType::Blue);
            
            ANE::UUID node4InPin = m_nodeEditor.addPinWithUUID(node4Ptr->id, "Valeur", true, ANE::PinType::Blue);
            
            m_nodeEditor.addConnectionWithUUIDByUUID(node1, node1OutPin, node3, node3InPin1);
            m_nodeEditor.addConnectionWithUUIDByUUID(node2, node2OutPin, node3, node3InPin2);
            m_nodeEditor.addConnectionWithUUIDByUUID(node3, node3OutPin, node4, node4InPin);
            
            ANE::UUID groupUuid = m_nodeEditor.addGroupWithUUID("Calcul", ANE::Vec2(80, 80), ANE::Vec2(350, 180));
            m_nodeEditor.addNodeToGroupByUUID(node1, groupUuid);
            m_nodeEditor.addNodeToGroupByUUID(node2, groupUuid);
            m_nodeEditor.addNodeToGroupByUUID(node3, groupUuid);
        }
        
        m_nodeEditor.createSubgraphWithUUID("Sous-graphe");
    }
    
public:
    Application() : m_window(nullptr), m_glContext(nullptr), m_running(false) {}
    
    bool initialize() {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            std::cerr << "Erreur SDL_Init: " << SDL_GetError() << std::endl;
            return false;
        }
        
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        m_window = SDL_CreateWindow("Démo NodeEditor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
        if (!m_window) {
            std::cerr << "Erreur création fenêtre: " << SDL_GetError() << std::endl;
            return false;
        }
        
        m_glContext = SDL_GL_CreateContext(m_window);
        SDL_GL_MakeCurrent(m_window, m_glContext);
        SDL_GL_SetSwapInterval(1);
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
       // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        
        ImGui::StyleColorsDark();
        
        ImGui_ImplSDL2_InitForOpenGL(m_window, m_glContext);
        ImGui_ImplOpenGL3_Init("#version 130");
        
        setupNodeTypes();
        createExampleNodes();
        
        m_running = true;
        return true;
    }
    
    void run() {
        while (m_running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                ImGui_ImplSDL2_ProcessEvent(&event);
                if (event.type == SDL_QUIT) {
                    m_running = false;
                }
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                    event.window.windowID == SDL_GetWindowID(m_window)) {
                    m_running = false;
                }
            }
            
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
            
            //ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
            
            ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(300, 720), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Palette de nœuds")) {
                if (ImGui::CollapsingHeader("Création de nœuds", ImGuiTreeNodeFlags_DefaultOpen)) {
                    std::string lastCategory;
                    for (const auto& nodeType : m_nodeTypes) {
                        if (lastCategory != nodeType.category) {
                            lastCategory = nodeType.category;
                            ImGui::Separator();
                            ImGui::Text("%s", lastCategory.c_str());
                        }
                        
                        ImVec2 buttonSize(ImGui::GetContentRegionAvail().x, 0);
                        ImGui::PushStyleColor(ImGuiCol_Button, 
                            ImVec4(nodeType.color.r, nodeType.color.g, nodeType.color.b, 0.7f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, 
                            ImVec4(nodeType.color.r + 0.1f, nodeType.color.g + 0.1f, nodeType.color.b + 0.1f, 0.8f));
                        
                        if (ImGui::Button(nodeType.name.c_str(), buttonSize)) {
                            ANE::Node* node = m_nodeEditor.createNodeOfType(nodeType.name, ANE::Vec2(400, 300));
                            if (node) {
                                m_nodeUuids.push_back(node->uuid);
                            }
                        }
                        
                        ImGui::PopStyleColor(2);
                    }
                }
                
                ImGui::Separator();
                
                if (ImGui::CollapsingHeader("Opérations", ImGuiTreeNodeFlags_DefaultOpen)) {
                    if (ImGui::Button("Recentrer vue", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                        m_nodeEditor.centerView();
                    }
                    
                    if (ImGui::Button("Créer groupe", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                        m_nodeEditor.addGroupWithUUID("Nouveau groupe", ANE::Vec2(300, 300), ANE::Vec2(250, 200));
                    }
                    
                    if (ImGui::Button("Mode debug", ImVec2(ImGui::GetContentRegionAvail().x, 0))) {
                        m_nodeEditor.setDebugMode(!m_nodeEditor.isDebugMode());
                    }
                }
                
                ImGui::Separator();
                
                if (ImGui::CollapsingHeader("Informations", ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Nœuds: %zu", m_nodeUuids.size());
                    ImGui::Text("Position vue: (%.1f, %.1f)", 
                                m_nodeEditor.getViewPosition().x, m_nodeEditor.getViewPosition().y);
                    ImGui::Text("Échelle vue: %.2f", m_nodeEditor.getViewScale());
                    
                    std::vector<int> selectedNodes = m_nodeEditor.getSelectedNodes();
                    ImGui::Text("Nœuds sélectionnés: %zu", selectedNodes.size());
                    
                    for (int nodeId : selectedNodes) {
                        ANE::Node* node = m_nodeEditor.getNode(nodeId);
                        if (node) {
                            ImGui::BulletText("%s (ID: %d)", node->name.c_str(), node->id);
                        }
                    }
                }
            }
            ImGui::End();
            
            ImGui::SetNextWindowPos(ImVec2(300, 0), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(980, 720), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Éditeur de nœuds")) {
                m_nodeEditor.beginFrame();
                m_nodeEditor.render();
                m_nodeEditor.endFrame();
            }
            ImGui::End();
            
            ImGui::Render();
            
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            SDL_GL_SwapWindow(m_window);
        }
    }
    
    void shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();
        
        SDL_GL_DeleteContext(m_glContext);
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    Application app;
    
    if (!app.initialize()) {
        return 1;
    }
    
    app.run();
    app.shutdown();
    
    return 0;
}