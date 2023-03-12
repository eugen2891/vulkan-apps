#include "Resources.hpp"

#include "InitHelpers.hpp"


namespace vulkan
{

const MemoryFlags MemoryFlags::kDeviceOnly{ VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT };
const MemoryFlags MemoryFlags::kHostWriteable{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
const MemoryFlags MemoryFlags::kMaybeBARMemory{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };
const MemoryFlags MemoryFlags::kBARMemory{ VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT };

void Buffer::initialize(const APIState& vk, const VkBufferCreateInfo& bci, const MemoryFlags& flags)
{
	vulkan::PhysicalDeviceMemoryProperties memProps(vk.physicalDevice());
	BreakIfFailed(vkCreateBuffer(vk.device(), &bci, vk.alloc(), &m_handle));
	VkMemoryRequirements bufferMemReq;
	vkGetBufferMemoryRequirements(vk.device(), m_handle, &bufferMemReq);
	VkMemoryAllocateFlagsInfo mafi{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO, nullptr, VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT };
	vulkan::MemoryAllocateInfo bmai{ bufferMemReq.size, memProps.findMemoryType(bufferMemReq, flags.include, flags.exclude, flags.optional) };
	if (bci.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) bmai.pNext = &mafi;
	BreakIfFailed(vkAllocateMemory(vk.device(), &bmai, vk.alloc(), &m_memory));
	BreakIfFailed(vkBindBufferMemory(vk.device(), m_handle, m_memory, 0));
	if (bci.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		VkBufferDeviceAddressInfo bdai{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, nullptr, m_handle };
		m_address = vkGetBufferDeviceAddress(vk.device(), &bdai);
	}	
}

void Buffer::finalize(const APIState& vk)
{
	vkDestroyBuffer(vk.device(), m_handle, vk.alloc());
	vkFreeMemory(vk.device(), m_memory, vk.alloc());
}

void Image::initialize(const APIState& vk, const VkImageCreateInfo& ici, const MemoryFlags& flags)
{
	vulkan::PhysicalDeviceMemoryProperties memProps(vk.physicalDevice());
	BreakIfFailed(vkCreateImage(vk.device(), &ici, vk.alloc(), &m_handle));
	VkMemoryRequirements imageMemReq;
	vkGetImageMemoryRequirements(vk.device(), m_handle, &imageMemReq);
	vulkan::MemoryAllocateInfo imai{ imageMemReq.size, memProps.findMemoryType(imageMemReq, flags.include, flags.exclude, flags.optional) };
	BreakIfFailed(vkAllocateMemory(vk.device(), &imai, vk.alloc(), &m_memory));
	BreakIfFailed(vkBindImageMemory(vk.device(), m_handle, m_memory, 0));
}

void Image::finalize(const APIState& vk)
{
	vkDestroyImage(vk.device(), m_handle, vk.alloc());
	vkFreeMemory(vk.device(), m_memory, vk.alloc());
}

}
