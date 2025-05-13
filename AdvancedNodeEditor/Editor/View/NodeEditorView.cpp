#include "NodeEditorView.h"
#include "../../NodeEditor.h"
#include <algorithm>

#include "../../Core/Conversions/Conversions.h"

namespace ANE {

NodeEditorView::NodeEditorView(std::shared_ptr<INodeEditorController> controller)
    : m_controller(controller)
    , m_coreEditor(std::make_shared<NodeEditorCore::NodeEditor>())
    , m_nextLayerId(1)
    , m_viewScale(1.0f)
{
    m_controller->addEventListener(EventType::NodeCreated, [this](const Event& event) {
        int nodeId = event.getData<int>("nodeId");
        const auto* modelNode = m_controller->getModel()->getNode(nodeId);
        if (modelNode) {
            m_coreEditor->addNode(modelNode->name, modelNode->type, NodeEditorCore::Vec2(modelNode->position.x, modelNode->position.y));
        }
    });
    
    m_controller->addEventListener(EventType::NodeDeleted, [this](const Event& event) {
        int nodeId = event.getData<int>("nodeId");
        m_coreEditor->removeNode(nodeId);
    });
    
    m_controller->addEventListener(EventType::ConnectionCreated, [this](const Event& event) {
        int connectionId = event.getData<int>("connectionId");
        const auto* modelConn = m_controller->getModel()->getConnection(connectionId);
        if (modelConn) {
            m_coreEditor->addConnection(
                modelConn->startNodeId, 
                modelConn->startPinId, 
                modelConn->endNodeId, 
                modelConn->endPinId
            );
        }
    });
    
    m_controller->addEventListener(EventType::ConnectionDeleted, [this](const Event& event) {
        int connectionId = event.getData<int>("connectionId");
        m_coreEditor->removeConnection(connectionId);
    });
    
    m_controller->addEventListener(EventType::GroupCreated, [this](const Event& event) {
        int groupId = event.getData<int>("groupId");
        const auto* modelGroup = m_controller->getModel()->getGroup(groupId);
        if (modelGroup) {
            m_coreEditor->addGroup(modelGroup->name,
                      NodeEditorCore::Vec2(modelGroup->position.x, modelGroup->position.y),
                      NodeEditorCore::Vec2(modelGroup->size.x, modelGroup->size.y));
        }
    });
    
    m_controller->addEventListener(EventType::GroupDeleted, [this](const Event& event) {
        int groupId = event.getData<int>("groupId");
        m_coreEditor->removeGroup(groupId);
    });
    
    m_coreEditor->setNodeCreatedCallback([this](int nodeId) {
    });
    
    m_coreEditor->setNodeRemovedCallback([this](int nodeId) {
    });
    
    m_coreEditor->setConnectionCreatedCallback([this](int connectionId) {
    });
    
    m_coreEditor->setConnectionRemovedCallback([this](int connectionId) {
    });
    
    addLayer("Grid", 0, [this](ImDrawList* drawList, const ImVec2& canvasPos) {
    });
    
    addLayer("Groups", 10, [this](ImDrawList* drawList, const ImVec2& canvasPos) {
    });
    
    addLayer("Connections", 20, [this](ImDrawList* drawList, const ImVec2& canvasPos) {
    });
    
    addLayer("Nodes", 30, [this](ImDrawList* drawList, const ImVec2& canvasPos) {
    });
    
    addLayer("Breadcrumbs", 40, [this](ImDrawList* drawList, const ImVec2& canvasPos) {
        drawSubgraphBreadcrumbs(drawList, canvasPos);
    });
}

NodeEditorView::~NodeEditorView() = default;

void NodeEditorView::beginFrame() {
    m_coreEditor->beginFrame();
}

void NodeEditorView::render() {
    m_coreEditor->render();
}

void NodeEditorView::endFrame() {
    m_coreEditor->endFrame();
}

void NodeEditorView::setViewPosition(const Vec2& position) {
    m_viewPosition = position;
    m_coreEditor->setViewPosition(NodeEditorCore::Vec2(position.x, position.y));
}

Vec2 NodeEditorView::getViewPosition() const {
    NodeEditorCore::Vec2 pos = m_coreEditor->getViewPosition();
    return Vec2(pos.x, pos.y);
}

void NodeEditorView::setViewScale(float scale) {
    m_viewScale = scale;
    m_coreEditor->setViewScale(scale);
}

float NodeEditorView::getViewScale() const {
    return m_coreEditor->getViewScale();
}

void NodeEditorView::centerView() {
    m_coreEditor->centerView();
    NodeEditorCore::Vec2 pos = m_coreEditor->getViewPosition();
    m_viewPosition = Vec2(pos.x, pos.y);
}

void NodeEditorView::centerOnNode(int nodeId) {
    m_coreEditor->centerOnNode(nodeId);
    NodeEditorCore::Vec2 pos = m_coreEditor->getViewPosition();
    m_viewPosition = Vec2(pos.x, pos.y);
}

void NodeEditorView::setStyle(const EditorStyle& style) {
    m_coreEditor->setStyle(NodeEditorCore::convertToInternalStyle(style));
}

EditorStyle NodeEditorView::getStyle() const {
    return NodeEditorCore::convertToAPIStyle(m_coreEditor->getStyle());
}

int NodeEditorView::addLayer(const std::string& name, int zOrder, std::function<void(ImDrawList*, const ImVec2&)> drawCallback) {
    int layerId = m_nextLayerId++;
    m_layers[layerId] = NodeEditorCore::internal::Layer(name, zOrder, drawCallback);
    return layerId;
}

void NodeEditorView::removeLayer(int layerId) {
    m_layers.erase(layerId);
}

void NodeEditorView::setLayerVisible(int layerId, bool visible) {
    auto it = m_layers.find(layerId);
    if (it != m_layers.end()) {
        it->second.visible = visible;
    }
}

void NodeEditorView::setLayerZOrder(int layerId, int zOrder) {
    auto it = m_layers.find(layerId);
    if (it != m_layers.end()) {
        it->second.zOrder = zOrder;
    }
}

void NodeEditorView::drawSubgraphBreadcrumbs(ImDrawList* drawList, const ImVec2& canvasPos) {
    auto model = m_controller->getModel();
    int currentSubgraphId = model->getState<int>("currentSubgraphId", -1);
    if (currentSubgraphId < 0) return;
    
    auto navigationStack = model->getState<std::vector<int>>("subgraphStack", std::vector<int>());
    if (navigationStack.empty() && currentSubgraphId < 0) return;
    
    ImVec2 windowSize = ImGui::GetWindowSize();
    float breadcrumbHeight = 30.0f;
    ImU32 breadcrumbBgColor = IM_COL32(40, 44, 52, 220);
    
    drawList->AddRectFilled(
        ImVec2(canvasPos.x, canvasPos.y),
        ImVec2(canvasPos.x + windowSize.x, canvasPos.y + breadcrumbHeight),
        breadcrumbBgColor
    );
    
    float x = canvasPos.x + 10.0f;
    float y = canvasPos.y + breadcrumbHeight * 0.5f;
    ImU32 textColor = IM_COL32(200, 200, 200, 255);
    ImU32 separatorColor = IM_COL32(100, 100, 100, 255);
    
    auto drawBreadcrumbItem = [&](int subgraphId, bool isLast) {
        const Subgraph* subgraph = model->getSubgraph(subgraphId);
        if (!subgraph) return;
        
        std::string name = subgraph->name;
        ImVec2 textSize = ImGui::CalcTextSize(name.c_str());
        
        drawList->AddText(
            ImVec2(x, y - textSize.y * 0.5f),
            textColor,
            name.c_str()
        );
        
        x += textSize.x + 5.0f;
        
        if (!isLast) {
            drawList->AddLine(
                ImVec2(x, y - 5.0f),
                ImVec2(x, y + 5.0f),
                separatorColor
            );
            x += 10.0f;
        }
    };
    
    drawList->AddText(
        ImVec2(x, y - ImGui::CalcTextSize("Root").y * 0.5f),
        textColor,
        "Root"
    );
    
    x += ImGui::CalcTextSize("Root").x + 5.0f;
    
    if (!navigationStack.empty() || currentSubgraphId >= 0) {
        drawList->AddLine(
            ImVec2(x, y - 5.0f),
            ImVec2(x, y + 5.0f),
            separatorColor
        );
        x += 10.0f;
    }
    
    for (size_t i = 0; i < navigationStack.size(); i++) {
        drawBreadcrumbItem(navigationStack[i], i == navigationStack.size() - 1 && currentSubgraphId < 0);
    }
    
    if (currentSubgraphId >= 0) {
        drawBreadcrumbItem(currentSubgraphId, true);
    }
}

void NodeEditorView::renderLayers(ImDrawList* drawList, const ImVec2& canvasPos) {
    std::vector<std::reference_wrapper<NodeEditorCore::internal::Layer>> sortedLayers;
    for (auto& [id, layer] : m_layers) {
        if (layer.visible) {
            sortedLayers.push_back(std::ref(layer));
        }
    }
    
    std::sort(sortedLayers.begin(), sortedLayers.end(), 
             [](const NodeEditorCore::internal::Layer& a, const NodeEditorCore::internal::Layer& b) {
                 return a.zOrder < b.zOrder;
             });
    
    for (auto& layer : sortedLayers) {
        layer.get().drawCallback(drawList, canvasPos);
    }
}

}
