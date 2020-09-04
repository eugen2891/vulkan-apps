#if defined(_WIN32)
struct IUnknown;
#endif

#define VKAPI_INL

#include <vkutil/vulkan_app.h>
#include <vkutil/mem_stack.h>

#include <GLFW/glfw3.h>

template <typename T>
inline bool LoadVkInstanceProc(VkInstance vkInst, const char* pName, T& out)
{
    out = reinterpret_cast<T>(glfwGetInstanceProcAddress(vkInst, pName));
    return (out != nullptr);
}

template <typename T>
inline bool LoadVkDeviceProc(VkDevice vkDev, const char* pName, T& out)
{
    out = reinterpret_cast<T>(vkGetDeviceProcAddr(vkDev, pName));
    return (out != nullptr);
}

#define LOAD_VK_INSTANCE_PROC_OR_FALSE(i,n) if (!LoadVkInstanceProc(i, #n, n)) return false
#define LOAD_VK_DEVICE_PROC_OR_FALSE(d,n) if (!LoadVkDeviceProc(d, #n, n)) return false

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
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkEnumerateInstanceVersion);
#endif
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
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkEnumeratePhysicalDeviceGroups);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceFeatures2);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceProperties2);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceFormatProperties2);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceImageFormatProperties2);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceQueueFamilyProperties2);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceMemoryProperties2);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceSparseImageFormatProperties2);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceExternalBufferProperties);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceExternalFenceProperties);
        LOAD_VK_INSTANCE_PROC_OR_FALSE(pAppImpl->vkInstance, vkGetPhysicalDeviceExternalSemaphoreProperties);
