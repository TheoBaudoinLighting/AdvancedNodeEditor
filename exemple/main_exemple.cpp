#include "../AdvancedNodeEditor/NodeEditor.h"
#include "../AdvancedNodeEditor/Editor/Controller/NodeEditorController.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <cmath>
#include <functional>
#include <memory>
#include <stack>
#include <map>
#include <string>
#include <cstdio>

#define LOG_DEBUG(fmt, ...) printf("[DEBUG] %s:%d - " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__); fflush(stdout)

namespace ANE {
    class INodeEditorController;
}

bool canConnect(const ANE::Pin& sourcePin, const ANE::Pin& destinationPin) {
    if (sourcePin.type == destinationPin.type)
        return true;

    if (sourcePin.type == ANE::PinType::Yellow && destinationPin.type == ANE::PinType::Green)
        return true;
    if (sourcePin.type == ANE::PinType::Green && destinationPin.type == ANE::PinType::Yellow)
        return true;

    if (sourcePin.type == ANE::PinType::Orange && destinationPin.type == ANE::PinType::White)
        return true;
    if (sourcePin.type == ANE::PinType::White && destinationPin.type == ANE::PinType::Orange)
        return true;

    if (sourcePin.type == ANE::PinType::Blue && destinationPin.type == ANE::PinType::Blue)
        return true;

    return false;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    LOG_DEBUG("Application started");

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        LOG_DEBUG("SDL_Init error: %s", SDL_GetError());
        printf("Error: %s\n", SDL_GetError());
        return 1;
    }
    LOG_DEBUG("SDL initialized successfully");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Advanced Node Editor with Subgraphs", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr) {
        LOG_DEBUG("Window creation error: %s", SDL_GetError());
        printf("Window creation error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    LOG_DEBUG("SDL window created successfully");

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        LOG_DEBUG("OpenGL context creation error: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }
    LOG_DEBUG("OpenGL context created successfully");
    
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    LOG_DEBUG("ImGui context created");
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");
    LOG_DEBUG("ImGui initialized successfully");

    LOG_DEBUG("Creating editor controller");
    std::shared_ptr<ANE::INodeEditorController> controller = std::make_shared<ANE::NodeEditorController>();
    LOG_DEBUG("Creating node editor");
    ANE::NodeEditor editor;
    LOG_DEBUG("Node editor created successfully");

    LOG_DEBUG("Ensuring editor starts with a clean subgraph stack");
    while (editor.getCurrentSubgraphId() >= 0) {
        LOG_DEBUG("Exiting unexpected active subgraph (ID: %d)", editor.getCurrentSubgraphId());
        editor.exitSubgraph();
    }
    LOG_DEBUG("Editor is now in main graph level");

    ANE::EditorStyle style;
    style.backgroundColor = ANE::Color(0.10f, 0.11f, 0.12f, 1.00f);
    style.gridColor = ANE::Color(0.16f, 0.17f, 0.18f, 0.50f);
    style.selectionColor = ANE::Color(0.00f, 0.44f, 0.80f, 0.30f);
    style.gridSpacing = 20.0f;
    style.nodeRounding = 5.0f;
    style.pinRadius = 4.0f;
    style.connectionThickness = 2.5f;

    ANE::NodeStyle geometryStyle;
    geometryStyle.baseColor = ANE::Color(0.20f, 0.25f, 0.30f, 1.0f);
    geometryStyle.headerColor = ANE::Color(0.18f, 0.22f, 0.26f, 0.8f);
    geometryStyle.accentColor = ANE::Color(0.20f, 0.65f, 0.90f, 1.0f);
    geometryStyle.borderColor = ANE::Color(0.22f, 0.28f, 0.35f, 1.0f);
    geometryStyle.selectedColor = ANE::Color(0.20f, 0.75f, 1.00f, 1.0f);
    geometryStyle.hoveredColor = ANE::Color(0.25f, 0.70f, 0.95f, 1.0f);
    geometryStyle.glowColor = ANE::Color(0.20f, 0.60f, 0.90f, 0.2f);
    style.nodeStyles["Geometry"] = geometryStyle;

    ANE::NodeStyle materialStyle;
    materialStyle.baseColor = ANE::Color(0.30f, 0.22f, 0.25f, 1.0f);
    materialStyle.headerColor = ANE::Color(0.26f, 0.18f, 0.22f, 0.8f);
    materialStyle.accentColor = ANE::Color(0.90f, 0.40f, 0.50f, 1.0f);
    materialStyle.borderColor = ANE::Color(0.35f, 0.25f, 0.28f, 1.0f);
    materialStyle.selectedColor = ANE::Color(1.00f, 0.50f, 0.60f, 1.0f);
    materialStyle.hoveredColor = ANE::Color(0.95f, 0.45f, 0.55f, 1.0f);
    materialStyle.glowColor = ANE::Color(0.90f, 0.30f, 0.40f, 0.2f);
    style.nodeStyles["Material"] = materialStyle;

    ANE::NodeStyle utilityStyle;
    utilityStyle.baseColor = ANE::Color(0.28f, 0.28f, 0.30f, 1.0f);
    utilityStyle.headerColor = ANE::Color(0.24f, 0.24f, 0.26f, 0.8f);
    utilityStyle.accentColor = ANE::Color(0.80f, 0.80f, 0.90f, 1.0f);
    utilityStyle.borderColor = ANE::Color(0.32f, 0.32f, 0.35f, 1.0f);
    utilityStyle.selectedColor = ANE::Color(0.85f, 0.85f, 0.95f, 1.0f);
    utilityStyle.hoveredColor = ANE::Color(0.82f, 0.82f, 0.92f, 1.0f);
    utilityStyle.glowColor = ANE::Color(0.75f, 0.75f, 0.85f, 0.2f);
    style.nodeStyles["Utility"] = utilityStyle;

    ANE::NodeStyle subgraphStyle;
    subgraphStyle.baseColor = ANE::Color(0.20f, 0.30f, 0.20f, 1.0f);
    subgraphStyle.headerColor = ANE::Color(0.18f, 0.26f, 0.18f, 0.8f);
    subgraphStyle.accentColor = ANE::Color(0.40f, 0.85f, 0.40f, 1.0f);
    subgraphStyle.borderColor = ANE::Color(0.22f, 0.35f, 0.22f, 1.0f);
    subgraphStyle.selectedColor = ANE::Color(0.50f, 0.95f, 0.50f, 1.0f);
    subgraphStyle.hoveredColor = ANE::Color(0.45f, 0.90f, 0.45f, 1.0f);
    subgraphStyle.glowColor = ANE::Color(0.40f, 0.80f, 0.40f, 0.2f);
    style.nodeStyles["Subgraph"] = subgraphStyle;

    ANE::PinStyle vec3Style;
    vec3Style.color = ANE::Color(0.22f, 0.70f, 0.40f, 1.0f);
    vec3Style.hoverColor = ANE::Color(0.32f, 0.80f, 0.50f, 1.0f);
    vec3Style.connectedColor = ANE::Color(0.42f, 0.90f, 0.60f, 1.0f);
    style.pinStyles["Vec3"] = vec3Style;

    style.connectionStyle.baseColor = ANE::Color(0.600f, 0.650f, 0.700f, 0.627f);
    style.connectionStyle.selectedColor = ANE::Color(0.850f, 0.800f, 1.000f, 0.941f);
    style.connectionStyle.hoveredColor = ANE::Color(0.750f, 0.750f, 0.880f, 0.863f);
    style.connectionStyle.validColor = ANE::Color(0.750f, 0.950f, 0.800f, 0.902f);
    style.connectionStyle.invalidColor = ANE::Color(0.950f, 0.750f, 0.750f, 0.784f);

    int groupImport = editor.addGroup("1. Import & Preparation", ANE::Vec2(250, 10), ANE::Vec2(280, 380));
    int groupModeling = editor.addGroup("2. Geometric Modeling", ANE::Vec2(200, 420), ANE::Vec2(200, 400));
    int groupTextures = editor.addGroup("3. Textures & Materials", ANE::Vec2(450, 420), ANE::Vec2(200, 400));
    int groupSubgraphs = editor.addGroup("4. Subgraphs", ANE::Vec2(680, 200), ANE::Vec2(250, 250));
    int groupRendering = editor.addGroup("5. Rendering & Export", ANE::Vec2(300, 850), ANE::Vec2(180, 380));

    int geometrySubgraphId;
    int texturingSubgraphId;
    int shadingSubgraphId;

    LOG_DEBUG("Creating main nodes");
    int nodeCADFile = editor.addNode("Import CAD", "Utility", ANE::Vec2(300, 50));
    int nodeFBXFile = editor.addNode("Import FBX", "Utility", ANE::Vec2(300, 140));
    int nodeConvergence = editor.addNode("Convergence", "Utility", ANE::Vec2(300, 230));
    int nodePreprocess = editor.addNode("Preprocess", "Utility", ANE::Vec2(300, 320));

    LOG_DEBUG("Creating geometry subgraph");
    geometrySubgraphId = editor.createSubgraph("Geometry Processing");
    LOG_DEBUG("Geometry subgraph created with ID: %d", geometrySubgraphId);
    
    LOG_DEBUG("Creating texturing subgraph");
    texturingSubgraphId = editor.createSubgraph("Texturing Pipeline"); 
    LOG_DEBUG("Texturing subgraph created with ID: %d", texturingSubgraphId);
    
    LOG_DEBUG("Creating shading subgraph");
    shadingSubgraphId = editor.createSubgraph("Shading System");
    LOG_DEBUG("Shading subgraph created with ID: %d", shadingSubgraphId);

    LOG_DEBUG("Creating subgraph nodes in main graph");
    
    LOG_DEBUG("Creating subgraph nodes with manual approach");
    int geometryNodeId = editor.addNode("Geometry Processing", "Subgraph", ANE::Vec2(720, 230));
    int texturingNodeId = editor.addNode("Texturing Pipeline", "Subgraph", ANE::Vec2(720, 320));
    int shadingNodeId = editor.addNode("Shading System", "Subgraph", ANE::Vec2(720, 410));
    
    LOG_DEBUG("Created node IDs: Geometry: %d, Texturing: %d, Shading: %d", 
              geometryNodeId, texturingNodeId, shadingNodeId);
    
    controller->updateNode(geometryNodeId, [geometrySubgraphId](ANE::Node& node) {
        node.setAsSubgraph(true, geometrySubgraphId);
        node.setIconSymbol("G");
    });
    LOG_DEBUG("geometryNode configured via controller, ID: %d", geometryNodeId);
    
    controller->updateNode(texturingNodeId, [texturingSubgraphId](ANE::Node& node) {
        node.setAsSubgraph(true, texturingSubgraphId);
        node.setIconSymbol("T");
    });
    LOG_DEBUG("texturingNode configured via controller, ID: %d", texturingNodeId);
    
    controller->updateNode(shadingNodeId, [shadingSubgraphId](ANE::Node& node) {
        node.setAsSubgraph(true, shadingSubgraphId);
        node.setIconSymbol("S");
    });
    LOG_DEBUG("shadingNode configured via controller, ID: %d", shadingNodeId);
    
    LOG_DEBUG("Storing node-subgraph associations");
    std::map<int, int> nodeToSubgraph;
    nodeToSubgraph[geometryNodeId] = geometrySubgraphId;
    nodeToSubgraph[texturingNodeId] = texturingSubgraphId;
    nodeToSubgraph[shadingNodeId] = shadingSubgraphId;
    LOG_DEBUG("Node-subgraph associations stored in memory map");
    
    LOG_DEBUG("Final subgraph node IDs - Geometry: %d, Texturing: %d, Shading: %d", 
              geometryNodeId, texturingNodeId, shadingNodeId);
    
    if (geometryNodeId == texturingNodeId || 
        geometryNodeId == shadingNodeId ||
        texturingNodeId == shadingNodeId) {
        LOG_DEBUG("ERROR: Multiple subgraph nodes have same ID - possible conflict!");
    } else {
        LOG_DEBUG("All subgraph nodes have unique IDs - good!");
    }
    
    LOG_DEBUG("Creating content for each subgraph");
    
    LOG_DEBUG("Skipping subgraph content creation - will be added manually by user");
    LOG_DEBUG("All subgraphs are prepared but empty");
    
    int nodeTextureSet = editor.addNode("Texture Set", "Material", ANE::Vec2(500, 550));
    int nodeShaderPBR = editor.addNode("PBR Shader", "Material", ANE::Vec2(500, 640));
    int nodeVariants = editor.addNode("Variants", "Material", ANE::Vec2(500, 730));

    int nodeLighting = editor.addNode("Lighting", "Utility", ANE::Vec2(350, 890));
    int nodePostprocess = editor.addNode("Post-Process", "Utility", ANE::Vec2(350, 980));
    int nodePreview = editor.addNode("Preview", "Utility", ANE::Vec2(350, 1070));
    int nodeExport = editor.addNode("Export GLTF", "Utility", ANE::Vec2(350, 1160));

    editor.addNodeToGroup(nodeCADFile, groupImport);
    editor.addNodeToGroup(nodeFBXFile, groupImport);
    editor.addNodeToGroup(nodeConvergence, groupImport);
    editor.addNodeToGroup(nodePreprocess, groupImport);

    editor.addNodeToGroup(nodeTextureSet, groupTextures);
    editor.addNodeToGroup(nodeShaderPBR, groupTextures);
    editor.addNodeToGroup(nodeVariants, groupTextures);

    editor.addNodeToGroup(nodeLighting, groupRendering);
    editor.addNodeToGroup(nodePostprocess, groupRendering);
    editor.addNodeToGroup(nodePreview, groupRendering);
    editor.addNodeToGroup(nodeExport, groupRendering);

    editor.addNodeToGroup(geometryNodeId, groupSubgraphs);
    editor.addNodeToGroup(texturingNodeId, groupSubgraphs);
    editor.addNodeToGroup(shadingNodeId, groupSubgraphs);

    LOG_DEBUG("Skipping subgraph content creation to avoid crash");
    LOG_DEBUG("Only displaying main graph nodes");

    bool showOptions = false;
    bool showHelp = false;
    float zoom = 1.0f;
    bool firstCycle = true;
    bool done = false;

    LOG_DEBUG("Starting main loop");
    while (!done) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) {
                LOG_DEBUG("SDL_QUIT event received, closing application");
                done = true;
            }
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window)) {
                LOG_DEBUG("SDL_WINDOWEVENT_CLOSE event received, closing application");
                done = true;
            }
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 300, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImGui::Begin("ModFlow - Advanced Node Graph with Subgraphs", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);

        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("New Project", "Ctrl+N")) {}
                if (ImGui::MenuItem("Open Project...", "Ctrl+O")) {}
                if (ImGui::MenuItem("Save", "Ctrl+S")) {}
                if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Import Model...")) {}
                if (ImGui::MenuItem("Export Selection...")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4")) {
                    done = true;
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit")) {
                if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
                if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "Ctrl+X")) {}
                if (ImGui::MenuItem("Copy", "Ctrl+C")) {}
                if (ImGui::MenuItem("Paste", "Ctrl+V")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Duplicate Nodes", "Ctrl+D")) {}
                if (ImGui::MenuItem("Delete Nodes", "Delete")) {}
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View")) {
                if (ImGui::MenuItem("Center View", "F")) {
                    editor.centerView();
                }

                if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                    zoom = std::min(zoom * 1.1f, 2.0f);
                    editor.setViewScale(zoom);
                }

                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    zoom = std::max(zoom * 0.9f, 0.5f);
                    editor.setViewScale(zoom);
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Reset Zoom", "Ctrl+0")) {
                    zoom = 1.0f;
                    editor.setViewScale(zoom);
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Subgraphs")) {
                if (ImGui::MenuItem("Return to Main Graph") && editor.getCurrentSubgraphId() >= 0) {
                    editor.exitSubgraph();
                }
                ImGui::Separator();
                
                if (ImGui::MenuItem("Geometry Processing")) {
                    editor.enterSubgraph(geometrySubgraphId);
                }
                if (ImGui::MenuItem("Texturing Pipeline")) {
                    editor.enterSubgraph(texturingSubgraphId);
                }
                if (ImGui::MenuItem("Shading System")) {
                    editor.enterSubgraph(shadingSubgraphId);
                }
                
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("Documentation", "F1")) {}
                if (ImGui::MenuItem("Tutorials")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("About")) {
                    showHelp = true;
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenuBar();
        }

        int currentSubgraphId = editor.getCurrentSubgraphId();
        if (currentSubgraphId >= 0) {
            ANE::Subgraph* subgraph = editor.getSubgraph(currentSubgraphId);
            if (subgraph) {
                ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "Editing Subgraph: %s", subgraph->name.c_str());
                
                if (ImGui::Button("Return to Main Graph")) {
                    editor.exitSubgraph();
                }
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Double-click navigation disabled to prevent crashes");
            }
        }

        static bool wasDoubleClicked = false;
        if (ImGui::IsMouseDoubleClicked(0) && !wasDoubleClicked) {
            wasDoubleClicked = true;
            LOG_DEBUG("Double-click detected");
            auto selectedNodes = editor.getSelectedNodes();
            if (selectedNodes.size() == 1) {
                LOG_DEBUG("One node selected, ID: %d", selectedNodes[0]);
                ANE::Node* node = editor.getNode(selectedNodes[0]);
                if (node == nullptr) {
                    LOG_DEBUG("ERROR: Selected node is nullptr");
                } else if (node->isSubgraph) {
                    LOG_DEBUG("Selected node is a subgraph, ID: %d, subgraphId: %d", node->id, node->subgraphId);
                    if (editor.enterSubgraph(node->subgraphId)) {
                        LOG_DEBUG("Entered subgraph successfully, ID: %d", node->subgraphId);
                    } else {
                        LOG_DEBUG("Failed to enter subgraph, ID: %d", node->subgraphId);
                    }
                } else {
                    LOG_DEBUG("Selected node is not a subgraph");
                }
            } else {
                LOG_DEBUG("No node selected or multiple selection (%zu nodes)", selectedNodes.size());
            }
        } else if (!ImGui::IsMouseDoubleClicked(0)) {
            wasDoubleClicked = false;
        }

        editor.beginFrame();
        editor.render();
        editor.endFrame();

        if (firstCycle) {
            LOG_DEBUG("First frame, centering view");
            editor.centerView();
            firstCycle = false;
            LOG_DEBUG("View centered successfully");
        }
        ImGui::End();

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 300, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImGui::Begin("Control Panel", nullptr);

        if (ImGui::CollapsingHeader("Node Library", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 16.0f);

            if (ImGui::TreeNode("Import & Export")) {
                ImGui::Text("Import CAD");
                ImGui::Text("Import FBX");
                ImGui::Text("Import OBJ");
                ImGui::Text("Export GLTF");
                ImGui::Text("Export FBX");
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Geometry")) {
                ImGui::Text("Decimation");
                ImGui::Text("Topology Correction");
                ImGui::Text("Subdivision");
                ImGui::Text("UV Mapping");
                ImGui::Text("Boolean");
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Materials")) {
                ImGui::Text("Baking AO");
                ImGui::Text("Texture Set");
                ImGui::Text("PBR Shader");
                ImGui::Text("Variants");
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Subgraphs")) {
                ImGui::Text("Geometry Processing");
                ImGui::Text("Texturing Pipeline");
                ImGui::Text("Shading System");
                ImGui::Text("Custom Subgraph");
                ImGui::TreePop();
            }

            if (ImGui::TreeNode("Utilities")) {
                ImGui::Text("Convergence");
                ImGui::Text("Cleanup");
                ImGui::Text("Analysis");
                ImGui::Text("Cache");
                ImGui::TreePop();
            }

            ImGui::PopStyleVar();
        }

        if (ImGui::CollapsingHeader("Subgraphs", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Available Subgraphs:");
            
            if (ImGui::Button("Geometry Processing")) {
                editor.enterSubgraph(geometrySubgraphId);
            }
            if (ImGui::Button("Texturing Pipeline")) {
                editor.enterSubgraph(texturingSubgraphId);
            }
            if (ImGui::Button("Shading System")) {
                editor.enterSubgraph(shadingSubgraphId);
            }

            ImGui::Separator();

            int currentId = editor.getCurrentSubgraphId();
            if (currentId >= 0) {
                ANE::Subgraph* subgraph = editor.getSubgraph(currentId);
                if (subgraph) {
                    ImGui::Text("Current Subgraph: %s", subgraph->name.c_str());
                    if (ImGui::Button("Return to Main Graph")) {
                        editor.exitSubgraph();
                    }
                    
                    auto stack = editor.getSubgraphStack();
                    ImGui::Text("Subgraph Depth: %d", static_cast<int>(stack.size()) + 1);
                }
            } else {
                ImGui::Text("Currently in Main Graph");
            }
        }

        if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto selectedNodes = controller->getSelectedNodes();
            ImGui::Text("Selection: %zu node(s)", selectedNodes.size());

            if (!selectedNodes.empty()) {
                auto* selectedNode = controller->getModel()->getNode(selectedNodes[0]);
                if (selectedNode) {
                    ImGui::Separator();

                    ImGui::Text("Name: %s", selectedNode->name.c_str());
                    ImGui::Text("Type: %s", selectedNode->type.c_str());

                    if (selectedNode->isSubgraph) {
                        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "This is a Subgraph Node");
                        ImGui::Text("Subgraph ID: %d", selectedNode->subgraphId);

                        if (ImGui::Button("Enter Subgraph")) {
                            editor.enterSubgraph(selectedNode->subgraphId);
                        }
                    }

                    ImGui::Separator();

                    ImGui::Text("Position/Size: Not available in public API");

                    ImGui::Separator();

                    bool disabled = selectedNode->disabled;
                    if (ImGui::Checkbox("Disabled", &disabled)) {
                        controller->updateNode(selectedNodes[0], [disabled](ANE::Node& node) {
                            node.setDisabled(disabled);
                        });
                    }

                    bool isTemplate = selectedNode->isTemplate;
                    if (ImGui::Checkbox("Template", &isTemplate)) {
                        controller->updateNode(selectedNodes[0], [isTemplate](ANE::Node& node) {
                            node.setAsTemplate(isTemplate);
                        });
                    }

                    bool isCurrentFlag = selectedNode->isCurrentFlag;
                    if (ImGui::Checkbox("Current Flag", &isCurrentFlag)) {
                        controller->updateNode(selectedNodes[0], [isCurrentFlag](ANE::Node& node) {
                            node.setCurrentFlag(isCurrentFlag);
                        });
                    }
                }
            }
        }

        if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
            int currentId = editor.getCurrentSubgraphId();
            if (currentId >= 0) {
                ANE::Subgraph* subgraph = editor.getSubgraph(currentId);
                if (subgraph) {
                    ImGui::Text("Current Subgraph: %s", subgraph->name.c_str());
                    ImGui::Text("Nodes in Subgraph: %zu", subgraph->nodeIds.size());
                    ImGui::Text("Connections in Subgraph: %zu", subgraph->connectionIds.size());
                }
            } else {
                ImGui::Text("Main Graph");
            }
            ImGui::Text("Total Subgraphs: 3");
            
            auto stack = editor.getSubgraphStack();
            ImGui::Text("Subgraph Stack Depth: %zu", stack.size());
            ImGui::Separator();

            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        }

        ImGui::End();

        if (showHelp) {
            ImGui::SetNextWindowSize(ImVec2(520, 420), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("About ModFlow", &showHelp)) {
                ImGui::Text("ModFlow - 3D Modeling Pipeline with Subgraphs v1.0");
                ImGui::Text("Advanced Node-Based Workflow System");
                ImGui::Separator();

                ImGui::Text("Features:");
                ImGui::BulletText("Hierarchical node graph with subgraphs");
                ImGui::BulletText("Modular workflow design");
                ImGui::BulletText("Complete import to export pipeline");
                ImGui::BulletText("Real-time feedback and preview");

                ImGui::Separator();
                ImGui::Text("Working with Subgraphs:");
                ImGui::BulletText("Double-click on a subgraph node to enter it");
                ImGui::BulletText("Use 'Return to Main Graph' button to navigate back");
                ImGui::BulletText("The Subgraphs menu provides direct navigation");
                ImGui::BulletText("Create complex nested workflows with encapsulation");

                ImGui::Separator();
                if (ImGui::Button("Close", ImVec2(120, 0))) {
                    showHelp = false;
                }
            }
            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 20));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 20));
        ImGui::Begin("Status Bar", nullptr,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

        int currentId = editor.getCurrentSubgraphId();
        if (currentId >= 0) {
            ANE::Subgraph* subgraph = editor.getSubgraph(currentId);
            if (subgraph) {
                ImGui::Text("Editing Subgraph: %s | Zoom: %.0f%%",
                    subgraph->name.c_str(), zoom * 100.0f);
            }
        } else {
            ImGui::Text("Main Graph | Project: subgraphs_demo.mdfl | Zoom: %.0f%%", zoom * 100.0f);
        }

        ImGui::End();

        ImGui::Render();

        SDL_GL_MakeCurrent(window, gl_context);
        int display_w, display_h;
        SDL_GetWindowSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    LOG_DEBUG("Cleaning up ImGui");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    LOG_DEBUG("Cleaning up SDL");
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    LOG_DEBUG("Application terminated successfully");
    return 0;
}