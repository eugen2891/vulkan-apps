#include "BindingTable.hpp"

namespace vulkan
{

static void IncreasePoolSize(ArrayRef<VkDescriptorPoolSize>& sizes, VkDescriptorType type, uint32_t count)
{
	for (auto& size : sizes)
	{
		if (size.type == type)
		{
			size.descriptorCount += count;
			return;
		}
	}
	VkDescriptorPoolSize& size = sizes[sizes.num++];
	size.descriptorCount = count;
	size.type = type;
}

}

void vulkan::BindingTableLayout::initialize(std::initializer_list<BindingDescList> sets, std::initializer_list<VkPushConstantRange> push, uint32_t maxTables)
{
	ReturnIfNot(m_layout == VK_NULL_HANDLE);	
	m_sets = Array<VkDescriptorSetLayout>::New(uint32_t(sets.size()));

	uint32_t numSets = 0;
	VkDescriptorPoolSize sizes[32]{};
	static_assert(sizeof(sizes) <= 256);
	ArrayRef<VkDescriptorPoolSize> sizeRef{ sizes, 0 };
	for (auto const& set : sets)
	{
		VkDescriptorSetLayoutCreateInfo dslci{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO, nullptr, 0, uint32_t(set.size()), set.begin() };
		BreakIfFailed(vkCreateDescriptorSetLayout(m_vk.device(), &dslci, m_vk.alloc(), &m_sets[numSets++]));
		for (auto const& binding : set)
		{
			IncreasePoolSize(sizeRef, binding.descriptorType, binding.descriptorCount * maxTables);
			++m_maxWrites;
		}
	}

	VkDescriptorPoolCreateInfo dpci{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO, nullptr, 0, uint32_t(sets.size() * maxTables), sizeRef.num, sizes };
	BreakIfFailed(vkCreateDescriptorPool(m_vk.device(), &dpci, m_vk.alloc(), &m_pool));

	VkPipelineLayoutCreateInfo plci	{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, m_sets.num, m_sets.items, uint32_t(push.size()), push.begin() };
	BreakIfFailed(vkCreatePipelineLayout(m_vk.device(), &plci, m_vk.alloc(), &m_layout));	
}

void vulkan::BindingTableLayout::createBindingTable(BindingTable& table)
{
	table.m_sets = Array<VkDescriptorSet>::New(m_sets.num);
	VkDescriptorSetAllocateInfo dsai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO, nullptr, m_pool, m_sets.num, m_sets.items };
	BreakIfFailed(vkAllocateDescriptorSets(m_vk.device(), &dsai, table.m_sets.items));
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
	RetvalIfNot(set < m_sets.num, *this);
	if (!m_writes.items) 
	{
		m_writes = Array<VkWriteDescriptorSet>::New(m_maxWrites);
		m_writes.num = 0;
	}
	VkWriteDescriptorSet& write = m_writes[m_writes.num++];
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
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_layout, 0, m_sets.num, m_sets.items, 0, nullptr);
}

void vulkan::BindingTable::useForCompute(VkCommandBuffer commandBuffer) const
{
	vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_layout, 0, m_sets.num, m_sets.items, 0, nullptr);
}

void vulkan::BindingTable::applyPendingUpdates(VkDevice device)
{
	if (m_writes.num)
	{
		vkUpdateDescriptorSets(device, m_writes.num, m_writes.items, 0, nullptr);
		m_writes.clear();
	}
}

VkPipelineLayout vulkan::BindingTable::layout() const
{
	return m_layout;
}
