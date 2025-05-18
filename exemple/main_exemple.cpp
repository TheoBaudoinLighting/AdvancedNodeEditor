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

    // Affichage des informations sur les pins pour le débogage
    std::cout << "Nœud créé: " << node->name << " (Type: " << node->type << ")" << std::endl;
    std::cout << "Pins d'entrée: " << node->inputs.size() << ", Pins de sortie: " << node->outputs.size() << std::endl;

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

        // Construire le graphe de dépendances inversé (pour DFS)
        for (const auto& connection : connections) {
            dependencyGraph[connection.endNodeId].push_back(connection.startNodeId);
        }

        // DFS à partir du noeud de sortie
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

        // Si un noeud de sortie spécifique est fourni, commencer par là
        if (outputNodeId != -1) {
            dfs(outputNodeId);
        } else {
            // Sinon, trouver tous les noeuds qui n'ont pas de connexions sortantes
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

        // Construire la carte des connexions (pinId de destination -> nodeId, pinId source)
        for (const auto& connection : connections) {
            connectionMap[connection.endNodeId][connection.endPinId] =
                (connection.startNodeId << 16) | connection.startPinId;
        }

        // Évaluer chaque noeud dans l'ordre
        for (int nodeId : evaluationOrder) {
            const Node* node = m_editor.getNode(nodeId);
            if (!node) continue;

            std::vector<NodeValue> inputValues;

            // Collecter les valeurs d'entrée pour ce noeud
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
                    // Pin non connectée
                    inputValues.push_back(NodeValue(0.0f));
                }
            }

            // Évaluer le noeud
            NodeDefinition def = GetNodeDefByType(node->type);
            NodeValue result = def.evaluator(inputValues);
            nodeValues[nodeId] = result;
        }

        // Retourner la valeur du noeud de sortie
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

        // Création du graphe de calcul mathématique
        editor.enterSubgraphByUUID(mathGraphUuid);

        // Créer les nœuds mathématiques avec vérification
        int const1NodeId = editor.addNode("Value A", "Math.Constant", Vec2(100, 100));
        Node* const1Node = editor.getNode(const1NodeId);
        if (const1Node) std::cout << "Nœud Constant1 créé: " << const1NodeId << ", pins sortie: " << const1Node->outputs.size() << std::endl;

        int const2NodeId = editor.addNode("Value B", "Math.Constant", Vec2(100, 200));
        Node* const2Node = editor.getNode(const2NodeId);
        if (const2Node) std::cout << "Nœud Constant2 créé: " << const2NodeId << ", pins sortie: " << const2Node->outputs.size() << std::endl;

        int const3NodeId = editor.addNode("Value C", "Math.Constant", Vec2(100, 300));
        Node* const3Node = editor.getNode(const3NodeId);
        if (const3Node) std::cout << "Nœud Constant3 créé: " << const3NodeId << ", pins sortie: " << const3Node->outputs.size() << std::endl;

        int addNodeId = editor.addNode("Add", "Math.Add", Vec2(300, 150));
        Node* addNode = editor.getNode(addNodeId);
        if (addNode) std::cout << "Nœud Add créé: " << addNodeId << ", pins entrée: " << addNode->inputs.size()
                               << ", pins sortie: " << addNode->outputs.size() << std::endl;

        int multiplyNodeId = editor.addNode("Multiply", "Math.Multiply", Vec2(500, 200));
        Node* multiplyNode = editor.getNode(multiplyNodeId);
        if (multiplyNode) std::cout << "Nœud Multiply créé: " << multiplyNodeId << ", pins entrée: " << multiplyNode->inputs.size()
                                   << ", pins sortie: " << multiplyNode->outputs.size() << std::endl;

        // Créer le nœud Subtract avec vérification explicite
        int subtractNodeId = editor.addNode("Subtract", "Math.Subtract", Vec2(700, 150));
        Node* subtractNode = editor.getNode(subtractNodeId);
        if (subtractNode) std::cout << "Nœud Subtract créé: " << subtractNodeId << ", pins entrée: " << subtractNode->inputs.size()
                                   << ", pins sortie: " << subtractNode->outputs.size() << std::endl;

        // Si le nœud de soustraction n'a pas de pin de sortie, en ajouter une manuellement
        if (subtractNode && subtractNode->outputs.empty()) {
            int subtractOutputPinId = editor.addPin(subtractNodeId, "Result", false, PinType::Blue);
            std::cout << "Ajout manuel d'une pin de sortie au nœud Subtract: " << subtractOutputPinId << std::endl;
            subtractNode = editor.getNode(subtractNodeId); // Récupérer le nœud mis à jour
        }

        // Créer le nœud de sortie explicitement
        int outputNodeId = editor.addNode("Output", "Default", Vec2(900, 150));
        // Ajouter explicitement une pin d'entrée au nœud de sortie
        int outputPinId = editor.addPin(outputNodeId, "Value", true, PinType::Blue);
        Node* outputNode = editor.getNode(outputNodeId);

        std::cout << "Nœud Output créé avec ID: " << outputNodeId << ", Pin entrée ID: " << outputPinId << std::endl;

        // Obtenir les pointeurs vers les nœuds (déjà déclarés lors de la création)
        // Pas besoin de les redéclarer ici

        // Connexions: const1 et const2 -> add
        if (const1Node && !const1Node->outputs.empty() && addNode && !addNode->inputs.empty()) {
            editor.addConnection(const1NodeId, const1Node->outputs[0].id, addNodeId, addNode->inputs[0].id);
        }
        if (const2Node && !const2Node->outputs.empty() && addNode && addNode->inputs.size() >= 2) {
            editor.addConnection(const2NodeId, const2Node->outputs[0].id, addNodeId, addNode->inputs[1].id);
        }

        // Connexion: add et const3 -> multiply
        if (addNode && !addNode->outputs.empty() && multiplyNode && !multiplyNode->inputs.empty()) {
            editor.addConnection(addNodeId, addNode->outputs[0].id, multiplyNodeId, multiplyNode->inputs[0].id);
        }
        if (const3Node && !const3Node->outputs.empty() && multiplyNode && multiplyNode->inputs.size() >= 2) {
            editor.addConnection(const3NodeId, const3Node->outputs[0].id, multiplyNodeId, multiplyNode->inputs[1].id);
        }

        // Connexion: multiply et const1 -> subtract
        if (multiplyNode && !multiplyNode->outputs.empty() && subtractNode && !subtractNode->inputs.empty()) {
            editor.addConnection(multiplyNodeId, multiplyNode->outputs[0].id, subtractNodeId, subtractNode->inputs[0].id);
        }
        if (const1Node && !const1Node->outputs.empty() && subtractNode && subtractNode->inputs.size() >= 2) {
            editor.addConnection(const1NodeId, const1Node->outputs[0].id, subtractNodeId, subtractNode->inputs[1].id);
        }

        // Connexion: subtract -> output
        if (subtractNode && !subtractNode->outputs.empty() && outputNode) {
            try {
                int subtractOutputPinId = subtractNode->outputs[0].id;
                editor.addConnection(subtractNodeId, subtractOutputPinId, outputNodeId, outputPinId);
                std::cout << "Connexion créée entre Subtract (pin " << subtractOutputPinId << ") et Output (pin " << outputPinId << ")" << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Erreur lors de la connexion à Output: " << e.what() << std::endl;
            }
        } else {
            if (!subtractNode) std::cerr << "subtractNode est nul" << std::endl;
            else if (subtractNode->outputs.empty()) std::cerr << "subtractNode->outputs est vide" << std::endl;
            if (!outputNode) std::cerr << "outputNode est nul" << std::endl;
        }

        // Configurer le sous-graphe avec des informations d'interface
        auto mathSubgraph = editor.getSubgraph(editor.getSubgraphId(mathGraphUuid));
        if (mathSubgraph) {
            mathSubgraph->interfaceOutputs.push_back((outputNodeId << 16) | outputPinId);
        }

        editor.exitSubgraph();

        // Création d'un noeud pour représenter notre sous-graphe mathématique dans le graphe principal
        Node* mathGraphNode = editor.createSubgraphNode(editor.getSubgraphId(mathGraphUuid),
                                                      "Math Calculation",
                                                      Vec2(300, 175));
        int mathGraphNodeId = mathGraphNode ? mathGraphNode->id : -1;

        // Variables pour stocker l'évaluation et les préférences
        bool showEvaluationWindow = true;
        NodeValue evaluationResult;
        std::vector<int> evaluationOrder;
        std::unordered_map<int, std::string> nodeNames;

        // Pour stocker les valeurs personnalisées des noeuds constants
        std::unordered_map<int, float> constantValues;
        constantValues[const1NodeId] = 5.0f;
        constantValues[const2NodeId] = 3.0f;
        constantValues[const3NodeId] = 2.0f;

        // Définir un évaluateur personnalisé qui utilise des valeurs constantes spécifiques
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

                    std::cout << "Ordre d'évaluation: ";
                    for (int id : evaluationOrder) {
                        std::cout << id << " ";
                    }
                    std::cout << std::endl;

                    // Construire la carte des connexions (pinId de destination -> nodeId, pinId source)
                    for (const auto& connection : connections) {
                        connectionMap[connection.endNodeId][connection.endPinId] =
                            (connection.startNodeId << 16) | connection.startPinId;
                        std::cout << "Connexion: " << connection.startNodeId << ":" << connection.startPinId
                                << " -> " << connection.endNodeId << ":" << connection.endPinId << std::endl;
                    }

                    // Évaluer chaque noeud dans l'ordre
                    for (int nodeId : evaluationOrder) {
                        const Node* node = m_editor.getNode(nodeId);
                        if (!node) {
                            std::cout << "Nœud " << nodeId << " non trouvé, ignoré" << std::endl;
                            continue;
                        }

                        std::cout << "Début évaluation du nœud " << nodeId << " (" << node->name << ", type: " << node->type << ")" << std::endl;

                        std::vector<NodeValue> inputValues;

                        // Collecter les valeurs d'entrée pour ce noeud
                        for (const auto& pin : node->inputs) {
                            if (connectionMap.count(nodeId) && connectionMap[nodeId].count(pin.id)) {
                                int sourceInfo = connectionMap[nodeId][pin.id];
                                int sourceNodeId = sourceInfo >> 16;
                                int sourcePinId = sourceInfo & 0xFFFF;

                                std::cout << "  Pin " << pin.id << " connectée au nœud " << sourceNodeId << " pin " << sourcePinId << std::endl;

                                if (nodeValues.count(sourceNodeId)) {
                                    inputValues.push_back(nodeValues[sourceNodeId]);
                                    std::cout << "  Valeur d'entrée: " << nodeValues[sourceNodeId].getString() << std::endl;
                                } else {
                                    inputValues.push_back(NodeValue(0.0f));
                                    std::cout << "  Nœud source non évalué, utilisation de 0.0 par défaut" << std::endl;
                                }
                            } else {
                                // Pin non connectée
                                inputValues.push_back(NodeValue(0.0f));
                                std::cout << "  Pin " << pin.id << " non connectée, utilisation de 0.0 par défaut" << std::endl;
                            }
                        }

                        // Évaluer le noeud avec nos valeurs personnalisées
                        NodeValue result;

                        // Traitement spécial pour les noeuds constants
                        if (node->type == "Math.Constant" && m_constantValues.count(nodeId)) {
                            result = NodeValue(m_constantValues.at(nodeId));
                            std::cout << "  Nœud constant, valeur définie: " << m_constantValues.at(nodeId) << std::endl;
                        }
                        // Traitement spécial pour les nœuds de type Default (comme le nœud Output)
                        else if (node->type == "Default") {
                            // Pour un nœud Default, on passe simplement la première valeur d'entrée
                            if (!inputValues.empty()) {
                                result = inputValues[0];
                                std::cout << "  Nœud Default, transfert de la valeur d'entrée: " << inputValues[0].getString() << std::endl;
                            } else {
                                result = NodeValue(0.0f);
                                std::cout << "  Nœud Default sans entrées, valeur par défaut: 0" << std::endl;
                            }
                        }
                        else {
                            try {
                                NodeDefinition def = GetNodeDefByType(node->type);
                                result = def.evaluator(inputValues);
                                std::cout << "  Évaluation standard" << std::endl;
                            } catch (const std::exception& e) {
                                std::cout << "  ERREUR d'évaluation: " << e.what() << std::endl;
                                result = NodeValue(0.0f);
                            }
                        }

                        nodeValues[nodeId] = result;

                        // Afficher la valeur calculée pour ce nœud
                        std::cout << "Évaluation du nœud " << nodeId << " (" << node->name << "): ";
                        if (result.isNumeric()) {
                            std::cout << result.getNumeric() << std::endl;
                        } else {
                            std::cout << result.getString() << std::endl;
                        }
                    }

                    // Retourner la valeur du noeud de sortie
                    if (outputNodeId != -1 && nodeValues.count(outputNodeId)) {
                        std::cout << "Retour de la valeur du nœud de sortie spécifié: " << outputNodeId << std::endl;
                        return nodeValues[outputNodeId];
                    } else if (!evaluationOrder.empty() && nodeValues.count(evaluationOrder.back())) {
                        std::cout << "Retour de la valeur du dernier nœud évalué: " << evaluationOrder.back() << std::endl;
                        return nodeValues[evaluationOrder.back()];
                    }

                    std::cout << "Aucune valeur trouvée, retour de 0.0" << std::endl;
                    return NodeValue(0.0f);
                } catch (const std::exception& e) {
                    std::cerr << "Exception dans evaluateGraph: " << e.what() << std::endl;
                    return NodeValue(0.0f);
                } catch (...) {
                    std::cerr << "Exception inconnue dans evaluateGraph" << std::endl;
                    return NodeValue(0.0f);
                }
            }

        private:
            const NodeEditor& m_editor;
            const std::unordered_map<int, float>& m_constantValues;
            GraphEvaluator m_baseEvaluator;
        };

        // Créer l'évaluateur personnalisé
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
                        // Entrer dans le sous-graphe mathématique
                        editor.enterSubgraphByUUID(mathGraphUuid);

                        // Évaluer le graphe
                        evaluationResult = evaluator.evaluateGraph(outputNodeId);
                        evaluationOrder = evaluator.computeEvaluationOrder(outputNodeId);

                        // Collecter les noms des noeuds
                        nodeNames.clear();
                        for (int id : evaluationOrder) {
                            const Node* node = editor.getNode(id);
                            if (node) {
                                nodeNames[id] = node->name;
                            }
                        }

                        // Sortir du sous-graphe
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
                                    // Valeur mise à jour
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

                if (ImGui::BeginTabItem("Add Nodes")) {
                    ImGui::Text("Node Library");
                    ImGui::Separator();

                    if (ImGui::CollapsingHeader("Math", ImGuiTreeNodeFlags_DefaultOpen)) {
                        ImGui::Text("Dernière évaluation : %.2f", evaluationResult.getNumeric());
                        ImGui::Separator();

                        if (ImGui::Button("Add")) {
                            editor.createNodeOfType("Math.Add", Vec2(300, 200));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Multiply")) {
                            editor.createNodeOfType("Math.Multiply", Vec2(300, 300));
                        }

                        if (ImGui::Button("Subtract")) {
                            editor.createNodeOfType("Math.Subtract", Vec2(300, 400));
                        }
                        ImGui::SameLine();
                        if (ImGui::Button("Divide")) {
                            editor.createNodeOfType("Math.Divide", Vec2(300, 500));
                        }

                        if (ImGui::Button("Constant")) {
                            editor.createNodeOfType("Math.Constant", Vec2(100, 300));
                        }
                    }

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Evaluation")) {
                    ImGui::Text("Graph Evaluation");
                    ImGui::Separator();

                    if (ImGui::Button("Evaluate Graph")) {
                        // Sauvegarder l'état actuel
                        int currentSubgraphId = editor.getCurrentSubgraphId();

                        // Entrer dans le sous-graphe mathématique
                        while (editor.getCurrentSubgraphId() >= 0) {
                            editor.exitSubgraph();
                        }
                        editor.enterSubgraphByUUID(mathGraphUuid);

                        // Évaluer le graphe
                        evaluationResult = evaluator.evaluateGraph(outputNodeId);
                        evaluationOrder = evaluator.computeEvaluationOrder(outputNodeId);

                        // Collecter les noms des noeuds
                        nodeNames.clear();
                        for (int id : evaluationOrder) {
                            const Node* node = editor.getNode(id);
                            if (node) {
                                nodeNames[id] = node->name;
                            }
                        }

                        // Restaurer l'état précédent
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

            // Fenêtre d'évaluation séparée
            if (showEvaluationWindow) {
                ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
                ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
                ImGui::Begin("Graph Evaluation Results", &showEvaluationWindow);

                if (ImGui::Button("Evaluate Graph Now")) {
                    // Sauvegarder l'état actuel
                    int currentSubgraphId = editor.getCurrentSubgraphId();

                    // Entrer dans le sous-graphe mathématique
                    while (editor.getCurrentSubgraphId() >= 0) {
                        editor.exitSubgraph();
                    }
                    editor.enterSubgraphByUUID(mathGraphUuid);

                    // Modifier les évaluateurs des noeuds constants pour utiliser les valeurs personnalisées
                    auto originalEval = GetNodeDefByType("Math.Constant").evaluator;
                    auto modifiedConstantNodeDefs = GetNodeDefByType("Math.Constant");

                    // Évaluer le graphe
                    evaluationResult = evaluator.evaluateGraph(outputNodeId);
                    evaluationOrder = evaluator.computeEvaluationOrder(outputNodeId);

                    // Collecter les noms des noeuds
                    nodeNames.clear();
                    for (int id : evaluationOrder) {
                        const Node* node = editor.getNode(id);
                        if (node) {
                            nodeNames[id] = node->name;
                        }
                    }

                    // Restaurer l'état précédent
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

                // Modifier les valeurs constantes
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