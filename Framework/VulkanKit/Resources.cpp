#include "Resources.hpp"

#include "InitHelpers.hpp"

namespace vulkan
{

const MemoryFlags MemoryFlags::kDeviceOnly{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT };
const MemoryFlags MemoryFlags::kHostWriteable{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
const MemoryFlags MemoryFlags::kMaybeBARMemory{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
const MemoryFlags MemoryFlags::kBARMemory{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

const vulkan::SamplerFiltering SamplerFiltering::kMinMagMipNearest{};
const vulkan::SamplerFiltering SamplerFiltering::kMinMagMipLinear{ VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR };

const vulkan::SamplerAddressing SamplerAddressing::kWrap{};
const vulkan::SamplerAddressing SamplerAddressing::kBorder{ VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER };

Resource::Resource(APIState& vk, ResourceType type) noexcept
	: m_vk(vk), m_type(type)
{
	switch (type)
	{
	case eResourceType_Buffer:
		m_data.buffer = { VK_NULL_HANDLE, 0 };
		break;
	case eResourceType_Image:
		m_data.image = { VK_NULL_HANDLE, VK_NULL_HANDLE };
		break;
	default:
		BreakIfNot(0);
		break;
	}
}

void Resource::finalize()
{
	switch (m_type)
	{
	case eResourceType_Buffer:
		if (m_data.buffer.handle != VK_NULL_HANDLE) m_vk.destroy(m_data.buffer.handle);
		break;
	case eResourceType_Image:
		if (m_data.image.handle != VK_NULL_HANDLE) m_vk.destroy(m_data.image.handle);
		if (m_data.image.view != VK_NULL_HANDLE) m_vk.destroy(m_data.image.view);
		break;
	default:
		break;	
	}
	if (m_memory != VK_NULL_HANDLE) m_vk.freeMemory(m_memory);
}

void Resource::initializeAsBuffer(const VkBufferCreateInfo& bci, const MemoryFlags& flags)
{
	BreakIfNot(m_type == eResourceType_Buffer);
	vulkan::PhysicalDeviceMemoryProperties memProps(m_vk.physicalDevice());
	BreakIfFailed(vkCreateBuffer(m_vk.device(), &bci, m_vk.alloc(), &m_data.buffer.handle));

	VkMemoryRequirements bufferMemReq;
	vkGetBufferMemoryRequirements(m_vk.device(), m_data.buffer.handle, &bufferMemReq);

	VkMemoryAllocateFlagsInfo mafi{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, nullptr, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT };
	vulkan::MemoryAllocateInfo bmai{ bufferMemReq.size, memProps.findMemoryType(bufferMemReq, flags.include, flags.exclude, flags.optional) };

	if (bci.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) bmai.pNext = &mafi;
	BreakIfFailed(vkAllocateMemory(m_vk.device(), &bmai, m_vk.alloc(), &m_memory));
	BreakIfFailed(vkBindBufferMemory(m_vk.device(), m_data.buffer.handle, m_memory, 0));
	if (bci.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		VkBufferDeviceAddressInfo bdai{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, m_data.buffer.handle };
		m_data.buffer.address = vkGetBufferDeviceAddress(m_vk.device(), &bdai);
	}
}

void Resource::initializeAsImage(const VkImageCreateInfo& ici, const MemoryFlags& flags)
{
	vulkan::PhysicalDeviceMemoryProperties memProps(m_vk.physicalDevice());
	BreakIfFailed(vkCreateImage(m_vk.device(), &ici, m_vk.alloc(), &m_data.image.handle));
	VkMemoryRequirements imageMemReq;
	vkGetImageMemoryRequirements(m_vk.device(), m_data.image.handle, &imageMemReq);
	vulkan::MemoryAllocateInfo imai{ imageMemReq.size, memProps.findMemoryType(imageMemReq, flags.include, flags.exclude, flags.optional) };
	BreakIfFailed(vkAllocateMemory(m_vk.device(), &imai, m_vk.alloc(), &m_memory));
	BreakIfFailed(vkBindImageMemory(m_vk.device(), m_data.image.handle, m_memory, 0));
	vulkan::Image2DViewCreateInfo ivci{ m_data.image.handle, ici.format, {}, { VK_IMAGE_ASPECT_COLOR_BIT, 0, ici.mipLevels, 0, ici.arrayLayers } };
	switch (ici.format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_X8_D24_UNORM_PACK32:
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
	case VK_FORMAT_S8_UINT:
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
		break;
	};
	BreakIfFailed(vkCreateImageView(m_vk.device(), &ivci, m_vk.alloc(), &m_data.image.view));
}

VkDeviceAddress Resource::bufferAddress() const noexcept
{
	RetvalIfNot(m_type == eResourceType_Buffer, 0);
	return m_data.buffer.address;
}

VkImageView Resource::imageView() const noexcept
{
	RetvalIfNot(m_type == eResourceType_Image, VK_NULL_HANDLE);
	return m_data.image.view;
}

Resource::operator VkBuffer() const noexcept
{
	RetvalIfNot(m_type == eResourceType_Buffer, VK_NULL_HANDLE);
	return m_data.buffer.handle;
}

Resource::operator VkImage() const noexcept
{
	RetvalIfNot(m_type == eResourceType_Image, VK_NULL_HANDLE);
	return m_data.image.handle;
}

bool Resource::valid() const noexcept
{
	switch (m_type)
	{
	case eResourceType_Buffer: return (m_data.buffer.handle != VK_NULL_HANDLE);		
	case eResourceType_Image: return (m_data.image.handle != VK_NULL_HANDLE);
	default: return false;
	}
}

}
