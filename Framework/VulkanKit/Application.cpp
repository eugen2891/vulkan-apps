#include "Application.hpp"
#include "Window.hpp"

#include <Volk/volk.c>

void vulkan::Application::main()
{
	initializeInternal();
	runApplication();
	finalizeInternal();
}

void vulkan::Application::initialize()
{
}

void vulkan::Application::finalize()
{
}

void vulkan::Application::setOutputWindow(Window* wndPtr)
{
	m_wndPtr = wndPtr;
}

const char* vulkan::Application::applicationName() const
{
	return "vulkan::ApplicationBase";
}

const char* vulkan::Application::engineName() const
{
	return "VulkanHelper";
}

bool vulkan::Application::canPresent(VkPhysicalDevice physicalDevice, uint32_t family) const
{
	if (m_wndPtr)
	{
		VkBool32 isSupported = VK_FALSE;
		RetvalIfFailed(vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, family, m_wndPtr->surface(), &isSupported), false);
		return (isSupported == VK_TRUE);
	}
	return false;
}

void vulkan::Application::initializeInternal()
{
	ReturnIfFailed(volkInitialize());

	const VkApplicationInfo ai
	{ 
		VK_STRUCTURE_TYPE_APPLICATION_INFO, nullptr, applicationName(), 0u, engineName(), 0u, VK_API_VERSION_1_3 
	};

	const char* instanceExt[]{ VK_KHR_SURFACE_EXTENSION_NAME, VK_NATIVE_SURFACE_EXTENSION_NAME };

	const VkInstanceCreateInfo ici
	{
		VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, nullptr, 0, &ai, 0, nullptr, CountOf(instanceExt), instanceExt
	};

	ReturnIfFailed(vkCreateInstance(&ici, m_alloc, &m_instance));
	volkLoadInstanceOnly(m_instance);

	if (m_wndPtr) m_wndPtr->createWindowAndSurface();

	VkPhysicalDevice discreteGpu = VK_NULL_HANDLE;
	VkPhysicalDevice integratedGpu = VK_NULL_HANDLE;

	PhysicalDevicesList physicalDevices(m_instance);
	for (VkPhysicalDevice physicalDevice : physicalDevices)
	{
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties(physicalDevice, &props);
		if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && !discreteGpu) discreteGpu = physicalDevice;
		else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && !integratedGpu) integratedGpu = physicalDevice;
	}
	 
	if (discreteGpu)
	{
		if (detectQueues(discreteGpu)) m_physicalDevice = discreteGpu; 
	}
	if (!m_physicalDevice && integratedGpu)
	{
		if (detectQueues(integratedGpu)) m_physicalDevice = integratedGpu; 
	}
	if (!m_physicalDevice) ReturnIfFailed(VK_ERROR_INITIALIZATION_FAILED);

	VkPhysicalDeviceVulkan13Features features13{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
	VkPhysicalDeviceVulkan12Features features12{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, &features13 };
	VkPhysicalDeviceVulkan12Features features11{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES, &features12 };
	VkPhysicalDeviceFeatures2 features10{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, &features11 };
	vkGetPhysicalDeviceFeatures2(m_physicalDevice, &features10);

	DeviceQueueCreateList queues = queueInfos();
	const char* deviceExt[]{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkDeviceCreateInfo dci
	{
		VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO, &features10, 0, queues.num, queues.items, 0, nullptr, CountOf(deviceExt), deviceExt
	};

	ReturnIfFailed(vkCreateDevice(m_physicalDevice, &dci, m_alloc, &m_device));
	volkLoadDevice(m_device);

	if (m_wndPtr) m_wndPtr->setDeviceInfo({ presentQueue() });

	initialize();
}

void vulkan::Application::finalizeInternal()
{
	BreakIfFailed(vkDeviceWaitIdle(m_device));
	finalize();

	if (m_wndPtr) m_wndPtr->destroySwapchain();
	vkDestroyDevice(m_device, m_alloc);

	if (m_wndPtr) m_wndPtr->destroyWindowAndSurface();
	vkDestroyInstance(m_instance, m_alloc);
}
