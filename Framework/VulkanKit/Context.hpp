#pragma once

#include "VulkanApi.hpp"
#include "PipelineBarrier.hpp"

#include "../Utilities/Array.hpp"

namespace vulkan
{

class Context
{
public:
	struct Config
	{
		VkDevice device;
		VkAllocationCallbacks* alloc;
		VkQueue submitQueue;
		uint32_t queueFamily;
		uint32_t numBuffers;
	};	
	void initialize(const Config& conf);
	PipelineBarrier& pipelineBarrier(bool reset = true);
	VkSemaphore submit(const ArrayRef<VkSemaphore>& waitFor, const VkPipelineStageFlags* waitStage);
	VkCommandBuffer commandBuffer() const;
	void beginCommandBuffer();
	void endCommandBuffer();
	void finalize();
private:
	PipelineBarrier m_pipelineBarrier;
	VkDevice m_device = VK_NULL_HANDLE;
	VkQueue m_submitQueue = VK_NULL_HANDLE;
	VkAllocationCallbacks* m_alloc = nullptr;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	Array<VkCommandBuffer> m_commandBuffers;
	Array<VkSemaphore> m_semaphores;
	Array<VkFence> m_fences;
	uint32_t m_current = 0;
};

}
