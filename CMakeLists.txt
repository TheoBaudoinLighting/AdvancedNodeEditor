# CMakeLists.txt  
cmake_minimum_required(VERSION 3.31)
project(AdvancedNodeEditor)

set(CMAKE_CXX_STANDARD 20)

# === Options ===
option(USE_SYSTEM_IMGUI "Use system ImGui instead of downloading" OFF)
option(USE_SYSTEM_SDL2 "Use system SDL2 instead of downloading" OFF)
option(BUILD_TESTS "Build tests" ON)

# === Executable ===
add_executable(AdvancedNodeEditor
        exemple/main_exemple.cpp
        AdvancedNodeEditor/Core/Types/CoreTypes.h
        AdvancedNodeEditor/Core/Conversions/Conversions.h
        AdvancedNodeEditor/Core/Style/StyleDefinitions.cpp
        AdvancedNodeEditor/Core/Style/StyleDefinitions.h
        AdvancedNodeEditor/Components/Node/NodeEditorComponents.cpp
        AdvancedNodeEditor/Editor/Operations/NodeEditorState.cpp
        AdvancedNodeEditor/NodeEditor.h
        AdvancedNodeEditor/Components/Connection/NodeEditorConnections.cpp
        AdvancedNodeEditor/Components/Group/NodeEditorGroups.cpp
        AdvancedNodeEditor/Rendering/NodeEditorRendering.cpp
        AdvancedNodeEditor/Editor/Operations/NodeEditorInteractions.cpp
        AdvancedNodeEditor/Rendering/NodeEditorDrawing.cpp
        AdvancedNodeEditor/Rendering/NodeEditorDrawNodes.cpp
        AdvancedNodeEditor/Editor/Model/NodeEditorModel.h
        AdvancedNodeEditor/Editor/Controller/NodeEditorController.h
        AdvancedNodeEditor/Editor/View/NodeEditorView.h
        AdvancedNodeEditor/Editor/Model/NodeEditorModel.cpp
        AdvancedNodeEditor/Editor/Controller/NodeEditorController.cpp
        AdvancedNodeEditor/Editor/View/NodeEditorView.cpp
        AdvancedNodeEditor/Editor/Controller/INodeEditorController.h
        AdvancedNodeEditor/Core/Conversions/Conversions.cpp
        AdvancedNodeEditor/Editor/Operations/NodeEditorOperations.cpp
        AdvancedNodeEditor/Rendering/NodeEditorUtilities.cpp
        AdvancedNodeEditor/Rendering/NodeEditorDrawConnections.cpp
        AdvancedNodeEditor/Rendering/NodeEditorDrawGroups.cpp
        AdvancedNodeEditor/Components/Subgraph/NodeEditorSubgraphs.cpp
        AdvancedNodeEditor/Evaluation/NodeEditorEvaluation.h
        AdvancedNodeEditor/Evaluation/NodeEditorEvaluation.cpp
        AdvancedNodeEditor/Core/Style/InteractionMode.h
        AdvancedNodeEditor/Utils/UuidGenerator.h
        AdvancedNodeEditor/Evaluation/NodeEvaluator.h
        AdvancedNodeEditor/Editor/View/ViewManager.cpp
        AdvancedNodeEditor/Editor/View/ViewManager.h
        AdvancedNodeEditor/Editor/View/GraphTitleManager.cpp
        AdvancedNodeEditor/Editor/View/GraphTitleManager.h
        AdvancedNodeEditor/Core/Style/ConnectionStyleManager.cpp
        AdvancedNodeEditor/Core/Style/ConnectionStyleManager.h
        AdvancedNodeEditor/Editor/View/NodeBoundingBoxManager.cpp
        AdvancedNodeEditor/Editor/View/NodeBoundingBoxManager.h
        AdvancedNodeEditor/Editor/View/MinimapManager.cpp
        AdvancedNodeEditor/Editor/View/MinimapManager.h
        AdvancedNodeEditor/Components/UUID/NodeEditorUuidOperations.cpp
)

# === FetchContent for ImGui and SDL2 ===
include(FetchContent)

# --- ImGui ---
if(USE_SYSTEM_IMGUI)
    find_package(imgui REQUIRED)
    target_link_libraries(AdvancedNodeEditor PRIVATE imgui::imgui)
else()
    FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.91.6
    )
    FetchContent_MakeAvailable(imgui)

    target_sources(AdvancedNodeEditor PRIVATE
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    )
    target_include_directories(AdvancedNodeEditor PRIVATE
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
            ${imgui_SOURCE_DIR}/misc/cpp
    )
endif()

# --- SDL2 ---
if(USE_SYSTEM_SDL2)
    find_package(SDL2 REQUIRED)
    target_link_libraries(AdvancedNodeEditor PRIVATE SDL2::SDL2)
else()
    FetchContent_Declare(
            SDL2
            GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
            GIT_TAG release-2.26.5
    )
    FetchContent_MakeAvailable(SDL2)
    target_link_libraries(AdvancedNodeEditor PRIVATE SDL2::SDL2)
endif()

# === Includes and defs ===
target_compile_definitions(AdvancedNodeEditor PRIVATE IMGUI_DEFINE_MATH_OPERATORS)

# === Add OpenGL libraries ===
if(WIN32)
    target_link_libraries(AdvancedNodeEditor PRIVATE opengl32)
elseif(APPLE)
    find_package(OpenGL REQUIRED)
    target_link_libraries(AdvancedNodeEditor PRIVATE ${OPENGL_gl_LIBRARY})
