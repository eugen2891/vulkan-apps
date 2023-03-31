#pragma once

#include "../Utilities/Debug.hpp"

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

	std::vector<VkImage> getSwapchainImages(VkSwapchainKHR swapchain) const noexcept;

	std::vector<VkPresentModeKHR> enumPresentModes(VkSurfaceKHR surface) const noexcept;

	std::vector<VkSurfaceFormatKHR> enumSurfaceFormats(VkSurfaceKHR surface) const noexcept;

	VkPipelineLayout createPipelineLayout(const VkPipelineLayoutCreateInfo& info) const noexcept;

	VkDescriptorPool createDescriptorPool(const VkDescriptorPoolCreateInfo& info) const noexcept;

	VkDescriptorSetLayout createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo& info) const noexcept;

	std::vector<VkDescriptorSet> allocateDescriptorSets(const VkDescriptorSetAllocateInfo& info) const noexcept;	

	void queueSubmit(VkQueue queue, const VkSubmitInfo& info, VkFence fence = VK_NULL_HANDLE) const noexcept { BreakIfFailed(vkQueueSubmit(queue, 1, &info, fence)); }

	void waitForFence(VkFence fence) const noexcept { if (fenceStatus(fence) != VK_SUCCESS) BreakIfFailed(vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX)); }

	void resetFence(VkFence fence) const noexcept { BreakIfFailed(vkResetFences(m_device, 1, &fence)); }
	
	VkResult fenceStatus(VkFence fence) const noexcept { return vkGetFenceStatus(m_device, fence); }

	void freeCommandBuffers(VkCommandPool pool, const std::vector<VkCommandBuffer>& items)  const noexcept { vkFreeCommandBuffers(m_device, pool, uint32_t(items.size()), items.data()); }

	void destroy(VkCommandPool val) const noexcept { vkDestroyCommandPool(m_device, val, m_alloc); }

	void destroy(VkSemaphore val) const noexcept { vkDestroySemaphore(m_device, val, m_alloc); }

	void destroy(VkFence val) const noexcept { vkDestroyFence(m_device, val, m_alloc); }	

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
	explicit APIClient(const APIState& vk) : m_vk(vk) {}
protected:
	const APIState& m_vk;
};

}
