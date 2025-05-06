/**
 * ModFlow - Advanced Node-Based 3D Modeling Pipeline
 * 
 * This file is provided as an example implementation only.
 * It is provided as-is without any warranty of fitness for any particular use or design.
 * This code demonstrates the usage of the AdvancedNodeEditor library for creating
 * node-based workflows for 3D modeling and processing pipelines.
 * 
 * The implementation shows how to create nodes, connections, and handle user interactions
 * in a professional node editor environment. It is intended for educational purposes only.
 * 
 * Copyright (c) TBM VFX - All rights reserved
 */


#include "../AdvancedNodeEditor/AdvancedNodeEditor.h"
#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <cmath>
#include <functional>

/**
 * Determines if two pins can be connected based on their types.
 * 
 * @param sourcePin The source pin to check
 * @param destinationPin The destination pin to check
 * @return True if the pins can be connected, false otherwise
 */
bool canConnect(const ANE::Pin& sourcePin, const ANE::Pin& destinationPin) {
    // Allow connection if pins are of the same type
    if (sourcePin.type == destinationPin.type) return true;

    // Allow numeric type conversions
    if (sourcePin.type == ANE::PinType::Float && destinationPin.type == ANE::PinType::Int) return true;
    if (sourcePin.type == ANE::PinType::Int && destinationPin.type == ANE::PinType::Float) return true;
    
    // Allow vector type conversions
    if (sourcePin.type == ANE::PinType::Vec3 && destinationPin.type == ANE::PinType::Vec4) return true;
    if (sourcePin.type == ANE::PinType::Vec4 && destinationPin.type == ANE::PinType::Vec3) return true;

    // Allow flow connections
    if ((sourcePin.type == ANE::PinType::Flow && destinationPin.type == ANE::PinType::Flow)) return true;

    // Disallow all other connections
    return false;
}

/**
 * Main application entry point
 */
