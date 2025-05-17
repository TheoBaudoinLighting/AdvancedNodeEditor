#ifndef NODE_EDITOR_VIEW_H
#define NODE_EDITOR_VIEW_H

#include "../Controller/INodeEditorController.h"
#include <memory>
#include <vector>
#include <string>
#include <imgui.h>

#include "../../NodeEditor.h"
#include "../../Core/Style/StyleDefinitions.h"
#include "../../Core/Types/CoreTypes.h"

namespace NodeEditorCore {
    class NodeEditorView {
    public:
        NodeEditorView(std::shared_ptr<INodeEditorController> controller);

        ~NodeEditorView();

        void beginFrame();

        void render();

        void endFrame();

        void setViewPosition(const Vec2 &position);

        Vec2 getViewPosition() const;

        void setViewScale(float scale);

        float getViewScale() const;

        void centerView();

        void centerOnNode(int nodeId);

        void centerViewWithSize(float windowWidth, float windowHeight);

        void centerOnNodeWithSize(int nodeId, float windowWidth, float windowHeight);

        void setStyle(const EditorStyle &style);

        int addLayer(const std::string &name, int zOrder,
                     std::function<void(ImDrawList *, const ImVec2 &)> drawCallback);

        void removeLayer(int layerId);

        void setLayerVisible(int layerId, bool visible);

        void setLayerZOrder(int layerId, int zOrder);

    private:
        std::shared_ptr<INodeEditorController> m_controller;
        std::shared_ptr<NodeEditorCore::NodeEditor> m_coreEditor;

        std::map<int, UI::Layer> m_layers;
        int m_nextLayerId;

        Vec2 m_viewPosition;
        float m_viewScale;

        void drawSubgraphBreadcrumbs(ImDrawList *drawList, const ImVec2 &canvasPos);

        void renderLayers(ImDrawList *drawList, const ImVec2 &canvasPos);
    };
}

#endif
