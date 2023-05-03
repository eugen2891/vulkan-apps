#pragma once

enum ImageTarget
{
	eImageTarget_FTN,
	eImageTarget_Velocity,
	eImageTarget_EnumMax
};

namespace vulkan
{
class Context;
class APIState;
class Resource;
class BindingTableLayout;
}

class ImageTargets
{

public:

	void initialize(vulkan::APIState& vk, const VkRect2D& rect, vulkan::BindingTableLayout& bindings);

	void resetImages(vulkan::Context& vkCtx);

	void bindToContext(vulkan::Context& vkCtx, vulkan::BindingTableLayout& bindings);

	void finalize(vulkan::APIState& vk);

	void swap(uint32_t target);

private:

	vulkan::Resource* m_targets[eImageTarget_EnumMax];

	VkDescriptorSet m_descriptors = VK_NULL_HANDLE;

	uint32_t m_outputs = UINT32_MAX;


};