#endif
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
        pAppImpl->vkPhDev = (bestFit == VK_NULL_HANDLE) ? fallback : bestFit;
    }
    vkGetPhysicalDeviceQueueFamilyProperties(pAppImpl->vkPhDev, &pAppImpl->numDevQueues, nullptr);
    pAppImpl->pDeviceQueue = memStack.allocLinear<DeviceQueue>(pAppImpl->numDevQueues);
    memStack.pushFrame();
    VkQueueFamilyProperties* qfProps = memStack.allocStack<VkQueueFamilyProperties>(pAppImpl->numDevQueues);
    vkGetPhysicalDeviceQueueFamilyProperties(pAppImpl->vkPhDev, &pAppImpl->numDevQueues, qfProps);
    for (uint32_t i = 0; i < pAppImpl->numDevQueues; i++)
    {
        pAppImpl->pDeviceQueue[i].properties = qfProps[i];
        vkGetPhysicalDeviceSurfaceSupportKHR(pAppImpl->vkPhDev, i, pAppImpl->vkSurface, &pAppImpl->pDeviceQueue[i].canPresent);
        if (pAppImpl->graphicsQueue == UINT32_MAX && (qfProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
            pAppImpl->graphicsQueue = i;
        if (pAppImpl->transferQueue == UINT32_MAX && (qfProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT))
            pAppImpl->transferQueue = i;
        if (pAppImpl->presentQueue == UINT32_MAX && (pAppImpl->pDeviceQueue[i].canPresent == VK_TRUE))
            pAppImpl->presentQueue = i;
    }
    memStack.popFrame();
    if (!pAppImpl->assignDeviceQueues())
        return false;
    {
        float queuePriority = 1.f;
        VkDeviceQueueCreateInfo qInfo[3]
        {
            { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, &queuePriority },
            { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, &queuePriority },
            { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, nullptr, 0, 0, 1, &queuePriority }
        };
        TInlineArray<const char*, 16> extensions;
        extensions.addItem(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
        pAppImpl->addDeviceExtensions(extensions);
        VkDeviceCreateInfo info = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        info.queueCreateInfoCount = 1;
        info.pQueueCreateInfos = qInfo;
        qInfo[0].queueFamilyIndex = pAppImpl->graphicsQueue;
        if (pAppImpl->transferQueue != pAppImpl->graphicsQueue)
            qInfo[info.queueCreateInfoCount++].queueFamilyIndex = pAppImpl->transferQueue;
        if ((pAppImpl->presentQueue != pAppImpl->graphicsQueue) &&
            (pAppImpl->presentQueue != pAppImpl->transferQueue))
            qInfo[info.queueCreateInfoCount++].queueFamilyIndex = pAppImpl->presentQueue;
        info.enabledExtensionCount = extensions.numItems;
        info.ppEnabledExtensionNames = extensions.pItems;
        vkResult = vkCreateDevice(pAppImpl->vkPhDev, &info, pAppImpl->getAllocator(), &pAppImpl->vkDevice);
        if (vkResult != VK_SUCCESS)
            return false;
        LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetDeviceQueue);
        for (uint32_t i = 0; i < info.queueCreateInfoCount; i++)
        {
            uint32_t family = info.pQueueCreateInfos[i].queueFamilyIndex;
            vkGetDeviceQueue(pAppImpl->vkDevice, family, 0, &pAppImpl->pDeviceQueue[family].handle);
        }
    }
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyDevice);    
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkQueueSubmit);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkQueueWaitIdle);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDeviceWaitIdle);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkAllocateMemory);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkFreeMemory);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkMapMemory);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkUnmapMemory);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkFlushMappedMemoryRanges);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkInvalidateMappedMemoryRanges);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetDeviceMemoryCommitment);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkBindBufferMemory);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkBindImageMemory);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetBufferMemoryRequirements);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetImageMemoryRequirements);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetImageSparseMemoryRequirements);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkQueueBindSparse);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateFence);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyFence);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkResetFences);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetFenceStatus);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkWaitForFences);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateSemaphore);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroySemaphore);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateEvent);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyEvent);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetEventStatus);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkSetEvent);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkResetEvent);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateQueryPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyQueryPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetQueryPoolResults);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateBufferView);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyBufferView);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateImage);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyImage);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetImageSubresourceLayout);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateImageView);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyImageView);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateShaderModule);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyShaderModule);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreatePipelineCache);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyPipelineCache);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetPipelineCacheData);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkMergePipelineCaches);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateGraphicsPipelines);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateComputePipelines);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyPipeline);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreatePipelineLayout);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyPipelineLayout);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateSampler);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroySampler);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateDescriptorSetLayout);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyDescriptorSetLayout);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateDescriptorPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyDescriptorPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkResetDescriptorPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkAllocateDescriptorSets);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkFreeDescriptorSets);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkUpdateDescriptorSets);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateFramebuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyFramebuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateRenderPass);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyRenderPass);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetRenderAreaGranularity);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateCommandPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyCommandPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkResetCommandPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkAllocateCommandBuffers);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkFreeCommandBuffers);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkBeginCommandBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkEndCommandBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkResetCommandBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdBindPipeline);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetViewport);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetScissor);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetLineWidth);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetDepthBias);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetBlendConstants);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetDepthBounds);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetStencilCompareMask);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetStencilWriteMask);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetStencilReference);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdBindDescriptorSets);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdBindIndexBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdBindVertexBuffers);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDraw);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDrawIndexed);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDrawIndirect);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDrawIndexedIndirect);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDispatch);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDispatchIndirect);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdCopyBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdCopyImage);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdBlitImage);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdCopyBufferToImage);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdCopyImageToBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdUpdateBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdFillBuffer);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdClearColorImage);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdClearDepthStencilImage);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdClearAttachments);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdResolveImage);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetEvent);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdResetEvent);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdWaitEvents);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdPipelineBarrier);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdBeginQuery);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdEndQuery);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdResetQueryPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdWriteTimestamp);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdCopyQueryPoolResults);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdPushConstants);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdBeginRenderPass);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdNextSubpass);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdEndRenderPass);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdExecuteCommands);        
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateSwapchainKHR);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroySwapchainKHR);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetSwapchainImagesKHR);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkAcquireNextImageKHR);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkQueuePresentKHR);
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkBindBufferMemory2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkBindImageMemory2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetDeviceGroupPeerMemoryFeatures);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdSetDeviceMask);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDispatchBase);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetImageMemoryRequirements2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetBufferMemoryRequirements2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetImageSparseMemoryRequirements2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkTrimCommandPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetDeviceQueue2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateSamplerYcbcrConversion);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroySamplerYcbcrConversion);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateDescriptorUpdateTemplate);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkDestroyDescriptorUpdateTemplate);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkUpdateDescriptorSetWithTemplate);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetDescriptorSetLayoutSupport);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetDeviceGroupPresentCapabilitiesKHR);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetDeviceGroupSurfacePresentModesKHR);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetPhysicalDevicePresentRectanglesKHR);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkAcquireNextImage2KHR);
#endif
#if (VKAPI_VERSION >= VK_API_VERSION_1_2)
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDrawIndirectCount);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdDrawIndexedIndirectCount);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCreateRenderPass2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdBeginRenderPass2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdNextSubpass2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkCmdEndRenderPass2);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkResetQueryPool);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetSemaphoreCounterValue);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkWaitSemaphores);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkSignalSemaphore);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetBufferDeviceAddress);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetBufferOpaqueCaptureAddress);
    LOAD_VK_DEVICE_PROC_OR_FALSE(pAppImpl->vkDevice, vkGetDeviceMemoryOpaqueCaptureAddress);
