#include "PipelineBarrier.hpp"

vulkan::PipelineBarrier::Image::Image(VkImageMemoryBarrier& imb)
	: m_imb(imb)
{
}

void vulkan::PipelineBarrier::Image::layout(VkImageLayout oldLayout, VkAccessFlags oldAccess, VkImageLayout newLayout, VkAccessFlags newAccess)
{
	m_imb.oldLayout = oldLayout;
	m_imb.srcAccessMask = oldAccess;
	m_imb.newLayout = newLayout;
	m_imb.dstAccessMask = newAccess;
}

vulkan::PipelineBarrier::Image vulkan::PipelineBarrier::image(VkImage imageHandle, const VkImageSubresourceRange& subset)
{
	BreakIfNot(m_numImages < kMaxImages);
	VkImageMemoryBarrier& imb = m_numImages < kMaxImages ? m_image[m_numImages++] : m_image[0];
	imb.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imb.pNext = nullptr;
	imb.srcAccessMask = 0;
	imb.dstAccessMask = 0;
	imb.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imb.newLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imb.subresourceRange = subset;
	imb.image = imageHandle;
	return Image(imb);

}

vulkan::PipelineBarrier::Image vulkan::PipelineBarrier::image(uint32_t index)
{
	BreakIfNot(index < kMaxImages);
	VkImageMemoryBarrier& imb = index < kMaxImages ? m_image[index] : m_image[0];
	return Image(imb);
}

void vulkan::PipelineBarrier::submit(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) const
{
	vkCmdPipelineBarrier(commandBuffer, srcStage, dstStage, 0, 0, nullptr, m_numBuffers, m_buffer, m_numImages, m_image);
}

void vulkan::PipelineBarrier::reset()
{
	m_numBuffers = 0;
	m_numImages = 0;
}
