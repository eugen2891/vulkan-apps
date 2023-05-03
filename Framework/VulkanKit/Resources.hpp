#pragma once

namespace vulkan
{

class APIState;

enum ResourceType
{
	eResourceType_Invalid = -1,
	eResourceType_Buffer,
	eResourceType_Image
};

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

//Sampler::kMinMagMipLinear

struct SamplerFiltering
{
	static const SamplerFiltering kMinMagMipNearest;
	static const SamplerFiltering kMinMagMipLinear;
	VkFilter magFilter = VK_FILTER_NEAREST;
	VkFilter minFilter = VK_FILTER_NEAREST;
	VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
};

struct SamplerAddressing
{
	static const SamplerAddressing kWrap;
	static const SamplerAddressing kBorder;
	VkSamplerAddressMode u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	VkSamplerAddressMode w = VK_SAMPLER_ADDRESS_MODE_REPEAT;
};

class Resource
{
public:
	Resource(APIState& vk, ResourceType type) noexcept;
	VkDeviceAddress bufferAddress() const noexcept;
	VkImageView imageView() const noexcept;
	operator VkBuffer() const noexcept;
	operator VkImage() const noexcept;
	bool valid() const noexcept;
	void finalize();
private:
	void initializeAsBuffer(const VkBufferCreateInfo& bci, const MemoryFlags& flags);
	void initializeAsImage(const VkImageCreateInfo& ici, const MemoryFlags& flags);
	APIState& m_vk;
	friend class APIState;
	const ResourceType m_type;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	union ResourceData
	{
		struct BufferData { VkBuffer handle; VkDeviceAddress address; } buffer;
		struct ImageData { VkImage handle; VkImageView view; } image;
	} m_data{};
};

}
