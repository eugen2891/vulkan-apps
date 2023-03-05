#include "Context.hpp"

#include <cstring>

void vulkan::Context::initialize(const Config& conf)
{
	VkCommandPoolCreateInfo cpci{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO, nullptr, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT, conf.queueFamily };
	ReturnIfFailed(vkCreateCommandPool(conf.device, &cpci, conf.alloc, &m_commandPool));
	m_commandBuffers = Array<VkCommandBuffer>::New(conf.numBuffers);
	VkCommandBufferAllocateInfo cbai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, nullptr, m_commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, conf.numBuffers };
	ReturnIfFailed(vkAllocateCommandBuffers(conf.device, &cbai, m_commandBuffers.items));
	m_semaphores = Array<VkSemaphore>::New(conf.numBuffers);
	m_fences = Array<VkFence>::New(conf.numBuffers);
	for (uint32_t i = 0; i < conf.numBuffers; i++)
	{
		VkSemaphoreCreateInfo sci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
		ReturnIfFailed(vkCreateSemaphore(conf.device, &sci, conf.alloc, &m_semaphores[i]));
		VkFenceCreateInfo fci{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT };
		ReturnIfFailed(vkCreateFence(conf.device, &fci, conf.alloc, &m_fences[i]));
	}
	m_submitQueue = conf.submitQueue;
	m_device = conf.device;
	m_alloc = conf.alloc;
}

vulkan::PipelineBarrier& vulkan::Context::pipelineBarrier(bool reset)
{
	if (reset) m_pipelineBarrier.reset();	
	return m_pipelineBarrier;
}

VkSemaphore vulkan::Context::submit(const ArrayRef<VkSemaphore>& waitFor, const VkPipelineStageFlags* waitStage)
{
	VkSemaphore sem = m_semaphores[m_current];
	VkSubmitInfo si
	{
		VK_STRUCTURE_TYPE_SUBMIT_INFO, nullptr, waitFor.num, waitFor.items, waitStage, 1, &m_commandBuffers[m_current], 1, &sem
	};
	BreakIfFailed(vkResetFences(m_device, 1, &m_fences[m_current]));
	BreakIfFailed(vkQueueSubmit(m_submitQueue, 1, &si, m_fences[m_current]));
	m_current = (m_current + 1) % m_commandBuffers.num;
	return sem;
}

VkCommandBuffer vulkan::Context::commandBuffer() const
{
	return m_commandBuffers[m_current];
}

void vulkan::Context::beginCommandBuffer()
{
	if (vkGetFenceStatus(m_device, m_fences[m_current]) != VK_SUCCESS)
	{
		BreakIfFailed(vkWaitForFences(m_device, 1, &m_fences[m_current], VK_TRUE, UINT64_MAX));
	}
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
	for (uint32_t i = 0; i < m_fences.num; i++) vkDestroyFence(m_device, m_fences[i], m_alloc);
	for (uint32_t i = 0; i < m_semaphores.num; i++) vkDestroySemaphore(m_device, m_semaphores[i], m_alloc);
	vkFreeCommandBuffers(m_device, m_commandPool, m_commandBuffers.num, m_commandBuffers.items);
	vkDestroyCommandPool(m_device, m_commandPool, m_alloc);
}
