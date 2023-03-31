#pragma once

#include "InitHelpers.hpp"

#define VULKAN_APPLICATION_INSTANCE(APP) \
namespace vulkan { Application& GetApplication() { \
static APP instance; return instance; } }

namespace vulkan
{

class Window;

class FeatureSet
{
public:
	VkPhysicalDeviceFeatures& v10() { return m_vk10.features; }
	VkPhysicalDeviceVulkan11Features& v11() { return m_vk11; }
	VkPhysicalDeviceVulkan12Features& v12() { return m_vk12; }
	VkPhysicalDeviceVulkan13Features& v13() { return m_vk13; }
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR& rt() { return m_rt; }
	VkPhysicalDeviceAccelerationStructureFeaturesKHR& as() { return m_as; }
private:
	VkPhysicalDeviceFeatures2 m_vk10{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &m_vk11 };
	VkPhysicalDeviceVulkan11Features m_vk11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, &m_vk12 };
	VkPhysicalDeviceVulkan12Features m_vk12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &m_vk13 };
	VkPhysicalDeviceVulkan13Features m_vk13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, &m_rt };
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rt{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, &m_as };
	VkPhysicalDeviceAccelerationStructureFeaturesKHR m_as{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
	friend class Application;
};

class Application : public APIState
{
public:
	void main();
protected:
	struct QueueRequest
	{
		VkQueueFlags include;
		VkQueueFlags exclude;
	};
	virtual void initialize();
	virtual void finalize();
	virtual void runApplication() = 0;
	void setOutputWindow(Window* wndPtr);
	virtual void requestFeatures(FeatureSet& features) const;
	virtual std::vector<QueueRequest> requestQueues() const;
	virtual const char* applicationName() const;
	virtual const char* engineName() const;
	explicit Application() = default;
	virtual ~Application() = default;
private:	
	bool canPresent(VkPhysicalDevice physicalDevice, uint32_t family) const;
	std::vector<VkDeviceQueueCreateInfo> detectQueues(VkPhysicalDevice physicalDevice, uint32_t& presentQueue);
	void initializeInternal();
	void finalizeInternal();
	Window* m_wndPtr = nullptr;
	std::vector<const char*> m_instanceExt;
	std::vector<const char*> m_deviceExt;
};

}
