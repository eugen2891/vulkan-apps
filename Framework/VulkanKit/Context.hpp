#pragma once

#include "VulkanApi.hpp"
#include "PipelineBarrier.hpp"

#include "../Utilities/Array.hpp"

namespace vulkan
{

class Context : public APIClient
{
public:
	using APIClient::APIClient;
	void initialize(uint32_t numBuffers, uint32_t queueFamily = 0);
	PipelineBarrier& pipelineBarrier(bool reset = true);
	VkSemaphore submit(const Range<VkSemaphore>& waitFor, const VkPipelineStageFlags* waitStage);
	VkCommandBuffer commandBuffer() const;
	void beginCommandBuffer();
	void endCommandBuffer();
	void finalize();

private:
	PipelineBarrier m_pipelineBarrier;
	VkQueue m_submitQueue = VK_NULL_HANDLE;
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	std::vector<VkCommandBuffer> m_commandBuffers;
	std::vector<VkSemaphore> m_semaphores;
	std::vector<VkFence> m_fences;
	uint32_t m_current = 0;
};

}
