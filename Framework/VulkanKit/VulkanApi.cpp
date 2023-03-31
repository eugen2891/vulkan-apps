#include <GlobalPCH.hpp>

#include "VulkanApi.hpp"

namespace vulkan
{

std::vector<VkPhysicalDevice> APIState::enumPhysicalDevices() const noexcept
{
	uint32_t num = 0;
	std::vector<VkPhysicalDevice> retval{};
	BreakIfFailed(vkEnumeratePhysicalDevices(m_instance, &num, nullptr));
	if (num > 0)
	{
		retval.resize(num);
		BreakIfFailed(vkEnumeratePhysicalDevices(m_instance, &num, retval.data()));
	}
	return retval;
}

VkQueue APIState::getDeviceQueue(uint32_t family, uint32_t index) const noexcept
{
	VkQueue retval = VK_NULL_HANDLE;
	vkGetDeviceQueue(m_device, family, index, &retval);
	return retval;
}

std::vector<VkImage> APIState::getSwapchainImages(VkSwapchainKHR swapchain) const noexcept
{
	uint32_t num = 0;
	std::vector<VkImage> retval{};
	BreakIfFailed(vkGetSwapchainImagesKHR(m_device, swapchain, &num, nullptr));
	if (num > 0)
	{
		retval.resize(num);
		BreakIfFailed(vkGetSwapchainImagesKHR(m_device, swapchain, &num, retval.data()));
	}
	return retval;
}

std::vector<VkPresentModeKHR> APIState::enumPresentModes(VkSurfaceKHR surface) const noexcept
{
	uint32_t num = 0;
	std::vector<VkPresentModeKHR> retval{};
	BreakIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &num, nullptr));
	if (num > 0)
	{
		retval.resize(num);
		BreakIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, surface, &num, retval.data()));
	}
	return retval;
}

std::vector<VkSurfaceFormatKHR> APIState::enumSurfaceFormats(VkSurfaceKHR surface) const noexcept
{
	uint32_t num = 0;
	std::vector<VkSurfaceFormatKHR> retval{};
	BreakIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &num, nullptr));
	if (num > 0)
	{
		retval.resize(num);
		BreakIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, surface, &num, retval.data()));
	}
	return retval;
}

VkPipelineLayout APIState::createPipelineLayout(const VkPipelineLayoutCreateInfo& info) const noexcept
{
	VkPipelineLayout retval = VK_NULL_HANDLE;
	BreakIfFailed(vkCreatePipelineLayout(m_device, &info, m_alloc, &retval));
	return retval;
}

VkDescriptorPool APIState::createDescriptorPool(const VkDescriptorPoolCreateInfo& info) const noexcept
{
	VkDescriptorPool retval = VK_NULL_HANDLE;
	BreakIfFailed(vkCreateDescriptorPool(m_device, &info, m_alloc, &retval));
	return retval;
}

VkDescriptorSetLayout APIState::createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) const noexcept
{
	VkDescriptorSetLayout retval = VK_NULL_HANDLE;
	BreakIfFailed(vkCreateDescriptorSetLayout(m_device, &info, m_alloc, &retval));
	return retval;
}

std::vector<VkDescriptorSet> APIState::allocateDescriptorSets(const VkDescriptorSetAllocateInfo& info) const noexcept
{
	std::vector<VkDescriptorSet> retval(info.descriptorSetCount);
	BreakIfFailed(vkAllocateDescriptorSets(m_device, &info, retval.data()));
	return retval;
}

const char* APIClient::toString(VkResult value)
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
#ifdef VK_ENABLE_BETA_EXTENSIONS
	case VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR:
		return "VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR";
	case VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR:
		return "VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR";
#endif
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

}
