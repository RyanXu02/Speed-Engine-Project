# Speed-Engine-Project
Speed run making game engine :thumb:

## Dependencies
`SpeedEngine` uses vcpkg manifest mode for third-party dependencies.

- Open `SpeedEngine/SpeedEngine.vcxproj` in Visual Studio and build `Debug|x64`.
- Visual Studio restores packages from `SpeedEngine/vcpkg.json` automatically on first build.
- The `SpeedEngine/ThirdParty` folder only keeps `pch.cpp` and `pch.h`.

## Contributions
- Do not push to main!
- Make your changes to a branch, and open a PR to get it reviewed, audited, and merged.
- Pay attention to the Linter, it will auto-fix any linting problems whenever changes are pushed to the PR. Make sure to pull frequently to avoid local complications.
