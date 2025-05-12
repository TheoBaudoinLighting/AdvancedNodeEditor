

#include "../AdvancedNodeEditor/NodeEditor.h"
#include "../AdvancedNodeEditor/NodeEditorController.h"
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

namespace ANE {
    class INodeEditorController;
}

// Structure pour suivre les informations de sous-graphe
struct SubgraphInfo {
    int subgraphId;
    int nodeId;
    std::string name;
    std::vector<int> internalNodes;
    std::vector<int> internalConnections;
};

bool canConnect(const ANE::Pin& sourcePin, const ANE::Pin& destinationPin) {
    // Permettre la connexion si les pins sont du même type
    if (sourcePin.type == destinationPin.type)
        return true;

    // Permettre les conversions de types numériques
    if (sourcePin.type == ANE::PinType::Yellow && destinationPin.type == ANE::PinType::Green)
        return true;
    if (sourcePin.type == ANE::PinType::Green && destinationPin.type == ANE::PinType::Yellow)
        return true;

    // Permettre les conversions de types vectoriels
    if (sourcePin.type == ANE::PinType::Orange && destinationPin.type == ANE::PinType::White)
        return true;
    if (sourcePin.type == ANE::PinType::White && destinationPin.type == ANE::PinType::Orange)
        return true;

    // Permettre les connexions de flux
    if (sourcePin.type == ANE::PinType::Blue && destinationPin.type == ANE::PinType::Blue)
        return true;

    // Interdire toutes les autres connexions
    return false;
}

