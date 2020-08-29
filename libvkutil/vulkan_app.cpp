#define VKAPP_IMPL

#if defined(_WIN32)
struct IUnknown;
#endif

#include <vkutil/vulkan_app.inl>

template <typename T>
inline bool LoadVkInstanceProc(VkInstance vkInst, const char* pName, T& out)
{
    out = reinterpret_cast<T>(glfwGetInstanceProcAddress(vkInst, pName));
    return (out != nullptr);
}
#define LOAD_VK_INSTANCE_PROC_OR_FALSE(i,n) if (!LoadVkInstanceProc(i, #n, n)) return false

template <typename T>
struct TArray
{
    void addItem(const char* pStr) { if (numItems < maxItems) pRawItems[numItems++] = pStr; }

    T*       pRawItems = nullptr;
    uint32_t numItems = 0;
    uint32_t maxItems = 0;
};

template <typename T, uint32_t N>
struct TInlineArray : public TArray<T>
{
    TInlineArray() { TArray<T>::maxItems = N; TArray<T>::pRawItems = pItems; }

    T pItems[N];
};

const char* VulkanApp::pEngineName = "VulkanApp Demo Engine";
const char* VulkanApp::pAppName = nullptr;
VulkanApp* VulkanApp::pAppImpl = nullptr;

int VulkanApp::main()
{
    if (glfwInit() && glfwVulkanSupported() && VulkanApp::startup())
    {
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* window = glfwCreateWindow(1280, 800, "", nullptr, nullptr);
        if (VulkanApp::createDeviceAndSwapchain(window))
        {
            glfwSetWindowSizeLimits(window, 1280, 800, GLFW_DONT_CARE, GLFW_DONT_CARE);
            while (!glfwWindowShouldClose(window))
            {
                glfwPollEvents();
            }
        }
        VulkanApp::shutdown();
        glfwTerminate();
    }
    return 0;
}

VulkanApp::VulkanApp(MemStack& stack)
    : memStack(stack)
{
}

bool VulkanApp::startup()
{
    if (pAppImpl != nullptr)
    {
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkCreateInstance);
        VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
        appInfo.pApplicationName = pAppName;
        appInfo.pEngineName = pEngineName;
        appInfo.apiVersion = VKAPI_VERSION;
        TInlineArray<const char*, 16> layers;
        TInlineArray<const char*, 16> extensions;
        extensions.addItem(VK_KHR_SURFACE_EXTENSION_NAME);
        extensions.addItem(VKAPI_NATIVE_SURFACE_EXT_NAME);
        pAppImpl->addInstanceExtensions(extensions);
        pAppImpl->addValidationLayers(layers);
        VkInstanceCreateInfo info = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        info.pApplicationInfo = &appInfo;
        info.enabledLayerCount = layers.numItems;
        info.ppEnabledLayerNames = layers.pItems;
        info.enabledExtensionCount = extensions.numItems;
        info.ppEnabledExtensionNames = extensions.pItems;
        VkResult vkResult = vkCreateInstance(&info, pAppImpl->getAllocator(), &pAppImpl->vkInstance);
        if (vkResult != VK_SUCCESS)
            return false;
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkDestroyInstance);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkDestroySurfaceKHR);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkEnumeratePhysicalDevices);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceFeatures);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceFormatProperties);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceImageFormatProperties);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceProperties);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceQueueFamilyProperties);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceMemoryProperties);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetInstanceProcAddr);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetDeviceProcAddr);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkCreateDevice);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkDestroySurfaceKHR);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceSurfaceSupportKHR);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceSurfaceFormatsKHR);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceSurfacePresentModesKHR);
    }
    return true;
}

bool VulkanApp::createDeviceAndSwapchain(GLFWwindow* window)
{
    if (!window)
        return false;
    MemStack& memStack = pAppImpl->memStack;
    VkResult vkResult = glfwCreateWindowSurface(pAppImpl->vkInstance, window, pAppImpl->getAllocator(), &pAppImpl->vkSurface);
    if (vkResult != VK_SUCCESS)
        return false;
    else
    {
        uint32_t numPhDev = 0;
        vkResult = vkEnumeratePhysicalDevices(pAppImpl->vkInstance, &numPhDev, nullptr);
        if (vkResult != VK_SUCCESS || numPhDev == 0)
            return false;
        memStack.pushFrame();
        VkPhysicalDevice bestFit = VK_NULL_HANDLE, fallback = VK_NULL_HANDLE;
        VkPhysicalDevice* pPhDev = memStack.allocStack<VkPhysicalDevice>(numPhDev);
        vkEnumeratePhysicalDevices(pAppImpl->vkInstance, &numPhDev, pPhDev);
        for (uint32_t i = 0; i < numPhDev; i++)
        {
            memStack.pushFrame();
            switch (pAppImpl->canUsePhysicalDevice(pPhDev[i]))
            {
            case PHDEV_BEST_FIT:
                if (bestFit == VK_NULL_HANDLE) bestFit = pPhDev[i];
                break;
            case PHDEV_FALLBACK:
                if (fallback == VK_NULL_HANDLE) fallback = pPhDev[i];
                break;
            case PHDEV_UNUSABLE:
            default:
                break;
            }
            memStack.popFrame();
        }
        memStack.popFrame();
        if (bestFit == VK_NULL_HANDLE && fallback == VK_NULL_HANDLE)
            return false;
    }
    //create logical device
    //load device functions
    //create swapchain
    //get swapchain images and device queues
    return pAppImpl->initialize();
}

void VulkanApp::shutdown()
{
    if (pAppImpl)
    {
        if (pAppImpl->vkInstance)
        {
            if (pAppImpl->vkSurface)
                vkDestroySurfaceKHR(pAppImpl->vkInstance, pAppImpl->vkSurface, pAppImpl->getAllocator());
            vkDestroyInstance(pAppImpl->vkInstance, pAppImpl->getAllocator());
        }
    }
}

VkAllocationCallbacks* VulkanApp::getAllocator()
{
    return nullptr;
}

void VulkanApp::addInstanceExtensions(TArray<const char*>& list)
{
}

void VulkanApp::addValidationLayers(TArray<const char*>& list)
{
}

VulkanApp::PhDevUsability VulkanApp::canUsePhysicalDevice(VkPhysicalDevice phDev)
{
    uint32_t numQueueFamilies = 0;
    VkBool32 canPresent = VK_FALSE;
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(phDev, &props);
    if (props.deviceType != VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && props.deviceType != VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        return PHDEV_UNUSABLE;
    vkGetPhysicalDeviceQueueFamilyProperties(phDev, &numQueueFamilies, nullptr);
    if (numQueueFamilies == 0)
        return PHDEV_UNUSABLE;
    VkQueueFamilyProperties* qfProps = memStack.allocStack<VkQueueFamilyProperties>(numQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(phDev, &numQueueFamilies, qfProps);
    for (uint32_t i = 0; i < numQueueFamilies; i++)
    {
        if (vkGetPhysicalDeviceSurfaceSupportKHR(phDev, i, surface, &canPresent) != VK_SUCCESS)
            return PHDEV_UNUSABLE;
        else if (canPresent == VK_TRUE)
            break;
    }
    if (canPresent != VK_TRUE)
        return PHDEV_UNUSABLE;
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        return PHDEV_FALLBACK;
    return PHDEV_BEST_FIT;
}

bool VulkanApp::initialize()
{
    return true;
}

void VulkanApp::setImpl(VulkanApp* pImpl, const char* pName)
{
    if (!pAppImpl)
    {
        pAppName = (pName) ? pName : "<nonanme>";
        pAppImpl = pImpl;
    }
}
