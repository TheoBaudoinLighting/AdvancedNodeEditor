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
        ContextMenu
    };

}

#endif //INTERACTIONMODE_H
