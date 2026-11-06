# Speed-Engine-Project

C++23 game engine project. The Visual Studio project builds the engine and copies `SpeedEngine/Assets` beside the executable.

## Features
### Rendering
- OpenGL 4.5 rendering through ordered render passes.
- Deferred lighting for opaque geometry.
- PBR materials with metallic and roughness maps.
- Forward rendering for transparent meshes.
- PCSS shadow mapping.
- Image-based lighting.
- HDR tone mapping.
- Billboard particle rendering.

### Scene and Simulation
- Component-based entities.
- JSON scene loading and saving.
- Jolt rigid-body physics.

### Input and UI
- Keyboard and mouse input.
- Event subscriptions for engine communication.
- Command-based UI focus navigation.
- Adjustable editor camera controls.

### Editor
- Dockable ImGui editor panels.
- Scene-file controls.
- Entity creation and component editing.

### Assets and Audio
- Audio playback through miniaudio.
- 3D model import through Assimp.
- Texture loading through stb.
- Font rendering.

### Scripting
- Lua scripts attached to entities.
- Lua bindings for components and systems.

## Architecture
`SE::Engine` owns and initializes the engine subsystems. Systems cover windowing, input, events, resources, rendering, physics, scenes, scripting, audio, and UI. Simulation updates run at a fixed 60 Hz and publish render frames through `FrameExchange`; the render loop consumes those frames. The `Editor|x64` configuration runs the update and render loops on one thread.

## Dependencies
`SpeedEngine/vcpkg.json` defines the vcpkg manifest dependencies:

- GLFW, GLAD, and Dear ImGui for windowing, OpenGL loading, and editor UI.
- GLM and nlohmann-json for math and JSON data.
- Jolt Physics, Lua, and sol2 for physics and scripting.
- Assimp, stb, FreeType, and miniaudio for asset loading, fonts, and audio.
- spdlog for logging.

## Build
- Open `SpeedEngine/SpeedEngine.vcxproj` in Visual Studio and build `Debug|x64`.
- Visual Studio restores the vcpkg manifest dependencies on the first build.
- The project uses the MSVC v145 toolset and C++23.
- `SpeedEngine/ThirdParty` contains the precompiled-header files only.

## Project Layout
- `SpeedEngine/src/Core`: engine lifecycle, features, resource managers, scenes, subsystems, and threading.
- `SpeedEngine/Assets`: shaders, scripts, meshes, textures, and sample scene data.

## Contributions
- Work on a branch and open a PR for review. Do not push directly to `main`.