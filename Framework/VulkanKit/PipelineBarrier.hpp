#pragma once

#include "VulkanApi.hpp"

namespace vulkan
{

class PipelineBarrier
{
public:
	static const uint32_t kMaxImages = 8u;
	static const uint32_t kMaxBuffers = 8u;
	class Image
	{
	public:
		explicit Image(VkImageMemoryBarrier& imb);
		void layout(VkImageLayout oldLayout, VkAccessFlags oldAccess, VkImageLayout newLayout, VkAccessFlags newAccess);
	private:
		VkImageMemoryBarrier& m_imb;
	};
	class Buffer
	{
	public:
		explicit Buffer(VkBufferMemoryBarrier& bmb);
		void access(VkAccessFlags oldAccess, VkAccessFlags newAccess);
	private:
		VkBufferMemoryBarrier& m_bmb;
	};
	Image image(uint32_t index);
	Buffer buffer(uint32_t index);
	Image image(VkImage imageHandle, const VkImageSubresourceRange& subset);
	Buffer buffer(VkBuffer bufferHandle, VkDeviceSize offset = 0, VkDeviceSize size = VK_WHOLE_SIZE);
	void submit(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) const;
	void reset();
private:
	VkImageMemoryBarrier m_image[kMaxImages]{};
	VkBufferMemoryBarrier m_buffer[kMaxBuffers]{};
	VkMemoryBarrier m_memory;
	uint32_t m_numBuffers = 0;
	uint32_t m_numImages = 0;
};

}