#pragma once

#include "VulkanApi.hpp"

namespace vulkan
{

struct MemoryFlags
{
	static const MemoryFlags kDeviceOnly;
	static const MemoryFlags kHostWriteable;
	static const MemoryFlags kMaybeBARMemory;
	static const MemoryFlags kBARMemory;
	VkMemoryPropertyFlags include = 0;
	VkMemoryPropertyFlags exclude = 0;
	VkMemoryPropertyFlags optional = 0;
};

class Buffer
{
public:
	VkBuffer handle() const { return m_handle; };
	VkDeviceAddress gpuAddress() const { return m_address; };
	bool valid() const { return m_handle != VK_NULL_HANDLE; }
	void initialize(const APIState& vk, const VkBufferCreateInfo& bci, const MemoryFlags& flags);
	void finalize(const APIState& vk);
private:
	friend struct Resources;
	VkBuffer m_handle = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	VkDeviceAddress m_address = 0;
};

class Image
{
public:
	VkImage handle() const { return m_handle; };
	bool valid() const { return m_handle != VK_NULL_HANDLE; }
	void initialize(const APIState& vk, const VkImageCreateInfo& bci, const MemoryFlags& flags);
	void finalize(const APIState& vk);
private:
	VkImage m_handle = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
};

}