#endif
    {
        VkSurfaceCapabilitiesKHR caps;
        VkSurfaceFormatKHR format = pAppImpl->getSurfaceFormat();
        VkPresentModeKHR presentMode = pAppImpl->getPresentMode();
        vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pAppImpl->vkPhDev, pAppImpl->vkSurface, &caps);
        if (vkResult != VK_SUCCESS)
            return false;
        uint32_t numImages = (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) ? 3 : 2;
        uint32_t queueFamilies[] = { pAppImpl->graphicsQueue, pAppImpl->presentQueue };
        VkSwapchainCreateInfoKHR info = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        info.surface = pAppImpl->vkSurface;
        info.minImageCount = (numImages < caps.minImageCount) ? caps.minImageCount : ((numImages > caps.maxImageCount) ? caps.maxImageCount : numImages);
        info.imageFormat = format.format;
        info.imageColorSpace = format.colorSpace;
        info.imageExtent = caps.currentExtent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode = (pAppImpl->graphicsQueue != pAppImpl->presentQueue) ? VK_SHARING_MODE_CONCURRENT : VK_SHARING_MODE_EXCLUSIVE;
        info.queueFamilyIndexCount = 2;
        info.pQueueFamilyIndices = queueFamilies;
        info.preTransform = caps.currentTransform;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = presentMode;
        info.clipped = VK_TRUE;
        info.oldSwapchain = VK_NULL_HANDLE;
        vkResult = vkCreateSwapchainKHR(pAppImpl->vkDevice, &info, pAppImpl->getAllocator(), &pAppImpl->vkSwapchain);
        if (vkResult != VK_SUCCESS)
            return false;
        vkResult = vkGetSwapchainImagesKHR(pAppImpl->vkDevice, pAppImpl->vkSwapchain, &pAppImpl->swapchainSize, nullptr);
        if (vkResult != VK_SUCCESS)
            return false;
        pAppImpl->pSwapchainImage = memStack.allocStack<VkImage>(pAppImpl->swapchainSize);
        pAppImpl->pSwapchainImageView = memStack.allocStack<VkImageView>(pAppImpl->swapchainSize);
        vkGetSwapchainImagesKHR(pAppImpl->vkDevice, pAppImpl->vkSwapchain, &pAppImpl->swapchainSize, pAppImpl->pSwapchainImage);
        for (uint32_t i = 0; i < pAppImpl->swapchainSize; i++)
        {
            if (vkResult == VK_SUCCESS)
            {
                VkImageViewCreateInfo viewInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
                viewInfo.image = pAppImpl->pSwapchainImage[i];
                viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
                viewInfo.format = format.format;
                viewInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
                viewInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
                vkResult = vkCreateImageView(pAppImpl->vkDevice, &viewInfo, pAppImpl->getAllocator(), &pAppImpl->pSwapchainImageView[i]);
            }
            if (vkResult != VK_SUCCESS)
                pAppImpl->pSwapchainImageView[i] = VK_NULL_HANDLE;
        }
    }
    return pAppImpl->initialize();
}

void VulkanApp::shutdown()
{
    if (pAppImpl)
    {
        if (pAppImpl->vkDevice)
        {
            vkDeviceWaitIdle(pAppImpl->vkDevice);
            if (pAppImpl->vkSwapchain)
            {
                for (uint32_t i = 0; i < pAppImpl->swapchainSize; i++)
                {
                    if (pAppImpl->pSwapchainImageView[i] != VK_NULL_HANDLE)
                        vkDestroyImageView(pAppImpl->vkDevice, pAppImpl->pSwapchainImageView[i], pAppImpl->getAllocator());
                }
                vkDestroySwapchainKHR(pAppImpl->vkDevice, pAppImpl->vkSwapchain, pAppImpl->getAllocator());
            }
            vkDestroyDevice(pAppImpl->vkDevice, pAppImpl->getAllocator());
        }
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

void VulkanApp::addDeviceExtensions(TArray<const char*>& list)
{
}

VkSurfaceFormatKHR VulkanApp::getSurfaceFormat()
{
    uint32_t numFormats = 0;
    VkSurfaceFormatKHR retVal = { /*VK_FORMAT_B8G8R8A8_SRGB*/ VK_FORMAT_UNDEFINED , VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
    VkResult vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(phDevice, surface, &numFormats, nullptr);
    if (vkResult == VK_SUCCESS)
    {
        memStack.pushFrame();
        VkSurfaceFormatKHR* formats = memStack.allocStack<VkSurfaceFormatKHR>(numFormats);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phDevice, surface, &numFormats, formats);
        for (uint32_t i = 0; i < numFormats; i++)
        {
            if (formats[i].format == VK_FORMAT_B8G8R8A8_SRGB
                && formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                retVal.format = VK_FORMAT_B8G8R8A8_SRGB;
                break;
            }
        }
        memStack.popFrame();
    }
    return retVal;
}

VkPresentModeKHR VulkanApp::getPresentMode()
{
    UINT32 numModes = 0;
    VkPresentModeKHR retVal = VK_PRESENT_MODE_FIFO_KHR;
    VkResult vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(phDevice, surface, &numModes, nullptr);
    if (vkResult == VK_SUCCESS)
    {
        memStack.pushFrame();
        VkPresentModeKHR* modes = memStack.allocStack<VkPresentModeKHR>(numModes);
        vkGetPhysicalDeviceSurfacePresentModesKHR(phDevice, surface, &numModes, modes);
        for (uint32_t i = 0; i < numModes; i++)
        {
            if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                retVal = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
        }
        memStack.popFrame();
    }
    return retVal;
}

bool VulkanApp::assignDeviceQueues()
{
    return true;
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
