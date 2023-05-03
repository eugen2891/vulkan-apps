#include "ImageTargets.hpp"

#include <VulkanKit/VulkanApi.hpp>
#include <VulkanKit/PipelineBarrier.hpp>
#include <VulkanKit/BindingTable.hpp>
#include <VulkanKit/Context.hpp>

struct ImageTargetInfo
{
	VkFormat format;
	VkImageUsageFlags usage;
};

static const ImageTargetInfo kImageTargetInfo[]
{
	{ VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT },
	{ VK_FORMAT_R16G16_SFLOAT, VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT }
};

void ImageTargets::initialize(vulkan::APIState& vk, const VkRect2D& rect, vulkan::BindingTableLayout& bindings)
{
	vulkan::BindingTableUpdate update;
	VkDescriptorImageInfo dii[eImageTarget_EnumMax]{};
	m_descriptors = bindings.allocateDescriptorSet();
	for (uint32_t i = 0; i < eImageTarget_EnumMax; i++)
	{
		const auto& info = kImageTargetInfo[i];
		m_targets[i] = vk.createImage2D(info.format, rect.extent, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, info.usage, vulkan::MemoryFlags::kDeviceOnly);
		dii[i].imageView = m_targets[i]->imageView();
		dii[i].imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		update.storageImage(m_descriptors, i, &dii[i]);
	}
	vk.updateDescriptorSets(update.writes(), update.copies());
}

void ImageTargets::resetImages(vulkan::Context& vkCtx)
{
	vulkan::PipelineBarrier barrier;
	VkCommandBuffer cb = vkCtx.commandBuffer();

	for (vulkan::Resource* image : m_targets)
	{
		barrier.image(*image, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }).layout(VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_WRITE_BIT);
	}

	barrier.submit(cb, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
	barrier.reset();

	VkClearColorValue clearColor{};
	VkImageSubresourceRange clearRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	for (vulkan::Resource* image : m_targets)
	{
		barrier.image(*image, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 })
			.layout(VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_GENERAL, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
		vkCmdClearColorImage(cb, *image, VK_IMAGE_LAYOUT_GENERAL, &clearColor, 1, &clearRange);
	}
	barrier.submit(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void ImageTargets::bindToContext(vulkan::Context& vkCtx, vulkan::BindingTableLayout& bindings)
{
	vkCmdBindDescriptorSets(vkCtx.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, bindings.layout(), 0, 1, &m_descriptors, 0, nullptr);
	vkCmdBindDescriptorSets(vkCtx.commandBuffer(), VK_PIPELINE_BIND_POINT_COMPUTE, bindings.layout(), 0, 1, &m_descriptors, 0, nullptr);
}

void ImageTargets::finalize(vulkan::APIState& vk)
{
	for (auto target : m_targets)
	{
		target->finalize();
	}
}

void ImageTargets::swap(uint32_t target)
{
	m_outputs ^= (1u << target);
}
