# Speed-Engine-Project
Speed run making game engine :thumb:

## Dependencies
`SpeedEngine` uses vcpkg manifest mode for third-party dependencies.

- Open `SpeedEngine/SpeedEngine.vcxproj` in Visual Studio and build `Debug|x64`.
- Visual Studio restores packages from `SpeedEngine/vcpkg.json` automatically on first build.
- The committed `SpeedEngine/ThirdParty` folder only keeps `pch.cpp` and `pch.h`.
