#pragma once

#include "VulkanApi.hpp"

#include "../Utilities/Array.hpp"

namespace vulkan
{

class BindingTable;

class BindingTableLayout : public APIClient
{
public:
	using APIClient::APIClient;
	using BindingDescList = std::initializer_list <VkDescriptorSetLayoutBinding>;
	void initialize(std::initializer_list<BindingDescList> sets, std::initializer_list<VkPushConstantRange> push = {}, uint32_t maxTables = 1);
	VkDescriptorSet allocateDescriptorSet(uint32_t layoutIndex = 0) const noexcept;
	VkPipelineLayout layout() const noexcept { return m_layout; };
	void createBindingTable(BindingTable& table);
	void finalize();
private:	
	VkPipelineLayout m_layout = VK_NULL_HANDLE;
	VkDescriptorPool m_pool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSetLayout> m_sets;
	uint32_t m_maxWrites = 0;
};

class BindingTableUpdate
{
public:
	BindingTableUpdate& uniformBuffer(VkDescriptorSet set, uint32_t binding, const VkDescriptorBufferInfo* info) noexcept;
	BindingTableUpdate& combinedImageSampler(VkDescriptorSet set, uint32_t binding, const VkDescriptorImageInfo* info) noexcept;
	BindingTableUpdate& storageImage(VkDescriptorSet set, uint32_t binding, const VkDescriptorImageInfo* info) noexcept;
	Range<VkWriteDescriptorSet> writes() noexcept { return { m_writes.data(), m_writes.size() }; }
	Range<VkCopyDescriptorSet> copies() noexcept { return { nullptr,0 }; }
private:
	std::vector<VkDescriptorImageInfo> m_images;
	std::vector<VkWriteDescriptorSet> m_writes;
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
	std::vector<VkWriteDescriptorSet> m_writes;
	std::vector<VkDescriptorSet> m_sets;
	uint32_t m_maxWrites;
};

}
