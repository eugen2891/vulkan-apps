#include <GlobalPCH.hpp>

#include "Application.hpp"
#include "Window.hpp"

#include <Volk/volk.c>

namespace vulkan
{

void Application::main()
{
	initializeInternal();
	runApplication();
	finalizeInternal();
}

void Application::initialize()
{
}

void Application::finalize()
{
}

void Application::setOutputWindow(Window* wndPtr)
{
	m_wndPtr = wndPtr;
	m_instanceExt.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	m_instanceExt.push_back(VK_NATIVE_SURFACE_EXTENSION_NAME);
#if _DEBUG
	m_instanceExt.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
	m_deviceExt.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
}

void Application::requestFeatures(FeatureSet& features) const
{
}

std::vector<Application::QueueRequest> Application::requestQueues() const
{
	std::vector<QueueRequest> retval{ { VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT } };
	return retval;
}

const char* Application::applicationName() const
{
	return "ApplicationBase";
}

const char* Application::engineName() const
{
	return "VulkanHelper";
}

bool Application::canPresent(VkPhysicalDevice physicalDevice, uint32_t family) const
{
	if (m_wndPtr)
	{
		VkBool32 isSupported = VK_FALSE;
		RetvalIfFailed(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, family, m_wndPtr->surface(), &isSupported), false);
		return (isSupported == VK_TRUE);
	}
	return false;
}

std::vector<VkDeviceQueueCreateInfo> Application::detectQueues(VkPhysicalDevice physicalDevice, uint32_t& presentQueue)
{
	uint32_t numFamilies = 0;
	static const float priority = 1.f;
	std::vector<VkDeviceQueueCreateInfo> retval;
	std::vector<VkQueueFamilyProperties> properties;
	std::vector<QueueRequest> requests = requestQueues();
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numFamilies, nullptr);
	if (numFamilies > 0)
	{
		properties.resize(numFamilies);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numFamilies, properties.data());
		for (const auto& req : requests)
		{
			uint32_t index = numFamilies;
			for (uint32_t i = 0; i < numFamilies; i++)
			{
				const VkQueueFamilyProperties props = properties[i];
				if (props.queueCount > 0 && (props.queueFlags & req.include) == req.include && (props.queueFlags & req.exclude) == 0)
				{
					index = i;
					break;
				}
			}
			if (index == numFamilies)
			{
				retval.clear();
				presentQueue = UINT32_MAX;
				break;
			}
			else
			{
				VkDeviceQueueCreateInfo info{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, index, 1, &priority };
				if (m_wndPtr && presentQueue == UINT32_MAX && canPresent(physicalDevice, index)) presentQueue = index;
				retval.push_back(info);
			}
		}		
	}
	return retval;
}

void Application::initializeInternal()
{
	ReturnIfFailed(volkInitialize());

	const VkApplicationInfo ai
	{
		VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, applicationName(), 0u, engineName(), 0u, VK_API_VERSION_1_3
	};

	const VkInstanceCreateInfo ici
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &ai, 0, nullptr, uint32_t(m_instanceExt.size()), m_instanceExt.data()
	};

	ReturnIfFailed(vkCreateInstance(&ici, m_alloc, &m_instance));
	volkLoadInstanceOnly(m_instance);

	if (m_wndPtr) m_wndPtr->createWindowAndSurface();

	VkPhysicalDevice discreteGpu = VK_NULL_HANDLE;
	VkPhysicalDevice integratedGpu = VK_NULL_HANDLE;

	for (VkPhysicalDevice physicalDevice : enumPhysicalDevices())
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevice, &props);
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && !discreteGpu) discreteGpu = physicalDevice;
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && !integratedGpu) integratedGpu = physicalDevice;
	}

	uint32_t presentQueueFamily = UINT32_MAX;
	std::vector<VkDeviceQueueCreateInfo> queues;
	if (discreteGpu)
	{
		queues = detectQueues(discreteGpu, presentQueueFamily);
		if (!queues.empty()) m_physicalDevice = discreteGpu;
		else if (integratedGpu)
		{
			queues = detectQueues(integratedGpu, presentQueueFamily);
			if (!queues.empty()) m_physicalDevice = integratedGpu;
		}
	}
	if (!m_physicalDevice) ReturnIfFailed(VK_ERROR_INITIALIZATION_FAILED)	
	
	FeatureSet features;
	features.v13().dynamicRendering = VK_TRUE;
	features.v12().bufferDeviceAddress = VK_TRUE;
	requestFeatures(features);

	VkDeviceCreateInfo dci
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &features.m_vk10, 0, uint32_t(queues.size()), queues.data(), 0, nullptr, uint32_t(m_deviceExt.size()), m_deviceExt.data()
	};

	ReturnIfFailed(vkCreateDevice(m_physicalDevice, &dci, m_alloc, &m_device));
	volkLoadDevice(m_device);

	if (m_wndPtr)
	{
		BreakIfNot(presentQueueFamily != UINT32_MAX);
		m_wndPtr->setPresentQueue(getDeviceQueue(presentQueueFamily));
	}

	initialize();
}

void Application::finalizeInternal()
{
	BreakIfFailed(vkDeviceWaitIdle(m_device));
	finalize();

	if (m_wndPtr) m_wndPtr->destroySwapchain();
	vkDestroyDevice(m_device, m_alloc);

	if (m_wndPtr) m_wndPtr->destroyWindowAndSurface();
	vkDestroyInstance(m_instance, m_alloc);
}

}
