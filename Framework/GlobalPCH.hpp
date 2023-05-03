#pragma once

#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cstdint>

#include <deque>
#include <vector>
#include <algorithm>
#include <type_traits>
#include <initializer_list>

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NATIVE_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#include <Volk/volk.h>
#include <glm/glm.hpp>

#ifdef _DEBUG

#define ReturnIfNot(expr) { \
	if (!(expr)) { __debugbreak(); return; } \
}

#define RetvalIfNot(expr, val) { \
	if (!(expr)) { __debugbreak(); return (val); } \
}

#define BreakIfNot(expr) { \
	if (!(expr)) { __debugbreak(); abort(); } \
}

#else

#define ReturnIfNot(expr) { \
	if (!(expr)) return; \
}

#define RetvalIfNot(expr, val) { \
	if (!(expr)) return (val); \
}

#define BreakIfNot(expr) (expr)

#endif

namespace util
{

template <typename T>
constexpr T Min(const T& a, const T& b) noexcept
{
	return (a < b) ? a : b;
}

template <typename T>
constexpr T Max(const T& a, const T& b) noexcept
{
	return (a > b) ? a : b;
}

}

