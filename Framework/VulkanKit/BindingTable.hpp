#pragma once

#include "VulkanApi.hpp"

#include "../Utilities/Array.hpp"

#include <initializer_list>

namespace vulkan
{

class BindingTable;

class BindingTableLayout : public APIClient
{
public:
	using APIClient::APIClient;
	using BindingDescList = std::initializer_list <VkDescriptorSetLayoutBinding>;
	void initialize(std::initializer_list<BindingDescList> sets, std::initializer_list<VkPushConstantRange> push = {}, uint32_t maxTables = 1);
	void createBindingTable(BindingTable& table);
	void finalize();
private:	
	VkPipelineLayout m_layout = VK_NULL_HANDLE;
	VkDescriptorPool m_pool = VK_NULL_HANDLE;
	Array<VkDescriptorSetLayout> m_sets;
	uint32_t m_maxWrites = 0;
};

class BindingTable
{
public:
	BindingTable& bindTexture(uint32_t set, uint32_t binding, const VkDescriptorImageInfo& imageInfo);
	void bindForGraphics(VkCommandBuffer commandBuffer) const;
	void useForCompute(VkCommandBuffer commandBuffer) const;
	void applyPendingUpdates(VkDevice device);
	VkPipelineLayout layout() const;
private:
	friend class BindingTableLayout;
	VkPipelineLayout m_layout = VK_NULL_HANDLE;
	Array<VkWriteDescriptorSet> m_writes;
	Array<VkDescriptorSet> m_sets;
	uint32_t m_maxWrites;
};

}
