#pragma once

#include "VulkanApi.hpp"

#include "../Utilities/Array.hpp"

namespace vulkan
{

struct PhysicalDevicesList : public Array<VkPhysicalDevice>
{
	explicit PhysicalDevicesList(VkInstance instance);
};

struct QueueFamiliesList : public Array<VkQueueFamilyProperties>
{
	explicit QueueFamiliesList(VkPhysicalDevice physicalDevice);
	uint32_t findByFlags(VkQueueFlags incl, VkQueueFlags excl = 0) const;
};

struct DeviceQueueCreateList : public Array<VkDeviceQueueCreateInfo>
{
	explicit DeviceQueueCreateList(uint32_t count);
};

struct SurfaceFormatsList : public Array<VkSurfaceFormatKHR>
{
	explicit SurfaceFormatsList(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};

struct PresentModesList : public Array<VkPresentModeKHR>
{
	explicit PresentModesList(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};

struct SwapchainImagesList : public Array<VkImage>
{
	explicit SwapchainImagesList(VkDevice device, VkSwapchainKHR swapchain);
};

struct PhysicalDeviceMemoryProperties : VkPhysicalDeviceMemoryProperties
{
	explicit PhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice);
	uint32_t findMemoryType(const VkMemoryRequirements& reqs, VkMemoryPropertyFlags flags, VkMemoryPropertyFlags exclude = 0, VkMemoryPropertyFlags maybe = 0) const;
};

struct BufferCreateInfo : VkBufferCreateInfo
{
	explicit BufferCreateInfo(VkDeviceSize size, VkBufferUsageFlags usage) : VkBufferCreateInfo{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, nullptr, 0, size, usage } {}
};

struct Image1DCreateInfo : VkImageCreateInfo
{
	explicit Image1DCreateInfo(VkFormat format, uint32_t extent, uint32_t mips, uint32_t faces, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage)
		: VkImageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_1D, format, { extent, 1, 1 }, mips, faces, samples, tiling, usage } {}
};

struct Image2DCreateInfo : VkImageCreateInfo
{
	explicit Image2DCreateInfo(VkFormat format, const VkExtent2D& extent, uint32_t mips, uint32_t faces, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage)
		: VkImageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_2D, format, { extent.width, extent.height, 1 }, mips, faces, samples, tiling, usage } {}
};

struct Image3DCreateInfo : VkImageCreateInfo
{
	explicit Image3DCreateInfo(VkFormat format, const VkExtent3D& extent, uint32_t mips, uint32_t faces, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage)
		: VkImageCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, nullptr, 0, VK_IMAGE_TYPE_3D, format, extent, mips, faces, samples, tiling, usage } {}
};

struct Image2DViewCreateInfo : VkImageViewCreateInfo
{
	explicit Image2DViewCreateInfo(VkImage image, VkFormat format, const VkComponentMapping& components, const VkImageSubresourceRange& range)
		: VkImageViewCreateInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, nullptr, 0, image, VK_IMAGE_VIEW_TYPE_2D, format, components, range } {}
};

struct MemoryAllocateInfo : VkMemoryAllocateInfo
{
	template <typename... Args> explicit MemoryAllocateInfo(Args... args) : VkMemoryAllocateInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO, nullptr, args... } {}
};

struct MappedMemoryRange : VkMappedMemoryRange
{
	template <typename... Args> explicit MappedMemoryRange(Args... args) : VkMappedMemoryRange{ VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE, nullptr, args... } {}
};

struct ShaderModuleCreateInfo : VkShaderModuleCreateInfo
{
	template <typename... Args> explicit ShaderModuleCreateInfo(Args... args) : VkShaderModuleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, args... } {}
};

class ClearValue
{
public:
	explicit ClearValue() : ClearValue(0.f, 0.f, 0.f, 0.f) {}
	explicit ClearValue(float d, uint8_t s = 0) { m_value.depthStencil.depth = d; m_value.depthStencil.stencil = s; }
	explicit ClearValue(float r, float g, float b, float a) { m_value.color.float32[0] = r; m_value.color.float32[1] = g; m_value.color.float32[2] = b; m_value.color.float32[3] = a; }
	operator const VkClearValue& () const { return m_value; }
private:
	VkClearValue m_value;
};

}
