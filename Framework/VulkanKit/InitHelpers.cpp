#include "InitHelpers.hpp"

vulkan::PhysicalDeviceMemoryProperties::PhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice)
{
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, this);
}

uint32_t vulkan::PhysicalDeviceMemoryProperties::findMemoryType(const VkMemoryRequirements& reqs, VkMemoryPropertyFlags flags, VkMemoryPropertyFlags exclude, VkMemoryPropertyFlags maybe) const
{
	uint32_t retval = memoryTypeCount;
	for (uint32_t i = 0; i < memoryTypeCount; i++)
	{
		if ((reqs.memoryTypeBits & (1 << i)) && (memoryTypes[i].propertyFlags & flags))
		{
			retval = i;
			break;
		}
	}
	return retval;
}
