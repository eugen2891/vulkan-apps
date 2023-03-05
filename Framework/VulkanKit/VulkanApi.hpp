#pragma once

#include "../Utilities/Debug.hpp"

#if defined(_WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#define VK_NATIVE_SURFACE_EXTENSION_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#define ReturnIfFailed(expr) ReturnIfNot((expr) == VK_SUCCESS)
#define RetvalIfFailed(expr, val) RetvalIfNot((expr) == VK_SUCCESS, val)
#define BreakIfFailed(expr) BreakIfNot((expr) == VK_SUCCESS)

#include <Volk/volk.h>

namespace vulkan
{

class APIState
{
public:
	VkInstance instance() const { return m_instance; }
	VkAllocationCallbacks* alloc() const { return m_alloc; }
	VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }
	VkDevice device() const { return m_device; }
protected:
	VkInstance m_instance = VK_NULL_HANDLE;
	VkAllocationCallbacks* m_alloc = nullptr;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
};

class APIClient
{
public:
	static const char* toString(VkResult);
protected:
	explicit APIClient(const APIState& vk) : m_vk(vk) {}
	const APIState& m_vk;
};

}
