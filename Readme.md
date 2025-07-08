# Advanced Node Editor

A powerful and flexible node-based visual editor library built with ImGui. This library provides a complete framework for creating node-based interfaces for visual programming, data flow systems, and other node-graph applications.

### (Figure 1 : Little game engine made with advanced node editor)
![image](https://github.com/user-attachments/assets/16b3ddc6-ae1b-4c63-8a93-942dafcba0da)

## Features

- Intuitive node-based interface with customizable styles
- Support for different node and pin types with automatic styling
- Smooth connections with bezier curves
- Connection validation system
- Pin shape customization (Circle, Square, Triangle, Diamond)
- Node grouping functionality
- Zoom and pan navigation
- Selection, deletion, and other editing operations
- Visual effects like shadows, glows, and highlights
- Customizable styling system

## Requirements

- C++17 compatible compiler
- CMake 3.31 or higher
- OpenGL support

The project can automatically download and build dependencies through CMake's FetchContent:
- ImGui (v1.91.6)
- SDL2 (v2.26.5)

## Building the Project

### Clone the Repository

```bash
git clone https://github.com/TheoBaudoinLighting/AdvancedNodeEditor.git
cd AdvancedNodeEditor
```

### Build with CMake

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### CMake Options

- `USE_SYSTEM_IMGUI`: Set to ON to use system-installed ImGui instead of downloading (default: OFF)
- `USE_SYSTEM_SDL2`: Set to ON to use system-installed SDL2 instead of downloading (default: OFF)

Example:
```bash
cmake -DUSE_SYSTEM_SDL2=ON ..
```

## Usage

```cpp
#include "AdvancedNodeEditor/AdvancedNodeEditor.h"

// Create a node editor instance
ANE::NodeEditor editor;

void setup() {
    // Create nodes
    int nodeId = editor.addNode("Math", "Math", ANE::Vec2(100, 100));
    
    // Create pins
    editor.addPin(nodeId, "Value A", true, ANE::PinType::Float);
    editor.addPin(nodeId, "Value B", true, ANE::PinType::Float);
    editor.addPin(nodeId, "Result", false, ANE::PinType::Float);
    
    // Setup validation logic
    editor.setCanConnectCallback([](const ANE::Pin& a, const ANE::Pin& b) {
        return a.type == b.type;
    });
}

void render() {
    // In your render loop
    editor.beginFrame();
    editor.render();
    editor.endFrame();
}
```

## Project Structure

- `AdvancedNodeEditor/` - Core library files
  - `AdvancedNodeEditor.h` - Main header file
  - `AdvancedNodeEditor.cpp` - Implementation file
- `main.cpp` - Example application

## Customization

The library offers extensive customization options:

```cpp
// Customize editor style
ANE::EditorStyle style;
style.backgroundColor = ANE::Color(0.15f, 0.16f, 0.18f, 1.0f);
style.gridColor = ANE::Color(0.2f, 0.21f, 0.23f, 0.5f);
style.nodeRounding = 6.0f;
style.pinRadius = 5.0f;
editor.setStyle(style);
```

## Platform Support

- Windows
- macOS
- Linux

## License

MIT License

Copyright (c) 2023 Theo Baudoin

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request.
