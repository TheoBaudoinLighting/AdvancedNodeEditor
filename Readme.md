# Advanced Node Editor

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![Version](https://img.shields.io/badge/version-1.0.0-green.svg)](https://github.com/yourorg/nodeeditor/releases)
[![Documentation](https://img.shields.io/badge/docs-latest-brightgreen.svg)](https://yourorg.github.io/nodeeditor/)
[![Downloads](https://img.shields.io/github/downloads/TheoBaudoinLighting/AdvancedNodeEditor/total?style=flat-square)](https://github.com/TheoBaudoinLighting/AdvancedNodeEditor/releases)

A modern, robust C++20 framework for creating interactive graphical node editors with hierarchical subgraph support, real-time evaluation, and intuitive user interface.

## Features

- **Complete Node Management**: Creation, deletion, selection, and manipulation
- **Connection System**: Node-to-node connections with type validation
- **Hierarchical Subgraphs**: Full support for nested graph structures
- **Real-time Evaluation**: Dependency-ordered evaluation system
- **Modern Interface**: Interactive view with zoom, pan, and automatic centering
- **Visual Groups**: Logical organization of nodes into groups
- **Connection Rerouting**: Redirection points for complex connections
- **Event System**: Real-time event notifications via callbacks
- **Clean API**: Intuitive interface for seamless integration

### (Figure 1 : Little game engine made with advanced node editor)
![image](https://github.com/user-attachments/assets/16b3ddc6-ae1b-4c63-8a93-942dafcba0da)

## Installation

### Prerequisites

- C++20 compatible compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.16+
- Rendering system dependencies (ImGui, OpenGL, etc.)

### Build

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

### Integration

```cpp
#include "NodeEditorAPI.h"
using namespace NodeEditorCore;
```

## Quick Start

### Basic Initialization

```cpp
NodeEditorAPI editor;
editor.initialize();
editor.setWindowSize(1280, 720);
```

### Node Type Definition

```cpp
NodeEditorAPI::NodeDefinition mathNode;
mathNode.type = "math_add";
mathNode.name = "Addition";
mathNode.category = "Math";
mathNode.description = "Adds two numeric values";
mathNode.iconSymbol = "+";
mathNode.inputs = {{"A", PinType::Float}, {"B", PinType::Float}};
mathNode.outputs = {{"Result", PinType::Float}};

editor.registerNodeType(mathNode);
```

### Node Creation and Connection

```cpp
UUID nodeA = editor.createNode("math_add", "Addition 1", Vec2(100, 100));
UUID nodeB = editor.createNode("math_add", "Addition 2", Vec2(300, 100));

UUID connection = editor.connectNodes(nodeA, "Result", nodeB, "A");
```

### Render Loop

```cpp
while (running) {
    editor.beginFrame();
    editor.render();
    editor.endFrame();
}
```

## API Reference

### Node Management

- `createNode(type, name, position)` - Creates a new node
- `removeNode(nodeId)` - Removes a node
- `selectNode(nodeId, append)` - Selects a node
- `deselectAllNodes()` - Deselects all nodes
- `getSelectedNodes()` - Gets selected nodes

### Connection Management

- `connectNodes(startNode, outputPin, endNode, inputPin)` - Connects two nodes
- `disconnectNodes(connectionId)` - Disconnects nodes
- `addRerouteToConnection(connectionId, position)` - Adds reroute point

### View Management

- `centerView()` - Centers view on all nodes
- `centerOnNode(nodeId)` - Centers view on specific node
- `zoomToFit(padding)` - Fits all nodes in view
- `setViewPosition(position)` - Sets view position
- `setViewScale(scale)` - Sets zoom level

### Subgraph Management

- `createGraph(name)` - Creates new graph
- `switchToGraph(graphId)` - Switches to graph
- `enterSubgraph(subgraphId)` - Enters subgraph
- `exitSubgraph()` - Exits current subgraph

### Evaluation System

- `registerEvaluator(nodeType, evaluator)` - Registers evaluation function
- `evaluateGraph(outputNodeId)` - Evaluates graph
- `setConstantValue(nodeId, value)` - Sets constant value
- `getConstantValue(nodeId)` - Gets constant value

### Event Callbacks

- `setNodeCreatedCallback(callback)` - Node creation callback
- `setNodeRemovedCallback(callback)` - Node removal callback
- `setConnectionCreatedCallback(callback)` - Connection creation callback
- `setConnectionRemovedCallback(callback)` - Connection removal callback

## Data Types

### NodeDefinition

```cpp
struct NodeDefinition {
    std::string type;                                             // Unique node type
    std::string name;                                             // Display name
    std::string category;                                         // Organization category
    std::string description;                                      // Tooltip description
    std::string iconSymbol;                                       // Display icon
    std::vector<std::pair<std::string, PinType>> inputs;         // Input pins
    std::vector<std::pair<std::string, PinType>> outputs;        // Output pins
};
```

### EvaluationResult

```cpp
struct EvaluationResult {
    std::any value;                      // Result value
    std::vector<UUID> evaluationOrder;   // Evaluation order
};
```

## Advanced Examples

### Mathematical Evaluation System

```cpp
editor.registerEvaluator("math_add", [](const std::vector<std::any>& inputs) -> std::any {
    if (inputs.size() >= 2) {
        try {
            float a = std::any_cast<float>(inputs[0]);
            float b = std::any_cast<float>(inputs[1]);
            return a + b;
        } catch (const std::bad_any_cast&) {
            return 0.0f;
        }
    }
    return 0.0f;
});
```

### Connection Path Tracing

```cpp
std::vector<UUID> path = editor.traceConnectionPath(
    startNodeId, "output",
    endNodeId, "input"
);

for (const auto& connectionId : path) {
    // Process each connection in path
}
```

### Group Management

```cpp
UUID groupId = editor.createGroup("Math Operations", Vec2(50, 50), Vec2(400, 300));
editor.addNodeToGroup(nodeA, groupId);
editor.addNodeToGroup(nodeB, groupId);
```

### Event Handling

```cpp
editor.setNodeCreatedCallback([](const UUID& nodeId) {
    std::cout << "Node created: " << nodeId << std::endl;
});

editor.setConnectionCreatedCallback([](const UUID& connectionId) {
    std::cout << "Connection created: " << connectionId << std::endl;
});
```

## Advanced Configuration

### Custom Pin Types

The framework supports various pin types through the `PinType` enumeration. You can extend the system to support custom types.

### Command System

```cpp
editor.executeCommand("custom_command", customData);
```

### Direct Editor Access

```cpp
NodeEditor* underlyingEditor = editor.getUnderlyingEditor();
// Direct access to advanced functionality
```

## Customization

The framework is designed to be extensible:

- **Custom Rendering**: Implement custom renderers
- **Node Types**: Create complex node types
- **Evaluators**: Flexible evaluation system
- **Interface**: Full UI customization

## Performance

- **Optimizations**: Efficient rendering with automatic culling
- **Memory**: Intelligent resource management
- **Multithreading**: Parallel evaluation support

## Error Handling

### Exception Management

```cpp
try {
    editor.zoomToFit();
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

### Validation

- Automatic connection validation
- Pin type checking
- Cycle detection in graphs

## Contributing

1. Fork the project
2. Create a feature branch (`git checkout -b feature/new-feature`)
3. Commit your changes (`git commit -am 'Add new feature'`)
4. Push to the branch (`git push origin feature/new-feature`)
5. Create a Pull Request

## License

MIT License

Copyright (c) 2025 Theo Baudoin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

***THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.***

## Links

(in progress -> links are now place holder sorry)
- [Complete API Documentation](docs/API.md)
- [Getting Started Guide](docs/GETTING_STARTED.md)
- [Examples](examples/)
- [Changelog](CHANGELOG.md)

---

**Advanced Node Editor Framework** - Build powerful and intuitive node editors with C++20

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
