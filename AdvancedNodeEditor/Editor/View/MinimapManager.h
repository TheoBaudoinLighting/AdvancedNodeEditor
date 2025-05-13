// MinimapManager.h
#ifndef MINIMAP_MANAGER_H
#define MINIMAP_MANAGER_H

#include "../../Core/Types/CoreTypes.h"
#include <imgui.h>
#include <vector>
#include <functional>

namespace NodeEditorCore {

class MinimapManager {
public:
    struct MinimapConfig {
        Vec2 position;        // Position dans la fenêtre (0,0 = coin supérieur gauche)
        Vec2 size;            // Taille de la minimap
        float opacity;        // Opacité globale
        Color backgroundColor; // Couleur de fond
        Color borderColor;    // Couleur de bordure
        Color viewportColor;  // Couleur du rectangle de viewport
        float padding;        // Marge intérieure
        bool showBorder;      // Afficher la bordure
        bool interactable;    // Permet de cliquer/déplacer sur la minimap

        MinimapConfig()
            : position(20.0f, 20.0f)
            , size(200.0f, 150.0f)
            , opacity(0.7f)
            , backgroundColor(0.1f, 0.1f, 0.12f, 0.8f)
            , borderColor(0.3f, 0.3f, 0.3f, 1.0f)
            , viewportColor(0.9f, 0.9f, 0.9f, 0.2f)
            , padding(10.0f)
            , showBorder(true)
            , interactable(true)
        {}
    };

    MinimapManager();
    ~MinimapManager();

    void setConfig(const MinimapConfig& config);
    MinimapConfig& getConfig();
    const MinimapConfig& getConfig() const;

    void setViewBounds(const Vec2& min, const Vec2& max);
    void setViewPosition(const Vec2& position);
    void setViewScale(float scale);

    // Définir une fonction pour obtenir les positions des nœuds
    using NodePositionProvider = std::function<std::vector<std::pair<Vec2, Vec2>>()>;
    void setNodePositionProvider(NodePositionProvider provider);

    // Fonction de rappel quand l'utilisateur clique sur la minimap
    using ViewportChangeCallback = std::function<void(const Vec2&)>;
    void setViewportChangeCallback(ViewportChangeCallback callback);

    // Dessiner la minimap
    bool draw(ImDrawList* drawList, const ImVec2& canvasPos, const ImVec2& canvasSize);

private:
    MinimapConfig m_config;
    Vec2 m_viewMin;
    Vec2 m_viewMax;
    Vec2 m_viewPosition;
    float m_viewScale;
    NodePositionProvider m_nodePositionProvider;
    ViewportChangeCallback m_viewportChangeCallback;
    bool m_dragging;
    Vec2 m_dragStart;

    // Convertir coordonnées du graphe en coordonnées minimap
    ImVec2 graphToMinimap(const Vec2& graphPos, const ImVec2& minimapPos, const ImVec2& minimapSize) const;

    // Convertir coordonnées minimap en coordonnées graphe
    Vec2 minimapToGraph(const ImVec2& minimapPos, const ImVec2& mapPos, const ImVec2& mapSize) const;

    // Gérer les interactions
    bool handleInteraction(const ImVec2& minimapPos, const ImVec2& minimapSize);
};

} // namespace NodeEditorCore

#endif // MINIMAP_MANAGER_H