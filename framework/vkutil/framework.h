#pragma once

#include <stdint.h>

#define VK_NO_PROTOTYPES

#define VKUTIL_WINDOW_W 512
#define VKUTIL_WINDOW_H 512

#if !defined(VKUTIL_VALIDATION)
#define VKUTIL_VALIDATION 0
#endif

#if defined(_WIN32)

#define VK_USE_PLATFORM_WIN32_KHR
#define WIN32_LEAN_AND_MEAN

#endif

#include <vulkan/vulkan.h>

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#define VKUTIL_KHR_SURFACE_EXTENSION_IMPL VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#define VKUTIL_INSTANCE_DEAFULT_EXT VK_KHR_SURFACE_EXTENSION_NAME, VKUTIL_KHR_SURFACE_EXTENSION_IMPL

#define VKUTIL_DEVICE_DEAFULT_EXT VK_KHR_SWAPCHAIN_EXTENSION_NAME

template <typename T, uint32_t N> 
constexpr uint32_t CountOf(T(&)[N]) { return N; }

namespace vkutil
{

    bool CheckResult(VkResult result, const char* pFile, int lineNo);

}

#if _DEBUG

#define VKUTIL_CHECK_RETURN(c, r) \
{ if (!vkutil::CheckResult((c), __FILE__, __LINE__)) return r; }

#define VKUTIL_CHECK(c) \
{ vkutil::CheckResult((c), __FILE__, __LINE__); }

#else

#define VKUTIL_CHECK_RETURN(c, r) \
{ if ((c) != VK_SUCCESS) return r; }

#define VKUTIL_CHECK(c) c

#endif