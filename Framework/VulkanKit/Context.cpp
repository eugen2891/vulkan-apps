#include "Context.hpp"

void vulkan::Context::initialize(uint32_t numBuffers, uint32_t queueFamily)
{
	m_fences.resize(numBuffers, VK_NULL_HANDLE);
	m_semaphores.resize(numBuffers, VK_NULL_HANDLE);
	m_commandBuffers.resize(numBuffers, VK_NULL_HANDLE);
	VkCommandPoolCreateInfo cpci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, queueFamily };
	ReturnIfFailed(vkCreateCommandPool(m_vk.device(), &cpci, m_vk.alloc(), &m_commandPool));
	VkCommandBufferAllocateInfo cbai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, numBuffers };
	ReturnIfFailed(vkAllocateCommandBuffers(m_vk.device(), &cbai, m_commandBuffers.data()));
	for (uint32_t i = 0; i < numBuffers; i++)
	{
		VkSemaphoreCreateInfo sci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
		ReturnIfFailed(vkCreateSemaphore(m_vk.device(), &sci, m_vk.alloc(), &m_semaphores[i]));
		VkFenceCreateInfo fci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };
		ReturnIfFailed(vkCreateFence(m_vk.device(), &fci, m_vk.alloc(), &m_fences[i]));
	}
	m_submitQueue = m_vk.getDeviceQueue(queueFamily);
	BreakIfNot(m_submitQueue != VK_NULL_HANDLE);
}

vulkan::PipelineBarrier& vulkan::Context::pipelineBarrier(bool reset)
{
	if (reset) m_pipelineBarrier.reset();	
	return m_pipelineBarrier;
}

VkSemaphore vulkan::Context::submit(const Range<VkSemaphore>& waitFor, const VkPipelineStageFlags* waitStage)
{
	m_vk.resetFence(m_fences[m_current]);
	VkSemaphore sem = m_semaphores[m_current];
	VkSubmitInfo si { VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, uint32_t(waitFor.num()), waitFor.get(), waitStage, 1,& m_commandBuffers[m_current], 1,& sem };
	m_vk.queueSubmit(m_submitQueue, si, m_fences[m_current]);
	m_current = (m_current + 1) % m_commandBuffers.size();
	return sem;
}

VkCommandBuffer vulkan::Context::commandBuffer() const
{
	return m_commandBuffers[m_current];
}

void vulkan::Context::beginCommandBuffer()
{
	m_vk.waitForFence(m_fences[m_current]);
	BreakIfFailed(vkResetCommandBuffer(m_commandBuffers[m_current], 0));
	VkCommandBufferBeginInfo cbbi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, nullptr, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };
	vkBeginCommandBuffer(m_commandBuffers[m_current], &cbbi);
}

void vulkan::Context::endCommandBuffer()
{
	vkEndCommandBuffer(m_commandBuffers[m_current]);
}

void vulkan::Context::finalize()
{
	for (auto val : m_fences) m_vk.destroy(val);
	for (auto val : m_semaphores) m_vk.destroy(val);
	m_vk.freeCommandBuffers(m_commandPool, m_commandBuffers);
	m_vk.destroy(m_commandPool);
}
