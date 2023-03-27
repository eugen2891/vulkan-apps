#pragma once

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <type_traits>
#include <initializer_list>

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NATIVE_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#include <Volk/volk.h>
#include <glm/glm.hpp>