#ifdef _WIN32
extern "C" int main(int argc, char** argv)
#else
int main(int argc, char** argv)
#endif
{
    (void)argc;
    (void)argv;
    
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
        printf("Erreur: %s\n", SDL_GetError());
        return 1;
    }

    // Configure OpenGL context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    
    // Create application window
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Advanced Node Editor", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr) {
        printf("Erreur de création de fenêtre: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    // Create OpenGL context
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    // Disable docking feature which appears to be unavailable
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Set ImGui style and initialize backends
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Create node editor instance
    ANE::NodeEditor editor;

    // Configure editor style
    ANE::EditorStyle style = editor.getStyle();
    style.backgroundColor = ANE::Color(0.10f, 0.11f, 0.12f, 1.00f);
    style.gridColor = ANE::Color(0.16f, 0.17f, 0.18f, 0.50f);
    style.selectionColor = ANE::Color(0.00f, 0.44f, 0.80f, 0.30f);
    style.gridSpacing = 20.0f;
    style.nodeRounding = 5.0f;
    style.pinRadius = 4.0f;
    style.connectionThickness = 2.5f;

    // Define style for Geometry node type
    ANE::NodeStyle geometryStyle;
    geometryStyle.baseColor = ANE::Color(0.20f, 0.25f, 0.30f, 1.0f);
    geometryStyle.headerColor = ANE::Color(0.18f, 0.22f, 0.26f, 0.8f);
    geometryStyle.accentColor = ANE::Color(0.20f, 0.65f, 0.90f, 1.0f);
    geometryStyle.borderColor = ANE::Color(0.22f, 0.28f, 0.35f, 1.0f);
    geometryStyle.selectedColor = ANE::Color(0.20f, 0.75f, 1.00f, 1.0f);
    geometryStyle.hoveredColor = ANE::Color(0.25f, 0.70f, 0.95f, 1.0f);
    geometryStyle.glowColor = ANE::Color(0.20f, 0.60f, 0.90f, 0.2f);
    style.nodeStyles["Geometry"] = geometryStyle;

    // Define style for Material node type
    ANE::NodeStyle materialStyle;
    materialStyle.baseColor = ANE::Color(0.30f, 0.22f, 0.25f, 1.0f);
    materialStyle.headerColor = ANE::Color(0.26f, 0.18f, 0.22f, 0.8f);
    materialStyle.accentColor = ANE::Color(0.90f, 0.40f, 0.50f, 1.0f);
    materialStyle.borderColor = ANE::Color(0.35f, 0.25f, 0.28f, 1.0f);
    materialStyle.selectedColor = ANE::Color(1.00f, 0.50f, 0.60f, 1.0f);
    materialStyle.hoveredColor = ANE::Color(0.95f, 0.45f, 0.55f, 1.0f);
    materialStyle.glowColor = ANE::Color(0.90f, 0.30f, 0.40f, 0.2f);
    style.nodeStyles["Material"] = materialStyle;

    // Define style for Utility node type
    ANE::NodeStyle utilityStyle;
    utilityStyle.baseColor = ANE::Color(0.28f, 0.28f, 0.30f, 1.0f);
    utilityStyle.headerColor = ANE::Color(0.24f, 0.24f, 0.26f, 0.8f);
    utilityStyle.accentColor = ANE::Color(0.80f, 0.80f, 0.90f, 1.0f);
    utilityStyle.borderColor = ANE::Color(0.32f, 0.32f, 0.35f, 1.0f);
    utilityStyle.selectedColor = ANE::Color(0.85f, 0.85f, 0.95f, 1.0f);
    utilityStyle.hoveredColor = ANE::Color(0.82f, 0.82f, 0.92f, 1.0f);
    utilityStyle.glowColor = ANE::Color(0.75f, 0.75f, 0.85f, 0.2f);
    style.nodeStyles["Utility"] = utilityStyle;

    // Define style for Vec3 pin type
    ANE::PinStyle vec3Style;
    vec3Style.color = ANE::Color(0.22f, 0.70f, 0.40f, 1.0f);
    vec3Style.hoverColor = ANE::Color(0.32f, 0.80f, 0.50f, 1.0f);
    vec3Style.connectedColor = ANE::Color(0.42f, 0.90f, 0.60f, 1.0f);
    style.pinStyles["Vec3"] = vec3Style;

    // Define style for Mesh pin type
    ANE::PinStyle meshStyle;
    meshStyle.color = ANE::Color(0.20f, 0.60f, 0.90f, 1.0f);
    meshStyle.hoverColor = ANE::Color(0.30f, 0.70f, 1.00f, 1.0f);
    meshStyle.connectedColor = ANE::Color(0.40f, 0.80f, 1.00f, 1.0f);
    style.pinStyles["Mesh"] = meshStyle;

    // Define style for Material pin type
    ANE::PinStyle materialPinStyle;
    materialPinStyle.color = ANE::Color(0.90f, 0.40f, 0.50f, 1.0f);
    materialPinStyle.hoverColor = ANE::Color(1.00f, 0.50f, 0.60f, 1.0f);
    materialPinStyle.connectedColor = ANE::Color(1.00f, 0.60f, 0.70f, 1.0f);
    style.pinStyles["Material"] = materialPinStyle;

    // Define connection styles
    style.connectionStyle.baseColor = ANE::Color(0.600f, 0.650f, 0.700f, 0.627f);
    style.connectionStyle.selectedColor = ANE::Color(0.850f, 0.800f, 1.000f, 0.941f);
    style.connectionStyle.hoveredColor = ANE::Color(0.750f, 0.750f, 0.880f, 0.863f);
    style.connectionStyle.validColor = ANE::Color(0.750f, 0.950f, 0.800f, 0.902f);
    style.connectionStyle.invalidColor = ANE::Color(0.950f, 0.750f, 0.750f, 0.784f);

    // Apply style and connection validation callback
    editor.setStyle(style);
    editor.setCanConnectCallback(canConnect);

    // Create node groups for workflow organization
    int groupImport = editor.addGroup("1. Import & Preparation", ANE::Vec2(300, 10), ANE::Vec2(180, 380));
    int groupModeling = editor.addGroup("2. Geometric Modeling", ANE::Vec2(200, 420), ANE::Vec2(200, 400));
    int groupTextures = editor.addGroup("3. Textures & Materials", ANE::Vec2(450, 420), ANE::Vec2(200, 400));
    int groupRendering = editor.addGroup("4. Rendering & Export", ANE::Vec2(300, 850), ANE::Vec2(180, 380));

    // Configure group colors
    ANE::Group* group = editor.getGroup(groupImport);
    if (group) group->setColor(ANE::Color(0.20f, 0.30f, 0.40f, 0.25f));

    group = editor.getGroup(groupModeling);
    if (group) group->setColor(ANE::Color(0.25f, 0.40f, 0.30f, 0.25f));

    group = editor.getGroup(groupTextures);
    if (group) group->setColor(ANE::Color(0.40f, 0.25f, 0.25f, 0.25f));

    group = editor.getGroup(groupRendering);
    if (group) group->setColor(ANE::Color(0.30f, 0.25f, 0.40f, 0.25f));

    // Create nodes for import and preparation stage
    int nodeCADFile = editor.addNode("Import CAD", "Utility", ANE::Vec2(350, 50));
    int nodeFBXFile = editor.addNode("Import FBX", "Utility", ANE::Vec2(350, 140));
    int nodeConvergence = editor.addNode("Convergence", "Utility", ANE::Vec2(350, 230));
    int nodePreprocess = editor.addNode("Preprocess", "Utility", ANE::Vec2(350, 320));

    // Create nodes for geometric modeling stage
    int nodeDecimation = editor.addNode("Decimation", "Geometry", ANE::Vec2(250, 460));
    int nodeTopology = editor.addNode("Topology Correction", "Geometry", ANE::Vec2(250, 550));
    int nodeSubdivision = editor.addNode("Subdivision", "Geometry", ANE::Vec2(250, 640));
    int nodeUVMapping = editor.addNode("UV Mapping", "Geometry", ANE::Vec2(250, 730));

    // Create nodes for textures and materials stage
    int nodeBakingAO = editor.addNode("Baking AO", "Material", ANE::Vec2(500, 460));
    int nodeTextureSet = editor.addNode("Texture Set", "Material", ANE::Vec2(500, 550));
    int nodeShaderPBR = editor.addNode("PBR Shader", "Material", ANE::Vec2(500, 640));
    int nodeVariants = editor.addNode("Variants", "Material", ANE::Vec2(500, 730));

    // Create nodes for rendering and export stage
    int nodeLighting = editor.addNode("Lighting", "Utility", ANE::Vec2(350, 890));
    int nodePostprocess = editor.addNode("Post-Process", "Utility", ANE::Vec2(350, 980));
    int nodePreview = editor.addNode("Preview", "Utility", ANE::Vec2(350, 1070));
    int nodeExport = editor.addNode("Export GLTF", "Utility", ANE::Vec2(350, 1160));

    // Assign nodes to their respective groups
    editor.addNodeToGroup(nodeCADFile, groupImport);
    editor.addNodeToGroup(nodeFBXFile, groupImport);
    editor.addNodeToGroup(nodeConvergence, groupImport);
    editor.addNodeToGroup(nodePreprocess, groupImport);

    editor.addNodeToGroup(nodeDecimation, groupModeling);
    editor.addNodeToGroup(nodeTopology, groupModeling);
    editor.addNodeToGroup(nodeSubdivision, groupModeling);
    editor.addNodeToGroup(nodeUVMapping, groupModeling);

    editor.addNodeToGroup(nodeBakingAO, groupTextures);
    editor.addNodeToGroup(nodeTextureSet, groupTextures);
    editor.addNodeToGroup(nodeShaderPBR, groupTextures);
    editor.addNodeToGroup(nodeVariants, groupTextures);

    editor.addNodeToGroup(nodeLighting, groupRendering);
    editor.addNodeToGroup(nodePostprocess, groupRendering);
    editor.addNodeToGroup(nodePreview, groupRendering);
    editor.addNodeToGroup(nodeExport, groupRendering);

    // Create pins for CAD import node
    int cadPath = editor.addPin(nodeCADFile, "Path", true, ANE::PinType::String, ANE::PinShape::Square);
    int cadOutput = editor.addPin(nodeCADFile, "Mesh", false, ANE::PinType::Flow, ANE::PinShape::Circle);
    int cadMetadata = editor.addPin(nodeCADFile, "Metadata", false, ANE::PinType::Flow, ANE::PinShape::Square);

    // Create pins for FBX import node
    int fbxPath = editor.addPin(nodeFBXFile, "Path", true, ANE::PinType::String, ANE::PinShape::Square);
    int fbxOutput = editor.addPin(nodeFBXFile, "Mesh", false, ANE::PinType::Flow, ANE::PinShape::Circle);
    int fbxMaterials = editor.addPin(nodeFBXFile, "Materials", false, ANE::PinType::Flow, ANE::PinShape::Square);

    // Create pins for convergence node
    int convInputA = editor.addPin(nodeConvergence, "Source A", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int convInputB = editor.addPin(nodeConvergence, "Source B", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int convOptions = editor.addPin(nodeConvergence, "Options", true, ANE::PinType::Int, ANE::PinShape::Square);
    int convOutput = editor.addPin(nodeConvergence, "Mesh", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for preprocess node
    int preprocessInput = editor.addPin(nodePreprocess, "Mesh", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int preprocessClean = editor.addPin(nodePreprocess, "Clean", true, ANE::PinType::Bool, ANE::PinShape::Diamond);
    int preprocessNormals = editor.addPin(nodePreprocess, "Recalc. Normals", true, ANE::PinType::Bool, ANE::PinShape::Diamond);
    int preprocessOutput = editor.addPin(nodePreprocess, "Mesh", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for decimation node
    int decimateInput = editor.addPin(nodeDecimation, "Mesh", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int decimateRatio = editor.addPin(nodeDecimation, "Ratio", true, ANE::PinType::Float, ANE::PinShape::Square);
    int decimateQuality = editor.addPin(nodeDecimation, "Quality", true, ANE::PinType::Float, ANE::PinShape::Square);
    int decimateOutput = editor.addPin(nodeDecimation, "Mesh", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for topology node
    int topoInput = editor.addPin(nodeTopology, "Mesh", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int topoMerge = editor.addPin(nodeTopology, "Merge Verts", true, ANE::PinType::Bool, ANE::PinShape::Diamond);
    int topoClean = editor.addPin(nodeTopology, "Clean Faces", true, ANE::PinType::Bool, ANE::PinShape::Diamond);
    int topoOutput = editor.addPin(nodeTopology, "Mesh", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for subdivision node
    int subdivInput = editor.addPin(nodeSubdivision, "Mesh", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int subdivLevel = editor.addPin(nodeSubdivision, "Level", true, ANE::PinType::Int, ANE::PinShape::Square);
    int subdivScheme = editor.addPin(nodeSubdivision, "Scheme", true, ANE::PinType::Int, ANE::PinShape::Square);
    int subdivOutput = editor.addPin(nodeSubdivision, "Mesh", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for UV mapping node
    int uvInput = editor.addPin(nodeUVMapping, "Mesh", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int uvMethod = editor.addPin(nodeUVMapping, "Method", true, ANE::PinType::Int, ANE::PinShape::Square);
    int uvPadding = editor.addPin(nodeUVMapping, "Padding", true, ANE::PinType::Float, ANE::PinShape::Square);
    int uvOutput = editor.addPin(nodeUVMapping, "Mesh+UV", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for ambient occlusion baking node
    int aoInput = editor.addPin(nodeBakingAO, "Mesh+UV", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int aoSamples = editor.addPin(nodeBakingAO, "Samples", true, ANE::PinType::Int, ANE::PinShape::Square);
    int aoOutput = editor.addPin(nodeBakingAO, "AO Texture", false, ANE::PinType::Flow, ANE::PinShape::Square);
    int aoMesh = editor.addPin(nodeBakingAO, "Mesh", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for texture set node
    int texInput = editor.addPin(nodeTextureSet, "Mesh", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int texAO = editor.addPin(nodeTextureSet, "AO Map", true, ANE::PinType::Flow, ANE::PinShape::Square);
    int texAlbedo = editor.addPin(nodeTextureSet, "Albedo", true, ANE::PinType::Flow, ANE::PinShape::Square);
    int texRoughness = editor.addPin(nodeTextureSet, "Roughness", true, ANE::PinType::Flow, ANE::PinShape::Square);
    int texNormal = editor.addPin(nodeTextureSet, "Normal", true, ANE::PinType::Flow, ANE::PinShape::Square);
    int texOutput = editor.addPin(nodeTextureSet, "TextureSet", false, ANE::PinType::Flow, ANE::PinShape::Square);

    // Create pins for PBR shader node
    int shaderMesh = editor.addPin(nodeShaderPBR, "Mesh", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int shaderTextures = editor.addPin(nodeShaderPBR, "TextureSet", true, ANE::PinType::Flow, ANE::PinShape::Square);
    int shaderMethod = editor.addPin(nodeShaderPBR, "Method", true, ANE::PinType::Int, ANE::PinShape::Square);
    int shaderOutput = editor.addPin(nodeShaderPBR, "Material", false, ANE::PinType::Flow, ANE::PinShape::Square);

    // Create pins for variants node
    int varInput = editor.addPin(nodeVariants, "Material", true, ANE::PinType::Flow, ANE::PinShape::Square);
    int varCount = editor.addPin(nodeVariants, "Count", true, ANE::PinType::Int, ANE::PinShape::Square);
    int varSeed = editor.addPin(nodeVariants, "Seed", true, ANE::PinType::Int, ANE::PinShape::Square);
    int varOutput = editor.addPin(nodeVariants, "Variants", false, ANE::PinType::Flow, ANE::PinShape::Square);

    // Create pins for lighting node
    int lightScene = editor.addPin(nodeLighting, "Scene", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int lightEnv = editor.addPin(nodeLighting, "HDR Env", true, ANE::PinType::String, ANE::PinShape::Square);
    int lightOutput = editor.addPin(nodeLighting, "Lit Scene", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for post-processing node
    int postInput = editor.addPin(nodePostprocess, "Render", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int postExposure = editor.addPin(nodePostprocess, "Exposure", true, ANE::PinType::Float, ANE::PinShape::Square);
    int postContrast = editor.addPin(nodePostprocess, "Contrast", true, ANE::PinType::Float, ANE::PinShape::Square);
    int postSaturation = editor.addPin(nodePostprocess, "Saturation", true, ANE::PinType::Float, ANE::PinShape::Square);
    int postOutput = editor.addPin(nodePostprocess, "Final Render", false, ANE::PinType::Flow, ANE::PinShape::Circle);

    // Create pins for preview node
    int previewInput = editor.addPin(nodePreview, "Render", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int previewRes = editor.addPin(nodePreview, "Resolution", true, ANE::PinType::Vec2, ANE::PinShape::Square);
    int previewOutput = editor.addPin(nodePreview, "Preview", false, ANE::PinType::Flow, ANE::PinShape::Square);

    // Create pins for export node
    int exportScene = editor.addPin(nodeExport, "Scene", true, ANE::PinType::Flow, ANE::PinShape::Circle);
    int exportMaterials = editor.addPin(nodeExport, "Materials", true, ANE::PinType::Flow, ANE::PinShape::Square);
    int exportPath = editor.addPin(nodeExport, "Path", true, ANE::PinType::String, ANE::PinShape::Square);
    int exportCompress = editor.addPin(nodeExport, "Compression", true, ANE::PinType::Bool, ANE::PinShape::Diamond);
    int exportOutput = editor.addPin(nodeExport, "Status", false, ANE::PinType::Int, ANE::PinShape::Square);

    // Configure node appearances
    ANE::Node* node;

    // Configure CAD file node
    node = editor.getNode(nodeCADFile);
    if (node) {
        node->setIconSymbol("C");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure FBX file node
    node = editor.getNode(nodeFBXFile);
    if (node) {
        node->setIconSymbol("F");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure convergence node
    node = editor.getNode(nodeConvergence);
    if (node) {
        node->setIconSymbol("M");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure preprocess node
    node = editor.getNode(nodePreprocess);
    if (node) {
        node->setIconSymbol("P");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure decimation node
    node = editor.getNode(nodeDecimation);
    if (node) {
        node->setIconSymbol("D");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure topology node
    node = editor.getNode(nodeTopology);
    if (node) {
        node->setIconSymbol("T");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure subdivision node
    node = editor.getNode(nodeSubdivision);
    if (node) {
        node->setIconSymbol("S");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure UV mapping node
    node = editor.getNode(nodeUVMapping);
    if (node) {
        node->setIconSymbol("U");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure baking AO node
    node = editor.getNode(nodeBakingAO);
    if (node) {
        node->setIconSymbol("A");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure texture set node
    node = editor.getNode(nodeTextureSet);
    if (node) {
        node->setIconSymbol("X");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure PBR shader node
    node = editor.getNode(nodeShaderPBR);
    if (node) {
        node->setIconSymbol("B");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure variants node
    node = editor.getNode(nodeVariants);
    if (node) {
        node->setIconSymbol("V");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure lighting node
    node = editor.getNode(nodeLighting);
    if (node) {
        node->setIconSymbol("L");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure post-process node
    node = editor.getNode(nodePostprocess);
    if (node) {
        node->setIconSymbol("O");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure preview node
    node = editor.getNode(nodePreview);
    if (node) {
        node->setIconSymbol("R");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
    }

    // Configure export node
    node = editor.getNode(nodeExport);
    if (node) {
        node->setIconSymbol("E");
        node->setLabelPosition(ANE::NodeLabelPosition::Right);
        node->setCurrentFlag(true);
    }

    // Create connections between nodes for the import and preparation stage
    editor.addConnection(nodeCADFile, cadOutput, nodeConvergence, convInputA);
    editor.addConnection(nodeFBXFile, fbxOutput, nodeConvergence, convInputB);
    editor.addConnection(nodeConvergence, convOutput, nodePreprocess, preprocessInput);
    // Create connections for the geometric modeling stage
    // Connect preprocessing output to decimation input
    editor.addConnection(nodePreprocess, preprocessOutput, nodeDecimation, decimateInput);
    // Connect decimation output to topology correction input
    editor.addConnection(nodeDecimation, decimateOutput, nodeTopology, topoInput);
    // Connect topology output to subdivision input
    editor.addConnection(nodeTopology, topoOutput, nodeSubdivision, subdivInput);
    // Connect subdivision output to UV mapping input
    editor.addConnection(nodeSubdivision, subdivOutput, nodeUVMapping, uvInput);

    // Create connections for the texturing and material stage
    // Connect UV mapping output to ambient occlusion baking input
    editor.addConnection(nodeUVMapping, uvOutput, nodeBakingAO, aoInput);
    // Connect AO baking output to texture set AO input
    editor.addConnection(nodeBakingAO, aoOutput, nodeTextureSet, texAO);
    // Connect AO mesh output to texture set mesh input
    editor.addConnection(nodeBakingAO, aoMesh, nodeTextureSet, texInput);
    // Connect texture set output to PBR shader textures input
    editor.addConnection(nodeTextureSet, texOutput, nodeShaderPBR, shaderTextures);
    // Connect UV mapping output to PBR shader mesh input
    editor.addConnection(nodeUVMapping, uvOutput, nodeShaderPBR, shaderMesh);
    // Connect shader output to variants input
    editor.addConnection(nodeShaderPBR, shaderOutput, nodeVariants, varInput);

    // Create connections for the rendering and export stage
    // Connect UV mapping output to lighting scene input
    editor.addConnection(nodeUVMapping, uvOutput, nodeLighting, lightScene);
    // Connect lighting output to post-processing input
    editor.addConnection(nodeLighting, lightOutput, nodePostprocess, postInput);
    // Connect post-processing output to preview input
    editor.addConnection(nodePostprocess, postOutput, nodePreview, previewInput);
    // Connect post-processing output to export scene input
    editor.addConnection(nodePostprocess, postOutput, nodeExport, exportScene);
    // Connect variants output to export materials input
    editor.addConnection(nodeVariants, varOutput, nodeExport, exportMaterials);

    // Create additional cross-stage connections
    // Connect FBX materials to texture set albedo input
    editor.addConnection(nodeFBXFile, fbxMaterials, nodeTextureSet, texAlbedo);
    // Connect shader output to lighting environment input
    editor.addConnection(nodeShaderPBR, shaderOutput, nodeLighting, lightEnv);

    // Select the export node as the current node
    editor.selectNode(nodeExport);

    // UI state variables
    bool showOptions = false;
    bool showHelp = false;
    float zoom = 1.0f;
    bool firstCycle = true;
    bool done = false;

    // Main application loop
    while (!done) {
        // Process SDL events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Pass events to ImGui
            ImGui_ImplSDL2_ProcessEvent(&event);
            // Handle application quit events
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // Set up main editor window
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x - 300, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        // Create main editor window with menu bar
        ImGui::Begin("ModFlow - 3D Modeling Pipeline", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_MenuBar);

        // Create menu bar with standard application menus
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
                // Center view menu item - calls editor.centerView() to focus on all nodes
                if (ImGui::MenuItem("Center View", "F")) {
                    editor.centerView();
                }
                // Zoom in menu item - increases zoom level with upper limit
                if (ImGui::MenuItem("Zoom In", "Ctrl++")) {
                    zoom = std::min(zoom * 1.1f, 2.0f);
                    editor.setViewScale(zoom);
                }
                // Zoom out menu item - decreases zoom level with lower limit
                if (ImGui::MenuItem("Zoom Out", "Ctrl+-")) {
                    zoom = std::max(zoom * 0.9f, 0.5f);
                    editor.setViewScale(zoom);
                }
                ImGui::Separator();
                // Reset zoom menu item - sets zoom to default 1.0
                if (ImGui::MenuItem("Reset Zoom", "Ctrl+0")) {
                    zoom = 1.0f;
                    editor.setViewScale(zoom);
                }
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Tools")) {
                if (ImGui::MenuItem("Check Connections", "Ctrl+E")) {}
                if (ImGui::MenuItem("Optimize Graph", "Ctrl+O")) {}
                ImGui::Separator();
                if (ImGui::MenuItem("Settings...", "Ctrl+P")) {}
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

        // Render the node editor - beginFrame/endFrame pattern is required
        editor.beginFrame();
        editor.render();
        editor.endFrame();

        // Center view on first frame to show all nodes
        if (firstCycle) {
            editor.centerView();
            firstCycle = false;
        }

        ImGui::End();

        // Create control panel window on the right side
        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 300, 0), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, io.DisplaySize.y), ImGuiCond_FirstUseEver);

        ImGui::Begin("Control Panel", nullptr);

        // Node library section - displays available node types organized in categories
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

            if (ImGui::TreeNode("Rendering")) {
                ImGui::Text("Lighting");
                ImGui::Text("Post-Process");
                ImGui::Text("Preview");
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

        // Properties section - displays and allows editing of selected node properties
        if (ImGui::CollapsingHeader("Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Selection: %zu node(s)", editor.getSelectedNodes().size());

            auto selectedNodes = editor.getSelectedNodes();
            if (!selectedNodes.empty()) {
                ANE::Node* selectedNode = editor.getNode(selectedNodes[0]);
                if (selectedNode) {
                    ImGui::Separator();

                    // Display node name and type (read-only)
                    ImGui::Text("Name: %s", selectedNode->name.c_str());
                    ImGui::Text("Type: %s", selectedNode->type.c_str());

                    ImGui::Separator();

                    // Position/size info not available through public API
                    ImGui::Text("Position/Size: Not available in public API");

                    ImGui::Separator();

                    // Node state flags that can be toggled
                    bool disabled = selectedNode->disabled;
                    if (ImGui::Checkbox("Disabled", &disabled)) {
                        selectedNode->setDisabled(disabled);
                    }

                    bool isTemplate = selectedNode->isTemplate;
                    if (ImGui::Checkbox("Template", &isTemplate)) {
                        selectedNode->setAsTemplate(isTemplate);
                    }

                    bool isCurrentFlag = selectedNode->isCurrentFlag;
                    if (ImGui::Checkbox("Current Flag", &isCurrentFlag)) {
                        selectedNode->setCurrentFlag(isCurrentFlag);
                    }

                    ImGui::Separator();

                    // Input/output pins info not available through public API
                    ImGui::Text("Inputs/Outputs: Not available in public API");
                }
            }
        }

        // Settings section - allows customization of node editor visual style
        if (ImGui::CollapsingHeader("Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Get current editor style
            ANE::EditorStyle currentStyle = editor.getStyle();
            
            // Grid spacing slider - controls distance between grid points
            float gridSpacing = currentStyle.gridSpacing;
            if (ImGui::SliderFloat("Grid Spacing", &gridSpacing, 8.0f, 32.0f)) {
                currentStyle.gridSpacing = gridSpacing;
                editor.setStyle(currentStyle);
            }

            // Node rounding slider - controls corner rounding of nodes
            float nodeRounding = currentStyle.nodeRounding;
            if (ImGui::SliderFloat("Node Rounding", &nodeRounding, 0.0f, 12.0f)) {
                currentStyle.nodeRounding = nodeRounding;
                editor.setStyle(currentStyle);
            }

            // Pin radius slider - controls size of connection pins
            float pinRadius = currentStyle.pinRadius;
            if (ImGui::SliderFloat("Pin Radius", &pinRadius, 2.0f, 8.0f)) {
                currentStyle.pinRadius = pinRadius;
                editor.setStyle(currentStyle);
            }

            // Connection thickness slider - controls width of connection lines
            float connThickness = currentStyle.connectionThickness;
            if (ImGui::SliderFloat("Connection Thickness", &connThickness, 1.0f, 5.0f)) {
                currentStyle.connectionThickness = connThickness;
                editor.setStyle(currentStyle);
            }

            ImGui::Separator();

            // Reset style button - reverts all style settings to defaults
            if (ImGui::Button("Reset Style", ImVec2(150, 0))) {
                editor.setStyle(ANE::EditorStyle());
            }
        }

        // Statistics section - displays editor and performance information
        if (ImGui::CollapsingHeader("Statistics", ImGuiTreeNodeFlags_DefaultOpen)) {
            // Node and connection counts not available through public API
            ImGui::Text("Nodes: Not available in public API");
            ImGui::Text("Connections: Not available in public API");
            ImGui::Text("Groups: 4"); // Known value from code

            ImGui::Separator();

            // Performance metrics from ImGui
            ImGui::Text("FPS: %.1f", io.Framerate);
            ImGui::Text("Frame time: %.3f ms", 1000.0f / io.Framerate);

            ImGui::Separator();

            // Additional performance metrics toggle
            static bool showPerformance = false;
            ImGui::Checkbox("Show Performance", &showPerformance);

            if (showPerformance) {
                // Placeholder performance metrics (random values for demo)
                ImGui::Text("Draw calls: %d", rand() % 100 + 50);
                ImGui::Text("Vertices: %d", rand() % 10000 + 5000);
                ImGui::Text("GPU Memory: %.1f MB", float(rand() % 1000) / 10.0f);
            }
        }

        ImGui::End();

        // About/Help modal window
        if (showHelp) {
            ImGui::SetNextWindowSize(ImVec2(520, 420), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("About ModFlow", &showHelp)) {
                ImGui::Text("ModFlow - 3D Modeling Pipeline v1.0");
                ImGui::Text("TBM VFX");
                ImGui::Separator();
                ImGui::Text("Professional node-based modeling environment");
                ImGui::Spacing();
                ImGui::BulletText("Complete production workflow");
                ImGui::BulletText("Import to export pipeline");
                ImGui::BulletText("3D model optimization and preparation");
                ImGui::BulletText("PBR texture and material generation");
                ImGui::BulletText("Real-time render preview");
                ImGui::BulletText("Export for game engines and VFX");

                ImGui::Separator();
                ImGui::Text("This software is designed for professionals working on:");
                ImGui::BulletText("Video games");
                ImGui::BulletText("Films and VFX");
                ImGui::BulletText("Architectural visualization");
                ImGui::BulletText("Virtual and augmented reality");
                ImGui::BulletText("3D printing");

                ImGui::Separator();
                ImGui::Text("Keyboard shortcuts:");
                ImGui::Columns(2);
                ImGui::Text("Navigation"); ImGui::NextColumn(); ImGui::Text("Middle-click + drag");
                ImGui::NextColumn();
                ImGui::Text("Zoom"); ImGui::NextColumn(); ImGui::Text("Mouse wheel");
                ImGui::NextColumn();
                ImGui::Text("Selection"); ImGui::NextColumn(); ImGui::Text("Left click");
                ImGui::NextColumn();
                ImGui::Text("Multi-selection"); ImGui::NextColumn(); ImGui::Text("Ctrl + Left click");
                ImGui::NextColumn();
                ImGui::Text("Box selection"); ImGui::NextColumn(); ImGui::Text("Left click + drag");
                ImGui::NextColumn();
                ImGui::Text("Delete"); ImGui::NextColumn(); ImGui::Text("Delete");
                ImGui::NextColumn();
                ImGui::Text("Duplicate"); ImGui::NextColumn(); ImGui::Text("Ctrl+D");
                ImGui::Columns(1);

                ImGui::Separator();
                if (ImGui::Button("Documentation", ImVec2(120, 0))) {
                    showHelp = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("Close", ImVec2(120, 0))) {
                    showHelp = false;
                }
            }
            ImGui::End();
        }

        // Status bar at bottom of screen
        ImGui::SetNextWindowPos(ImVec2(0, io.DisplaySize.y - 20));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 20));
        ImGui::Begin("Status Bar", nullptr,
                  ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

        ImGui::Text("Ready | Project: concept_model.mdfl | Last saved: 3 minutes ago | Zoom: %.0f%%", zoom * 100.0f);

        ImGui::End();

        // Render ImGui
        ImGui::Render();
        
        // OpenGL rendering
        SDL_GL_MakeCurrent(window, gl_context);
        int display_w, display_h;
        SDL_GetWindowSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.08f, 0.08f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    // Cleanup SDL
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}