else()
    find_package(OpenGL REQUIRED)
    target_link_libraries(AdvancedNodeEditor PRIVATE ${OPENGL_gl_LIBRARY})
endif()

# === Configure SDL2 main handling ===
target_compile_definitions(AdvancedNodeEditor PRIVATE SDL_MAIN_HANDLED)

# === Google Test ===
if(BUILD_TESTS)
    # Enable testing for the project
    enable_testing()

    # Fetch GoogleTest
    FetchContent_Declare(
            googletest
            GIT_REPOSITORY https://github.com/google/googletest.git
            GIT_TAG main
    )
    FetchContent_MakeAvailable(googletest)

    # Create test executable
    add_executable(node_editor_tests
            tests/main_test.cpp
            tests/core/NodeEditorTests.cpp
            tests/core/UuidGeneratorTests.cpp
            tests/components/NodeComponentsTests.cpp
            tests/components/ConnectionTests.cpp
            tests/components/GroupTests.cpp
            tests/components/SubgraphTests.cpp
            tests/evaluation/EvaluationTests.cpp
            tests/core/ConversionsTests.cpp
            tests/editor/ModelTests.cpp
            tests/editor/ControllerTests.cpp
            tests/editor/ViewTests.cpp
    )

    # Add ALL source files to test executable (excluding main.cpp)
    target_sources(node_editor_tests PRIVATE
            AdvancedNodeEditor/Core/Types/CoreTypes.h
            AdvancedNodeEditor/Core/Conversions/Conversions.h
            AdvancedNodeEditor/Core/Style/StyleDefinitions.cpp
            AdvancedNodeEditor/Core/Style/StyleDefinitions.h
            AdvancedNodeEditor/Components/Node/NodeEditorComponents.cpp
            AdvancedNodeEditor/Editor/Operations/NodeEditorState.cpp
            AdvancedNodeEditor/NodeEditor.h
            AdvancedNodeEditor/Components/Connection/NodeEditorConnections.cpp
            AdvancedNodeEditor/Components/Group/NodeEditorGroups.cpp
            AdvancedNodeEditor/Rendering/NodeEditorRendering.cpp
            AdvancedNodeEditor/Editor/Operations/NodeEditorInteractions.cpp
            AdvancedNodeEditor/Rendering/NodeEditorDrawing.cpp
            AdvancedNodeEditor/Rendering/NodeEditorDrawNodes.cpp
            AdvancedNodeEditor/Rendering/NodeEditorDrawConnections.cpp
            AdvancedNodeEditor/Rendering/NodeEditorDrawGroups.cpp
            AdvancedNodeEditor/Editor/Operations/NodeEditorOperations.cpp
            AdvancedNodeEditor/Rendering/NodeEditorUtilities.cpp
            AdvancedNodeEditor/Components/Subgraph/NodeEditorSubgraphs.cpp
            AdvancedNodeEditor/Components/UUID/NodeEditorUuidOperations.cpp
            AdvancedNodeEditor/Core/Conversions/Conversions.cpp
            AdvancedNodeEditor/Evaluation/NodeEditorEvaluation.cpp
            AdvancedNodeEditor/Core/Style/InteractionMode.h
            AdvancedNodeEditor/Utils/UuidGenerator.h
            AdvancedNodeEditor/Editor/Model/NodeEditorModel.cpp
            AdvancedNodeEditor/Editor/Controller/NodeEditorController.cpp
            AdvancedNodeEditor/Editor/View/NodeEditorView.cpp
            AdvancedNodeEditor/Editor/View/ViewManager.cpp
            AdvancedNodeEditor/Editor/View/GraphTitleManager.cpp
            AdvancedNodeEditor/Core/Style/ConnectionStyleManager.cpp
            AdvancedNodeEditor/Editor/View/NodeBoundingBoxManager.cpp
            AdvancedNodeEditor/Editor/View/MinimapManager.cpp
    )

    # Add ImGui and SDL sources to tests
    if(NOT USE_SYSTEM_IMGUI)
        target_sources(node_editor_tests PRIVATE
                ${imgui_SOURCE_DIR}/imgui.cpp
                ${imgui_SOURCE_DIR}/imgui_draw.cpp
                ${imgui_SOURCE_DIR}/imgui_widgets.cpp
                ${imgui_SOURCE_DIR}/imgui_tables.cpp
                ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
        )
        target_include_directories(node_editor_tests PRIVATE
                ${imgui_SOURCE_DIR}
                ${imgui_SOURCE_DIR}/misc/cpp
        )
    endif()

    # Link with Google Test and other dependencies
    target_link_libraries(node_editor_tests PRIVATE
            GTest::gtest
            GTest::gtest_main
    )

    if(USE_SYSTEM_IMGUI)
        target_link_libraries(node_editor_tests PRIVATE imgui::imgui)
    endif()

    if(USE_SYSTEM_SDL2)
        target_link_libraries(node_editor_tests PRIVATE SDL2::SDL2)
    else()
        target_link_libraries(node_editor_tests PRIVATE SDL2::SDL2)
    endif()

    # OpenGL libraries for tests
    if(WIN32)
        target_link_libraries(node_editor_tests PRIVATE opengl32)
    elseif(APPLE)
        target_link_libraries(node_editor_tests PRIVATE ${OPENGL_gl_LIBRARY})
    else()
        target_link_libraries(node_editor_tests PRIVATE ${OPENGL_gl_LIBRARY})
    endif()

    # Add tests to CTest
    include(GoogleTest)
    gtest_discover_tests(node_editor_tests)
endif()