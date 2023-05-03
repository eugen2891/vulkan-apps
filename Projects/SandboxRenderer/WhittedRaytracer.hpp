#include <VulkanKit/VulkanApi.hpp>

namespace vulkan
{
class Resource;
}

class WhittedRaytracer : public vulkan::APIClient
{
public:
	using APIClient::APIClient;
	void buildAccelerationStructures(VkCommandBuffer commandBuffer);
	void setVertexData(vulkan::Resource* buffer, VkFormat format, VkDeviceSize stride) noexcept;
	void setIndexData(vulkan::Resource* buffer, VkIndexType indexType, VkDeviceSize stride) noexcept;
private:
	VkFormat m_vertexFormat = VK_FORMAT_UNDEFINED;
	VkIndexType m_indexType = VK_INDEX_TYPE_MAX_ENUM;
	VkDeviceAddress m_vbAddress = 0;
	VkDeviceAddress m_ibAddress = 0;
	VkDeviceSize m_vertexStride = 0;
	VkDeviceSize m_indexStride = 0;
};
