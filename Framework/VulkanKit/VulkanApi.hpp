#pragma once

#include "Resources.hpp"

#include "Utilities/Array.hpp"

#define ReturnIfFailed(expr) ReturnIfNot((expr) == VK_SUCCESS)
#define RetvalIfFailed(expr, val) RetvalIfNot((expr) == VK_SUCCESS, val)
#define BreakIfFailed(expr) BreakIfNot((expr) == VK_SUCCESS)

namespace vulkan
{

class APIState
{
public:

	VkDevice device() const { return m_device; }
	VkInstance instance() const { return m_instance; }	
	VkAllocationCallbacks* alloc() const { return m_alloc; }	
	VkPhysicalDevice physicalDevice() const { return m_physicalDevice; }

	std::vector<VkPhysicalDevice> enumPhysicalDevices() const noexcept;
	VkQueue getDeviceQueue(uint32_t family, uint32_t index = 0) const noexcept;
	std::vector<VkImage> enumSwapchainImages(VkSwapchainKHR swapchain) const noexcept;
	std::vector<VkPresentModeKHR> enumPresentModes(VkSurfaceKHR surface) const noexcept;
	std::vector<VkSurfaceFormatKHR> enumSurfaceFormats(VkSurfaceKHR surface) const noexcept;

	Resource* createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, MemoryFlags memoryFlags) noexcept;
	Resource* createImage2D(VkFormat format, const VkExtent2D& extent, uint32_t mips, VkSampleCountFlagBits samples, VkImageTiling tiling, VkImageUsageFlags usage, MemoryFlags memoryFlags) noexcept;
	VkSampler createSampler(const SamplerFiltering& filter, const SamplerAddressing& address) const noexcept;
	VkPipeline createComputePipeline(const VkComputePipelineCreateInfo& info) const noexcept;

	VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo& info) const noexcept;
	VkDescriptorPool createDescriptorPool(const VkDescriptorPoolCreateInfo& info) const noexcept;
	VkDescriptorSetLayout createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) const noexcept;
	std::vector<VkDescriptorSet> allocateDescriptorSets(const VkDescriptorSetAllocateInfo& info) const noexcept;
	void updateDescriptorSets(const Range<VkWriteDescriptorSet>& writes, const Range<VkCopyDescriptorSet>& copies) const noexcept;

	void queueSubmit(VkQueue queue, const VkSubmitInfo& info, VkFence fence = VK_NULL_HANDLE) const noexcept { BreakIfFailed(vkQueueSubmit(queue, 1, &info, fence)); }
	void waitForFence(VkFence fence) const noexcept { if (getFenceStatus(fence) != VK_SUCCESS) BreakIfFailed(vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX)); }
	void resetFence(VkFence fence) const noexcept { BreakIfFailed(vkResetFences(m_device, 1, &fence)); }	
	VkResult getFenceStatus(VkFence fence) const noexcept { return vkGetFenceStatus(m_device, fence); }

	void freeCommandBuffers(VkCommandPool pool, const std::vector<VkCommandBuffer>& items)  const noexcept { vkFreeCommandBuffers(m_device, pool, uint32_t(items.size()), items.data()); }
	void freeMemory(VkDeviceMemory val) const noexcept { vkFreeMemory(m_device, val, m_alloc); };

	void destroy(VkCommandPool val) const noexcept { vkDestroyCommandPool(m_device, val, m_alloc); }
	void destroy(VkSemaphore val) const noexcept { vkDestroySemaphore(m_device, val, m_alloc); }
	void destroy(VkFence val) const noexcept { vkDestroyFence(m_device, val, m_alloc); }
	void destroy(VkImage val) const noexcept { vkDestroyImage(m_device, val, m_alloc); }
	void destroy(VkImageView val) const noexcept { vkDestroyImageView(m_device, val, m_alloc); }
	void destroy(VkBuffer val) const noexcept { vkDestroyBuffer(m_device, val, m_alloc); }

protected:
	VkInstance m_instance = VK_NULL_HANDLE;
	VkAllocationCallbacks* m_alloc = nullptr;
	VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
	VkDevice m_device = VK_NULL_HANDLE;
	std::deque<Resource> m_resources;
};

class CommandContext
{
public:

	explicit CommandContext(VkCommandBuffer cb) noexcept : m_cb(cb) {}
	
	void clearColorImage(VkImage img, VkImageLayout layout, const VkClearColorValue* color, const Range<VkImageSubresourceRange>& ranges) const noexcept
	{
		vkCmdClearColorImage(m_cb, img, layout, color, uint32_t(ranges.num()), ranges.get());
	}
	void bindGraphicsDescriptors(VkPipelineLayout layout, uint32_t firstSet, const Range<VkDescriptorSet>& sets, const Range<uint32_t>& offsets) const noexcept
	{
		vkCmdBindDescriptorSets(m_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, firstSet, uint32_t(sets.num()), sets.get(), uint32_t(offsets.num()), offsets.get());
	}
	void bindComputeDescriptors(VkPipelineLayout layout, uint32_t firstSet, const Range<VkDescriptorSet>& sets, const Range<uint32_t>& offsets) const noexcept
	{
		vkCmdBindDescriptorSets(m_cb, VK_PIPELINE_BIND_POINT_COMPUTE, layout, firstSet, uint32_t(sets.num()), sets.get(), uint32_t(offsets.num()), offsets.get());
	}
	void bindGraphicsPipeline(VkPipeline pipeline) const noexcept
	{
		vkCmdBindPipeline(m_cb, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	}
	void bindComputePipeline(VkPipeline pipeline) const noexcept
	{
		vkCmdBindPipeline(m_cb, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
	}
	void updateBuffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize size, const void* data) const noexcept
	{
		vkCmdUpdateBuffer(m_cb, buffer, offset, size, data);
	}
	void pushConstants(VkPipelineLayout layout, VkShaderStageFlags stages, size_t offset, size_t size, const void* data) const noexcept
	{
		vkCmdPushConstants(m_cb, layout, stages, uint32_t(offset), uint32_t(size), data);
	}
	void blit(VkImage src, VkImageLayout srcLayout, VkImage dst, VkImageLayout dstLayout, const Range<VkImageBlit>& regions, VkFilter filter)
	{
		vkCmdBlitImage(m_cb, src, srcLayout, dst, dstLayout, uint32_t(regions.num()), regions.get(), filter);
	}
	void draw(uint32_t numVerts, uint32_t firstVert = 0, uint32_t numInst = 1, uint32_t firstInst = 0) const noexcept
	{
		vkCmdDraw(m_cb, numVerts, numInst, firstVert, firstInst);
	}
	void dispatch(const VkExtent3D& grid) const noexcept
	{
		vkCmdDispatch(m_cb, grid.width, grid.height, grid.depth);
	}


private:
	VkCommandBuffer m_cb;
};

class APIClient
{
public:
	static const char* toString(VkResult);
	explicit APIClient(APIState& vk) : m_vk(vk) {}
protected:
	APIState& m_vk;
};

}
