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

vulkan::PipelineBarrier::Buffer vulkan::PipelineBarrier::buffer(VkBuffer bufferHandle, VkDeviceSize offset, VkDeviceSize size)
{
	BreakIfNot(m_numBuffers < kMaxBuffers);
	VkBufferMemoryBarrier& bmb = m_numBuffers < kMaxBuffers ? m_buffer[m_numBuffers++] : m_buffer[0];
	bmb.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	bmb.pNext = nullptr;
	bmb.srcAccessMask = 0;
	bmb.dstAccessMask = 0;
	bmb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bmb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	bmb.buffer = bufferHandle;
	bmb.offset = offset;
	bmb.size = size;
	return Buffer(bmb);
}

vulkan::PipelineBarrier::Image vulkan::PipelineBarrier::image(uint32_t index)
{
	BreakIfNot(index < kMaxImages);
	VkImageMemoryBarrier& imb = index < kMaxImages ? m_image[index] : m_image[0];
	return Image(imb);
}

vulkan::PipelineBarrier::Buffer vulkan::PipelineBarrier::buffer(uint32_t index)
{
	BreakIfNot(index < kMaxBuffers);
	VkBufferMemoryBarrier& bmb = index < kMaxBuffers ? m_buffer[index] : m_buffer[0];
	return Buffer(bmb);
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

vulkan::PipelineBarrier::Buffer::Buffer(VkBufferMemoryBarrier& bmb)
	: m_bmb(bmb)
{
}

void vulkan::PipelineBarrier::Buffer::access(VkAccessFlags oldAccess, VkAccessFlags newAccess)
{
	m_bmb.srcAccessMask = oldAccess;
	m_bmb.dstAccessMask = newAccess;
}
