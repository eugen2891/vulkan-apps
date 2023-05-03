#include "BindingTable.hpp"

namespace vulkan
{

static void IncreasePoolSize(std::vector<VkDescriptorPoolSize>& sizes, VkDescriptorType type, uint32_t count)
{
	for (auto& size : sizes)
	{
		if (size.type == type)
		{
			size.descriptorCount += count;
			return;
		}
	}
	VkDescriptorPoolSize size{ type, count };
	sizes.push_back(size);
}

BindingTableUpdate& BindingTableUpdate::uniformBuffer(VkDescriptorSet set, uint32_t binding, const VkDescriptorBufferInfo* info) noexcept
{
	VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0u, 1u, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, nullptr, info, nullptr };
	m_writes.push_back(write);
	return *this;
}

BindingTableUpdate& BindingTableUpdate::combinedImageSampler(VkDescriptorSet set, uint32_t binding, const VkDescriptorImageInfo* info) noexcept
{
	VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0u, 1u, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, info, nullptr, nullptr };
	m_writes.push_back(write);
	return *this;
}

vulkan::BindingTableUpdate& BindingTableUpdate::storageImage(VkDescriptorSet set, uint32_t binding, const VkDescriptorImageInfo* info) noexcept
{
	VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET, nullptr, set, binding, 0u, 1u, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, info, nullptr, nullptr };
	m_writes.push_back(write);
	return *this;
}

}

void vulkan::BindingTableLayout::initialize(std::initializer_list<BindingDescList> sets, std::initializer_list<VkPushConstantRange> push, uint32_t maxTables)
{
	ReturnIfNot(m_layout == VK_NULL_HANDLE);	
	std::vector<VkDescriptorPoolSize> sizes;

	uint32_t numSets = 0;
	m_sets.reserve(sets.size());
	for (auto const& set : sets)
	{
		m_sets.emplace_back(m_vk.createDescriptorSetLayout({ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, uint32_t(set.size()), set.begin() }));
		for (auto const& binding : set)
		{
			IncreasePoolSize(sizes, binding.descriptorType, binding.descriptorCount * maxTables);
			++m_maxWrites;
		}
	}

	m_layout = m_vk.createPipelineLayout({ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, uint32_t(m_sets.size()), m_sets.data(), uint32_t(push.size()), push.begin() });
	m_pool = m_vk.createDescriptorPool({ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, uint32_t(sets.size() * maxTables), uint32_t(sizes.size()), sizes.data() });
}

VkDescriptorSet vulkan::BindingTableLayout::allocateDescriptorSet(uint32_t layoutIndex) const noexcept
{
	return m_vk.allocateDescriptorSets({ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, m_pool, 1, &m_sets[layoutIndex]}).front();
}

void vulkan::BindingTableLayout::createBindingTable(BindingTable& table)
{
	table.m_sets = m_vk.allocateDescriptorSets({ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, m_pool, uint32_t(m_sets.size()), m_sets.data() });
	table.m_maxWrites = m_maxWrites;
	table.m_layout = m_layout;
}

void vulkan::BindingTableLayout::finalize()
{
	vkDestroyPipelineLayout(m_vk.device(), m_layout, m_vk.alloc());
	vkDestroyDescriptorPool(m_vk.device(), m_pool, m_vk.alloc());
	for (auto const& set : m_sets) vkDestroyDescriptorSetLayout(m_vk.device(), set, m_vk.alloc());
}

vulkan::BindingTable& vulkan::BindingTable::bindTexture(uint32_t set, uint32_t binding, const VkDescriptorImageInfo& imageInfo)
{
	RetvalIfNot(set < m_sets.size(), *this);
	VkWriteDescriptorSet& write = m_writes.emplace_back();
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.pNext = nullptr;
	write.dstSet = m_sets[set];
	write.dstBinding = binding;
	write.dstArrayElement = 0;
	write.descriptorCount = 1;
	write.descriptorType = imageInfo.sampler == VK_NULL_HANDLE ? VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE : VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write.pImageInfo = &imageInfo;
	write.pBufferInfo = nullptr;
	write.pTexelBufferView = nullptr;
	return *this;
}

void vulkan::BindingTable::bindForGraphics(VkCommandBuffer commandBuffer) const
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 0, uint32_t(m_sets.size()), m_sets.data(), 0, nullptr);
}

void vulkan::BindingTable::useForCompute(VkCommandBuffer commandBuffer) const
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_layout, 0, uint32_t(m_sets.size()), m_sets.data(), 0, nullptr);
}

void vulkan::BindingTable::applyPendingUpdates(VkDevice device)
{
	if (!m_writes.empty())
	{
		vkUpdateDescriptorSets(device, uint32_t(m_writes.size()), m_writes.data(), 0, nullptr);
		m_writes.clear();
	}
}

VkPipelineLayout vulkan::BindingTable::layout() const
{
	return m_layout;
}
