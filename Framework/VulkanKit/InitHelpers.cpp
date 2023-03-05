#include "InitHelpers.hpp"

vulkan::PhysicalDevicesList::PhysicalDevicesList(VkInstance instance) : Array<VkPhysicalDevice>()
{
	BreakIfFailed(vkEnumeratePhysicalDevices(instance, &num, items));
	if (num > 0)
	{
		items = new VkPhysicalDevice[num];
		BreakIfFailed(vkEnumeratePhysicalDevices(instance, &num, items));
	}
}

vulkan::QueueFamiliesList::QueueFamiliesList(VkPhysicalDevice physicalDevice) : Array<VkQueueFamilyProperties>()
{
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &num, items);
	if (num > 0)
	{
		items = new VkQueueFamilyProperties[num];
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &num, items);
	}
}

uint32_t vulkan::QueueFamiliesList::findByFlags(VkQueueFlags incl, VkQueueFlags excl) const
{
	for (uint32_t i = 0; i < num; i++)
	{
		if (items[i].queueCount && ((items[i].queueFlags & incl) == incl) && ((items[i].queueFlags & excl) == 0)) return i;
	}
	return num;
}

vulkan::DeviceQueueCreateList::DeviceQueueCreateList(uint32_t count) : Array<VkDeviceQueueCreateInfo>()
{
	static float defaultPrio = 1.f;
	num = count;
	if (num)
	{
		items = new VkDeviceQueueCreateInfo[num];
		for (VkDeviceQueueCreateInfo& item : *this)
		{
			item.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			item.pNext = nullptr;
			item.flags = 0;
			item.queueFamilyIndex = UINT32_MAX;
			item.queueCount = 0;
			item.pQueuePriorities = &defaultPrio;
		}
	}	
}

vulkan::SurfaceFormatsList::SurfaceFormatsList(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) : Array<VkSurfaceFormatKHR>()
{
	BreakIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &num, items));
	if (num > 0)
	{
		items = new VkSurfaceFormatKHR[num];
		BreakIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &num, items));
	}
}

vulkan::PresentModesList::PresentModesList(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) : Array<VkPresentModeKHR>()
{
	BreakIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &num, items));
	if (num > 0)
	{
		items = new VkPresentModeKHR[num];
		BreakIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &num, items));
	}
}

vulkan::SwapchainImagesList::SwapchainImagesList(VkDevice device, VkSwapchainKHR swapchain) : Array<VkImage>()
{
	BreakIfFailed(vkGetSwapchainImagesKHR(device, swapchain, &num, items));
	if (num > 0)
	{
		items = new VkImage[num];
		BreakIfFailed(vkGetSwapchainImagesKHR(device, swapchain, &num, items));
	}
}

vulkan::PhysicalDeviceMemoryProperties::PhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice)
{
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, this);
}

uint32_t vulkan::PhysicalDeviceMemoryProperties::findMemoryType(const VkMemoryRequirements& reqs, VkMemoryPropertyFlags flags, VkMemoryPropertyFlags exclude, VkMemoryPropertyFlags maybe) const
{
	uint32_t retval = memoryTypeCount;
	for (uint32_t i = 0; i < memoryTypeCount; i++)
	{
		if ((reqs.memoryTypeBits & (1 << i)) && (memoryTypes[i].propertyFlags & flags))
		{
			retval = i;
			break;
		}
	}
	return retval;
}

const char* ToString(VkResult value)
{
	switch (value)
	{
	case VK_SUCCESS:
		return "VK_SUCCESS";
	case VK_NOT_READY:
		return "VK_NOT_READY";
	case VK_TIMEOUT:
		return "VK_TIMEOUT";
	case VK_EVENT_SET:
		return "VK_EVENT_SET";
	case VK_EVENT_RESET:
		return "VK_EVENT_RESET";
	case VK_INCOMPLETE:
		return "VK_INCOMPLETE";
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return "VK_ERROR_OUT_OF_HOST_MEMORY";
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
	case VK_ERROR_INITIALIZATION_FAILED:
		return "VK_ERROR_INITIALIZATION_FAILED";
	case VK_ERROR_DEVICE_LOST:
		return "VK_ERROR_DEVICE_LOST";
	case VK_ERROR_MEMORY_MAP_FAILED:
		return "VK_ERROR_MEMORY_MAP_FAILED";
	case VK_ERROR_LAYER_NOT_PRESENT:
		return "VK_ERROR_LAYER_NOT_PRESENT";
	case VK_ERROR_EXTENSION_NOT_PRESENT:
		return "VK_ERROR_EXTENSION_NOT_PRESENT";
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return "VK_ERROR_FEATURE_NOT_PRESENT";
	case VK_ERROR_INCOMPATIBLE_DRIVER:
		return "VK_ERROR_INCOMPATIBLE_DRIVER";
	case VK_ERROR_TOO_MANY_OBJECTS:
		return "VK_ERROR_TOO_MANY_OBJECTS";
	case VK_ERROR_FORMAT_NOT_SUPPORTED:
		return "VK_ERROR_FORMAT_NOT_SUPPORTED";
	case VK_ERROR_FRAGMENTED_POOL:
		return "VK_ERROR_FRAGMENTED_POOL";
	case VK_ERROR_UNKNOWN:
		return "VK_ERROR_UNKNOWN";
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return "VK_ERROR_OUT_OF_POOL_MEMORY";
	case VK_ERROR_INVALID_EXTERNAL_HANDLE:
		return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
	case VK_ERROR_FRAGMENTATION:
		return "VK_ERROR_FRAGMENTATION";
	case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
		return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
	case VK_PIPELINE_COMPILE_REQUIRED:
		return "VK_PIPELINE_COMPILE_REQUIRED";
	case VK_ERROR_SURFACE_LOST_KHR:
		return "VK_ERROR_SURFACE_LOST_KHR";
	case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
		return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
	case VK_SUBOPTIMAL_KHR:
		return "VK_SUBOPTIMAL_KHR";
	case VK_ERROR_OUT_OF_DATE_KHR:
		return "VK_ERROR_OUT_OF_DATE_KHR";
	case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
		return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
	case VK_ERROR_VALIDATION_FAILED_EXT:
		return "VK_ERROR_VALIDATION_FAILED_EXT";
	case VK_ERROR_INVALID_SHADER_NV:
		return "VK_ERROR_INVALID_SHADER_NV";
	case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
		return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
	case VK_ERROR_NOT_PERMITTED_KHR:
		return "VK_ERROR_NOT_PERMITTED_KHR";
	case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
		return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
	case VK_THREAD_IDLE_KHR:
		return "VK_THREAD_IDLE_KHR";
	case VK_THREAD_DONE_KHR:
		return "VK_THREAD_DONE_KHR";
	case VK_OPERATION_DEFERRED_KHR:
		return "VK_OPERATION_DEFERRED_KHR";
	case VK_OPERATION_NOT_DEFERRED_KHR:
		return "VK_OPERATION_NOT_DEFERRED_KHR";
	case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
		return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
	default:
		return nullptr;
	};
}
