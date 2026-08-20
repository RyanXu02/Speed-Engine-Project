// Do not add files here that will be updated frequently 
// as this negates the performance advantage of precompile header.
#pragma once

// reduce Windows header size and conflicts
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
// Include before GLFW to avoid macro redefinition
#include <Windows.h>

#include <stdio.h>

#include <algorithm>
#include <any>
#include <array>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <ranges>
#include <set>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

#include <bitset>

#include <Jolt/Jolt.h>
