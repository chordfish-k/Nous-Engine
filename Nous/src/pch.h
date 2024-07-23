#pragma once

// 预编译头
#include <iostream>
#include <memory>
#include <algorithm>
#include <functional>
#include <filesystem>

#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "Nous/Core/Log.h"

#ifdef NS_PLATFORM_WINDOWS
#include <Windows.h>
#endif

namespace fs = std::filesystem;