int main(int argc, char** argv)
{
    (void)argc;
    (void)argv;

    // Initialiser SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Erreur: %s\n", SDL_GetError());
        return 1;
    }

    // Configurer le contexte OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Créer la fenêtre de l'application
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Advanced Node Editor with Subgraphs", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr) {
        printf("Erreur de création de fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Créer le contexte OpenGL
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Activer vsync

    // Initialiser ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Appliquer le style ImGui et initialiser les backends
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Créer une instance de l'éditeur de nœuds et son contrôleur
    std::shared_ptr<ANE::INodeEditorController> controller = std::make_shared<ANE::NodeEditorController>();
    ANE::NodeEditor editor;

    // Configuration des styles pour l'éditeur
    ANE::EditorStyle style;
    style.backgroundColor = ANE::Color(0.10f, 0.11f, 0.12f, 1.00f);
    style.gridColor = ANE::Color(0.16f, 0.17f, 0.18f, 0.50f);
    style.selectionColor = ANE::Color(0.00f, 0.44f, 0.80f, 0.30f);
    style.gridSpacing = 20.0f;
    style.nodeRounding = 5.0f;
    style.pinRadius = 4.0f;
    style.connectionThickness = 2.5f;

    // Définir le style pour les nœuds de type Geometry
    ANE::NodeStyle geometryStyle;
    geometryStyle.baseColor = ANE::Color(0.20f, 0.25f, 0.30f, 1.0f);
    geometryStyle.headerColor = ANE::Color(0.18f, 0.22f, 0.26f, 0.8f);
    geometryStyle.accentColor = ANE::Color(0.20f, 0.65f, 0.90f, 1.0f);
    geometryStyle.borderColor = ANE::Color(0.22f, 0.28f, 0.35f, 1.0f);
    geometryStyle.selectedColor = ANE::Color(0.20f, 0.75f, 1.00f, 1.0f);
    geometryStyle.hoveredColor = ANE::Color(0.25f, 0.70f, 0.95f, 1.0f);
    geometryStyle.glowColor = ANE::Color(0.20f, 0.60f, 0.90f, 0.2f);
    style.nodeStyles["Geometry"] = geometryStyle;

    // Définir le style pour les nœuds de type Material
    ANE::NodeStyle materialStyle;
    materialStyle.baseColor = ANE::Color(0.30f, 0.22f, 0.25f, 1.0f);
    materialStyle.headerColor = ANE::Color(0.26f, 0.18f, 0.22f, 0.8f);
    materialStyle.accentColor = ANE::Color(0.90f, 0.40f, 0.50f, 1.0f);
    materialStyle.borderColor = ANE::Color(0.35f, 0.25f, 0.28f, 1.0f);
    materialStyle.selectedColor = ANE::Color(1.00f, 0.50f, 0.60f, 1.0f);
    materialStyle.hoveredColor = ANE::Color(0.95f, 0.45f, 0.55f, 1.0f);
    materialStyle.glowColor = ANE::Color(0.90f, 0.30f, 0.40f, 0.2f);
    style.nodeStyles["Material"] = materialStyle;

    // Définir le style pour les nœuds de type Utility
    ANE::NodeStyle utilityStyle;
    utilityStyle.baseColor = ANE::Color(0.28f, 0.28f, 0.30f, 1.0f);
    utilityStyle.headerColor = ANE::Color(0.24f, 0.24f, 0.26f, 0.8f);
    utilityStyle.accentColor = ANE::Color(0.80f, 0.80f, 0.90f, 1.0f);
    utilityStyle.borderColor = ANE::Color(0.32f, 0.32f, 0.35f, 1.0f);
    utilityStyle.selectedColor = ANE::Color(0.85f, 0.85f, 0.95f, 1.0f);
    utilityStyle.hoveredColor = ANE::Color(0.82f, 0.82f, 0.92f, 1.0f);
    utilityStyle.glowColor = ANE::Color(0.75f, 0.75f, 0.85f, 0.2f);
    style.nodeStyles["Utility"] = utilityStyle;

    // Définir le style pour les nœuds de type Subgraph
    ANE::NodeStyle subgraphStyle;
    subgraphStyle.baseColor = ANE::Color(0.20f, 0.30f, 0.20f, 1.0f);
    subgraphStyle.headerColor = ANE::Color(0.18f, 0.26f, 0.18f, 0.8f);
    subgraphStyle.accentColor = ANE::Color(0.40f, 0.85f, 0.40f, 1.0f);
    subgraphStyle.borderColor = ANE::Color(0.22f, 0.35f, 0.22f, 1.0f);
    subgraphStyle.selectedColor = ANE::Color(0.50f, 0.95f, 0.50f, 1.0f);
    subgraphStyle.hoveredColor = ANE::Color(0.45f, 0.90f, 0.45f, 1.0f);
    subgraphStyle.glowColor = ANE::Color(0.40f, 0.80f, 0.40f, 0.2f);
    style.nodeStyles["Subgraph"] = subgraphStyle;

    // Définir le style pour les pins de type Vec3
    ANE::PinStyle vec3Style;
    vec3Style.color = ANE::Color(0.22f, 0.70f, 0.40f, 1.0f);
    vec3Style.hoverColor = ANE::Color(0.32f, 0.80f, 0.50f, 1.0f);
    vec3Style.connectedColor = ANE::Color(0.42f, 0.90f, 0.60f, 1.0f);
    style.pinStyles["Vec3"] = vec3Style;

    // Définir les styles de connexion
    style.connectionStyle.baseColor = ANE::Color(0.600f, 0.650f, 0.700f, 0.627f);
    style.connectionStyle.selectedColor = ANE::Color(0.850f, 0.800f, 1.000f, 0.941f);
    style.connectionStyle.hoveredColor = ANE::Color(0.750f, 0.750f, 0.880f, 0.863f);
    style.connectionStyle.validColor = ANE::Color(0.750f, 0.950f, 0.800f, 0.902f);
    style.connectionStyle.invalidColor = ANE::Color(0.950f, 0.750f, 0.750f, 0.784f);

    // Créer des groupes de nœuds pour organiser le flux de travail
    int groupImport = editor.addGroup("1. Import & Preparation", ANE::Vec2(250, 10), ANE::Vec2(280, 380));
    int groupModeling = editor.addGroup("2. Geometric Modeling", ANE::Vec2(200, 420), ANE::Vec2(200, 400));
    int groupTextures = editor.addGroup("3. Textures & Materials", ANE::Vec2(450, 420), ANE::Vec2(200, 400));
    int groupSubgraphs = editor.addGroup("4. Subgraphs", ANE::Vec2(680, 200), ANE::Vec2(250, 250));
    int groupRendering = editor.addGroup("5. Rendering & Export", ANE::Vec2(300, 850), ANE::Vec2(180, 380));

    // Structure pour gérer les sous-graphes
    std::map<int, SubgraphInfo> subgraphs;
    int currentSubgraphId = -1; // -1 signifie qu'on est dans le graphe principal
    std::stack<int> subgraphStack;

    // Identifiants pour les sous-graphes
    const int geometrySubgraphId = 1;
    const int texturingSubgraphId = 2;
    const int shadingSubgraphId = 3;

    // Créer les nœuds pour le graphe principal
    int nodeCADFile = editor.addNode("Import CAD", "Utility", ANE::Vec2(300, 50));
    int nodeFBXFile = editor.addNode("Import FBX", "Utility", ANE::Vec2(300, 140));
    int nodeConvergence = editor.addNode("Convergence", "Utility", ANE::Vec2(300, 230));
    int nodePreprocess = editor.addNode("Preprocess", "Utility", ANE::Vec2(300, 320));

    // Créer les nœuds qui représentent les sous-graphes dans le graphe principal
    int geometrySubgraphNodeId = editor.addNode("Geometry Processing", "Subgraph", ANE::Vec2(720, 230));
    int texturingSubgraphNodeId = editor.addNode("Texturing Pipeline", "Subgraph", ANE::Vec2(720, 320));
    int shadingSubgraphNodeId = editor.addNode("Shading System", "Subgraph", ANE::Vec2(720, 410));

    // Créer des nœuds pour les textures et matériaux
    int nodeTextureSet = editor.addNode("Texture Set", "Material", ANE::Vec2(500, 550));
    int nodeShaderPBR = editor.addNode("PBR Shader", "Material", ANE::Vec2(500, 640));
    int nodeVariants = editor.addNode("Variants", "Material", ANE::Vec2(500, 730));

    // Créer des nœuds pour le rendu et l'export
    int nodeLighting = editor.addNode("Lighting", "Utility", ANE::Vec2(350, 890));
    int nodePostprocess = editor.addNode("Post-Process", "Utility", ANE::Vec2(350, 980));
    int nodePreview = editor.addNode("Preview", "Utility", ANE::Vec2(350, 1070));
    int nodeExport = editor.addNode("Export GLTF", "Utility", ANE::Vec2(350, 1160));

    // Assigner les nœuds à leurs groupes respectifs
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

    editor.addNodeToGroup(geometrySubgraphNodeId, groupSubgraphs);
    editor.addNodeToGroup(texturingSubgraphNodeId, groupSubgraphs);
    editor.addNodeToGroup(shadingSubgraphNodeId, groupSubgraphs);

    // Stocker les informations de sous-graphe
    subgraphs[geometrySubgraphId] = { geometrySubgraphId, geometrySubgraphNodeId, "Geometry Processing", {}, {} };
    subgraphs[texturingSubgraphId] = { texturingSubgraphId, texturingSubgraphNodeId, "Texturing Pipeline", {}, {} };
    subgraphs[shadingSubgraphId] = { shadingSubgraphId, shadingSubgraphNodeId, "Shading System", {}, {} };

    // Configurer l'apparence des nœuds
    ANE::Node* geoNode = editor.getNode(geometrySubgraphNodeId);
    if (geoNode) {
        geoNode->setIconSymbol("G");
    }

    ANE::Node* texNode = editor.getNode(texturingSubgraphNodeId);
    if (texNode) {
        texNode->setIconSymbol("T");
    }

    ANE::Node* shadeNode = editor.getNode(shadingSubgraphNodeId);
    if (shadeNode) {
        shadeNode->setIconSymbol("S");
    }

    // CONTENU DU SOUS-GRAPHE DE GÉOMÉTRIE - Simulé, pas réellement créé
    {
        // Définition simulée des nœuds du sous-graphe Geometry Processing
        /*
        int nodeDecimation = editor.addNode("Decimation", "Geometry", ANE::Vec2(300, 100));
        int nodeTopology = editor.addNode("Topology Fix", "Geometry", ANE::Vec2(300, 200));
        int nodeUVMapping = editor.addNode("UV Mapping", "Geometry", ANE::Vec2(300, 300));
        int nodeInPort = editor.addNode("Input", "Utility", ANE::Vec2(100, 200));
        int nodeOutPort = editor.addNode("Output", "Utility", ANE::Vec2(500, 200));
        */

        // Enregistrer les nœuds du sous-graphe (simulés)
        subgraphs[geometrySubgraphId].internalNodes = {101, 102, 103, 104, 105};
    }

    // CONTENU DU SOUS-GRAPHE DE TEXTURING - Simulé, pas réellement créé
    {
        // Définition simulée des nœuds du sous-graphe Texturing Pipeline
        /*
        int nodeUVUnwrap = editor.addNode("UV Unwrap", "Material", ANE::Vec2(200, 100));
        int nodeBakeAO = editor.addNode("Bake AO", "Material", ANE::Vec2(200, 200));
        int nodeBakeNormal = editor.addNode("Bake Normal", "Material", ANE::Vec2(200, 300));
        int nodeBakeRough = editor.addNode("Bake Roughness", "Material", ANE::Vec2(200, 400));
        */

        // Enregistrer les nœuds du sous-graphe (simulés)
        subgraphs[texturingSubgraphId].internalNodes = {201, 202, 203, 204, 205, 206, 207};
    }

    // CONTENU DU SOUS-GRAPHE DE SHADING - Simulé, pas réellement créé
    {
        // Définition simulée des nœuds du sous-graphe Shading System
        /*
        int nodePBRCore = editor.addNode("PBR Core", "Material", ANE::Vec2(300, 200));
        int nodeMetallic = editor.addNode("Metallic Setup", "Material", ANE::Vec2(150, 100));
        int nodeSpecular = editor.addNode("Specular Setup", "Material", ANE::Vec2(150, 300));
        */

        // Enregistrer les nœuds du sous-graphe (simulés)
        subgraphs[shadingSubgraphId].internalNodes = {301, 302, 303, 304, 305};
    }

    // Variables d'état UI
    bool showOptions = false;
    bool showHelp = false;
    float zoom = 1.0f;
    bool firstCycle = true;
    bool done = false;

    // Principale boucle d'application
    while (!done) {
        // Traiter les événements SDL
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Passer les événements à ImGui
            ImGui_ImplSDL2_ProcessEvent(&event);
            // Gérer les événements de sortie de l'application
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Début d'une nouvelle frame ImGui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Configurer la fenêtre principale de l'éditeur
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 300, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        // Créer la fenêtre principale de l'éditeur avec barre de menu
        ImGui::Begin("ModFlow - Advanced Node Graph with Subgraphs", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);

        // Créer la barre de menu avec les menus standard d'application
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
                // Option "Center View" - appelle la méthode centerView() pour centrer la vue sur tous les nœuds
                if (ImGui::MenuItem("Center View", "F")) {
                    editor.centerView();
                }

                // Option "Zoom In" - augmente le niveau de zoom
                if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                    zoom = std::min(zoom * 1.1f, 2.0f);
                    editor.setViewScale(zoom);
                }

                // Option "Zoom Out" - diminue le niveau de zoom
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    zoom = std::max(zoom * 0.9f, 0.5f);
                    editor.setViewScale(zoom);
                }

                ImGui::Separator();

                // Option "Reset Zoom" - réinitialise le zoom à 1.0
                if (ImGui::MenuItem("Reset Zoom", "Ctrl+0")) {
                    zoom = 1.0f;
                    editor.setViewScale(zoom);
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Subgraphs")) {
                if (ImGui::MenuItem("Return to Main Graph") && currentSubgraphId >= 0) {
                    if (!subgraphStack.empty()) {
                        currentSubgraphId = subgraphStack.top();
                        subgraphStack.pop();
                    } else {
                        currentSubgraphId = -1; // Revenir au graphe principal
                    }
                }

                ImGui::Separator();

                if (ImGui::MenuItem("Geometry Processing") && currentSubgraphId != geometrySubgraphId) {
                    if (currentSubgraphId >= 0) {
                        subgraphStack.push(currentSubgraphId);
                    }
                    currentSubgraphId = geometrySubgraphId;
                }

                if (ImGui::MenuItem("Texturing Pipeline") && currentSubgraphId != texturingSubgraphId) {
                    if (currentSubgraphId >= 0) {
                        subgraphStack.push(currentSubgraphId);
                    }
                    currentSubgraphId = texturingSubgraphId;
                }

                if (ImGui::MenuItem("Shading System") && currentSubgraphId != shadingSubgraphId) {
                    if (currentSubgraphId >= 0) {
                        subgraphStack.push(currentSubgraphId);
                    }
                    currentSubgraphId = shadingSubgraphId;
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

        // Afficher le titre du sous-graphe actuel s'il y en a un
        if (currentSubgraphId >= 0 && subgraphs.find(currentSubgraphId) != subgraphs.end()) {
            ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "Editing Subgraph: %s", subgraphs[currentSubgraphId].name.c_str());

            if (ImGui::Button("Return to Main Graph")) {
                if (!subgraphStack.empty()) {
                    currentSubgraphId = subgraphStack.top();
                    subgraphStack.pop();
                } else {
                    currentSubgraphId = -1; // Revenir au graphe principal
                }
            }
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Double-click on subgraph nodes to navigate into them");
        }

        // Gestion des nœuds de sous-graphe sélectionnés - double-cliquer pour entrer dans le sous-graphe
        static bool wasDoubleClicked = false;
        if (ImGui::IsMouseDoubleClicked(0) && !wasDoubleClicked) {
            wasDoubleClicked = true;

            // Vérifier si un nœud de sous-graphe est sélectionné
            auto selectedNodes = editor.getSelectedNodes();
            if (selectedNodes.size() == 1) {
                ANE::Node* node = editor.getNode(selectedNodes[0]);
                if (node && (node->name == "Geometry Processing" ||
                            node->name == "Texturing Pipeline" ||
                            node->name == "Shading System")) {
                    // Simuler l'entrée dans un sous-graphe
                    if (node->name == "Geometry Processing") currentSubgraphId = 1;
                    else if (node->name == "Texturing Pipeline") currentSubgraphId = 2;
                    else if (node->name == "Shading System") currentSubgraphId = 3;

                    if (currentSubgraphId > 0) {
                        subgraphStack.push(currentSubgraphId);
                    }
                }
            }
        } else if (!ImGui::IsMouseDoubleClicked(0)) {
            wasDoubleClicked = false;
        }

        // Rendre l'éditeur de nœuds
        editor.beginFrame();
        editor.render();
        editor.endFrame();

        // Centre la vue à la première frame pour montrer tous les nœuds
        if (firstCycle) {
            editor.centerView();
            firstCycle = false;
        }
        ImGui::End();

        // Créer le panneau de contrôle sur le côté droit
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 300, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImGui::Begin("Control Panel", nullptr);

        // Section Bibliothèque de nœuds - affiche les types de nœuds disponibles organisés en catégories
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

        // Section Sous-graphes - affiche et permet d'accéder aux sous-graphes
        if (ImGui::CollapsingHeader("Subgraphs", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Available Subgraphs:");

            for (const auto& [id, info] : subgraphs) {
                ImGui::PushID(id);
                if (ImGui::Button(info.name.c_str(), ImVec2(200, 0))) {
                    if (currentSubgraphId >= 0) {
                        subgraphStack.push(currentSubgraphId);
                    }
                    currentSubgraphId = id;
                }
                ImGui::PopID();
            }

            ImGui::Separator();

            // Navigation dans les sous-graphes
            if (currentSubgraphId >= 0) {
                ImGui::Text("Current Subgraph: %s", subgraphs[currentSubgraphId].name.c_str());
                if (ImGui::Button("Return to Main Graph")) {
                    if (!subgraphStack.empty()) {
                        currentSubgraphId = subgraphStack.top();
                        subgraphStack.pop();
                    } else {
                        currentSubgraphId = -1;
                    }
                }
                ImGui::Text("Subgraph Depth: %d", subgraphStack.size() + 1);
            } else {
                ImGui::Text("Currently in Main Graph");
            }
        }

        // Section Propriétés - affiche et permet d'éditer les propriétés des nœuds sélectionnés
        if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto selectedNodes = controller->getSelectedNodes();
            ImGui::Text("Selection: %zu node(s)", selectedNodes.size());

            if (!selectedNodes.empty()) {
                auto* selectedNode = controller->getModel()->getNode(selectedNodes[0]);
                if (selectedNode) {
                    ImGui::Separator();

                    // Afficher le nom et le type du nœud (lecture seule)
                    ImGui::Text("Name: %s", selectedNode->name.c_str());
                    ImGui::Text("Type: %s", selectedNode->type.c_str());

                    if (selectedNode->isSubgraph) {
                        ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "This is a Subgraph Node");
                        ImGui::Text("Subgraph ID: %d", selectedNode->subgraphId);

                        if (ImGui::Button("Enter Subgraph", ImVec2(150, 0))) {
                            if (currentSubgraphId >= 0) {
                                subgraphStack.push(currentSubgraphId);
                            }
                            currentSubgraphId = selectedNode->subgraphId;
                        }
                    }

                    ImGui::Separator();

                    // Informations de position/taille non disponibles via l'API publique
                    ImGui::Text("Position/Size: Not available in public API");

                    ImGui::Separator();

                    // Indicateurs d'état des nœuds qui peuvent être basculés
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

        // Section Statistiques - affiche des informations sur l'éditeur et sur les performances
        if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (currentSubgraphId >= 0) {
                ImGui::Text("Current Subgraph: %s", subgraphs[currentSubgraphId].name.c_str());
                ImGui::Text("Nodes in Subgraph: %zu", subgraphs[currentSubgraphId].internalNodes.size());
            } else {
                ImGui::Text("Main Graph");
            }

            ImGui::Text("Total Subgraphs: %zu", subgraphs.size());
            ImGui::Text("Subgraph Stack Depth: %zu", subgraphStack.size());

            ImGui::Separator();

            // Métriques de performance d'ImGui
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);
        }

        ImGui::End();

        // Fenêtre modale À propos/Aide
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

        // Barre d'état en bas de l'écran
        ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 20));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 20));
        ImGui::Begin("Status Bar", nullptr,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

        if (currentSubgraphId >= 0) {
            ImGui::Text("Editing Subgraph: %s | Zoom: %.0f%%",
                subgraphs[currentSubgraphId].name.c_str(), zoom * 100.0f);
        } else {
            ImGui::Text("Main Graph | Project: subgraphs_demo.mdfl | Zoom: %.0f%%", zoom * 100.0f);
        }

        ImGui::End();

        // Rendu ImGui
        ImGui::Render();

        // Rendu OpenGL
        SDL_GL_MakeCurrent(window, gl_context);
        int display_w, display_h;
        SDL_GetWindowSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Nettoyer ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Nettoyer SDL
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}