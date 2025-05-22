#ifndef INTERACTIONMODE_H
#define INTERACTIONMODE_H

namespace NodeEditorCore {
    enum class InteractionMode {
        None,
        PanCanvas,
        BoxSelect,
        DragNode,
        ResizeNode,
        DragConnection,
        DragGroup,
        ResizeGroup,
        ContextMenu,
        DragReroute,
        DragRerouteConnection
    };
}

#endif //INTERACTIONMODE_H
