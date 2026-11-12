// Do not add files here that will be updated frequently
// as this negates the performance advantage of precompile header.
#pragma once

// reduce Windows header size and conflicts
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
// Include before GLFW to avoid macro redefinition
#include <Windows.h>

// glm swizzle
#define GLM_FORCE_SWIZZLE

#define JPH_DEBUG_RENDERER

#include <Jolt/Jolt.h>
#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <bitset>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <thread>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>
