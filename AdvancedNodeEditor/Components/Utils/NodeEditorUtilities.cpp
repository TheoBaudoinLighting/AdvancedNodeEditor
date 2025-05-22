#include "../../Core/NodeEditor.h"

namespace NodeEditorCore {
    ImVec2 NodeEditor::evaluateBezierCubic(const ImVec2 &p0, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3,
                                           float t) const {
        float u = 1.0f - t;
        float tt = t * t;
        float uu = u * u;
        float uuu = uu * u;
        float ttt = tt * t;

        return ImVec2(
            uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x,
            uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y
        );
    }

    bool NodeEditor::isPointNearCubicBezier(const ImVec2 &point, const ImVec2 &p1, const ImVec2 &p2, const ImVec2 &p3,
                                            const ImVec2 &p4, float threshold) const {
        const int steps = 10;
        ImVec2 prev = p1;

        for (int i = 1; i <= steps; ++i) {
            float t = i / (float) steps;
            float u = 1.0f - t;

            float w1 = u * u * u;
            float w2 = 3 * u * u * t;
            float w3 = 3 * u * t * t;
            float w4 = t * t * t;

            ImVec2 current(
                w1 * p1.x + w2 * p2.x + w3 * p3.x + w4 * p4.x,
                w1 * p1.y + w2 * p2.y + w3 * p3.y + w4 * p4.y
            );

            if (isPointNearLine(point, prev, current, threshold)) {
                return true;
            }

            prev = current;
        }

        return false;
    }
}
