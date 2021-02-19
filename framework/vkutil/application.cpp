#include "application.h"

#include <new>

#include <stdio.h>

bool vkutil::Application::Initialize(void* pWindow)
{
    if (!UpdateFunctionPointers(m_vkInstance, m_vkDevice))
        return false;
    m_appInfo.pApplicationName = GetApplicationName();
    m_appInfo.apiVersion = (vkEnumerateInstanceVersion) ? VK_API_VERSION_1_1 : VK_API_VERSION_1_0;
    if (vkEnumerateInstanceVersion)
        VKUTIL_CHECK_RETURN(vkEnumerateInstanceVersion(&m_appInfo.apiVersion), false);    
    {
        const char* pValidationLayer = "VK_LAYER_KHRONOS_validation";
        VkInstanceCreateInfo info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        info.pApplicationInfo = &m_appInfo;
        SetInstanceExtensions(info);
#if VKUTIL_VALIDATION
        info.enabledLayerCount = 1;
        info.ppEnabledLayerNames = &pValidationLayer;
#endif
        VKUTIL_CHECK_RETURN(vkCreateInstance(&info, m_pVkAlloc, &m_vkInstance), false);
    }
    if (!UpdateFunctionPointers(m_vkInstance, m_vkDevice, m_appInfo.apiVersion))
        return false;
    if (!CreateWindowSurface(pWindow))
        return false;
    if (!SelectPhysicalDevice(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU))
        if (!SelectPhysicalDevice(VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU))
            return false;
    {
        float queuePriority = 1.f;
        VkDeviceQueueCreateInfo queue{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
        queue.queueFamilyIndex = m_queueFamily;
        queue.queueCount = 1;
        queue.pQueuePriorities = &queuePriority;
        VkDeviceCreateInfo info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
        info.queueCreateInfoCount = 1;
        info.pQueueCreateInfos = &queue;
        SetDeviceExtensions(info);
        VKUTIL_CHECK_RETURN(vkCreateDevice(m_vkPhysicalDevice, &info, m_pVkAlloc, &m_vkDevice), false);
        if (!UpdateFunctionPointers(m_vkInstance, m_vkDevice))
            return false;
        vkGetDeviceQueue(m_vkDevice, m_queueFamily, 0, &m_vkQueue);
    }    
    if (!CreateOrUpdateSwapchain())
        return false;
    if (!SetupCommandContext())
        return false;
    return OnInitialized();
}

void vkutil::Application::Finalize()
{
    vkDeviceWaitIdle(m_vkDevice);
    OnFinalized();
    for (uint32_t i = 0; i < m_numSwapchainFrames; i++)
    {
        vkDestroyImageView(m_vkDevice, m_pSwapchainImageView[i], m_pVkAlloc);
        vkDestroyFence(m_vkDevice, m_pCommandBufferFence[i], m_pVkAlloc);
        vkDestroySemaphore(m_vkDevice, m_pImageReadySem[i], m_pVkAlloc);
        vkDestroySemaphore(m_vkDevice, m_pSubmitDoneSem[i], m_pVkAlloc);
    }
    vkFreeCommandBuffers(m_vkDevice, m_vkCommandPool, m_numSwapchainFrames, m_pCommandBuffer);
    vkDestroyCommandPool(m_vkDevice, m_vkCommandPool, m_pVkAlloc);
    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapchain, m_pVkAlloc);
    vkDestroyDevice(m_vkDevice, m_pVkAlloc);
    vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, m_pVkAlloc);
    vkDestroyInstance(m_vkInstance, m_pVkAlloc);
    delete[] m_pSwapchainImageView;
    delete[] m_pCommandBufferFence;
    delete[] m_pSubmitDoneSem;
    delete[] m_pImageReadySem;
    delete[] m_pCommandBuffer;
}

const char* vkutil::Application::GetWindowTitle()
{
    uint32_t vMaj = VK_VERSION_MAJOR(m_appInfo.apiVersion);
    uint32_t vMin = VK_VERSION_MINOR(m_appInfo.apiVersion);
    uint32_t vRev = VK_VERSION_PATCH(m_appInfo.apiVersion);
    const char* pTitle = (m_pWndTitle) ? m_pWndTitle : ((m_pAppName) ? m_pAppName : nullptr);
    if (pTitle)
        snprintf(m_wndTitleText, sizeof(m_wndTitleText), "%s [API: %u.%u.%u] [GPU: %s]", pTitle, vMaj, vMin, vRev, m_phdProperties.deviceName);
    else
        snprintf(m_wndTitleText, sizeof(m_wndTitleText), "[API: %u.%u.%u] [GPU: %s]", vMaj, vMin, vRev, m_phdProperties.deviceName);
    return m_wndTitleText;
}

void vkutil::Application::NextFrame(float elapsedT, float deltaT)
{
    UpdateState(elapsedT, deltaT);

    m_commandContext = (m_commandContext + 1) % m_numSwapchainFrames;
    VkFence commandBufferFence = m_pCommandBufferFence[m_commandContext];
    VkSemaphore imageReadySem = m_pImageReadySem[m_commandContext];
    VkSemaphore submitDoneSem = m_pSubmitDoneSem[m_commandContext];
    m_vkCommandBuffer = m_pCommandBuffer[m_commandContext];

    VKUTIL_CHECK(vkWaitForFences(m_vkDevice, 1, &commandBufferFence, VK_TRUE, UINT64_MAX));
    VKUTIL_CHECK(vkResetFences(m_vkDevice, 1, &commandBufferFence));
    {
        VKUTIL_CHECK(vkResetCommandBuffer(m_vkCommandBuffer, 0));
        VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        VKUTIL_CHECK(vkBeginCommandBuffer(m_vkCommandBuffer, &info));
    }    
    
    bool shouldPresent = RenderFrame(m_commandContext);
    
    {
        VKUTIL_CHECK(vkEndCommandBuffer(m_vkCommandBuffer));
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        info.waitSemaphoreCount = (shouldPresent) ? 1 : 0;
        info.pWaitSemaphores = &imageReadySem;
        info.pWaitDstStageMask = &dstStage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &m_vkCommandBuffer;
        info.signalSemaphoreCount = (shouldPresent) ? 1 : 0;
        info.pSignalSemaphores = &submitDoneSem;
        VKUTIL_CHECK(vkQueueSubmit(m_vkQueue, 1, &info, commandBufferFence));
    }

    if (shouldPresent)
    {
        VkPresentInfoKHR info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &submitDoneSem;
        info.swapchainCount = 1;
        info.pSwapchains = &m_vkSwapchain;
        info.pImageIndices = &m_swapchainImageIndex;
        VKUTIL_CHECK(vkQueuePresentKHR(m_vkQueue, &info));
        m_swapchainImageIndex = UINT32_MAX;
    }
}

bool vkutil::Application::OnInitialized()
{
    return true;
}

void vkutil::Application::OnFinalized()
{
}

void vkutil::Application::UpdateState(float elapsedT, float deltaT)
{
}

bool vkutil::Application::RenderFrame(uint32_t contextIdx)
{
    return false;
}

void vkutil::Application::SetInstanceExtensions(VkInstanceCreateInfo& info)
{
    static const char* items[]{ VKUTIL_INSTANCE_DEAFULT_EXT };
    info.enabledExtensionCount = CountOf(items);
    info.ppEnabledExtensionNames = items;
}

void vkutil::Application::SetDeviceExtensions(VkDeviceCreateInfo& info)
{
    static const char* items[]{ VKUTIL_DEVICE_DEAFULT_EXT };
    info.enabledExtensionCount = CountOf(items);
    info.ppEnabledExtensionNames = items;
}

uint32_t vkutil::Application::GetSwapchainImageIndex()
{
    while (m_swapchainImageIndex == UINT32_MAX)
    {
        VkResult res = vkAcquireNextImageKHR(m_vkDevice, m_vkSwapchain, UINT64_MAX, m_pImageReadySem[m_commandContext], VK_NULL_HANDLE, &m_swapchainImageIndex);
        if (res != VK_SUCCESS)
        {
            //TODOD if (res == VK_ERROR_OUT_OF_DATE_KHR)
            VKUTIL_CHECK(res);
        }
    }
    return m_swapchainImageIndex;
}

bool vkutil::Application::SelectPhysicalDevice(VkPhysicalDeviceType type)
{
    uint32_t numPhysicalDevices = 0;
    VkPhysicalDevice* pPhysicalDevice = nullptr;
    VKUTIL_CHECK_RETURN(vkEnumeratePhysicalDevices(m_vkInstance, &numPhysicalDevices, pPhysicalDevice), false);
    if (numPhysicalDevices)
    {
        pPhysicalDevice = new (std::nothrow) VkPhysicalDevice[numPhysicalDevices];
        VKUTIL_CHECK_RETURN(vkEnumeratePhysicalDevices(m_vkInstance, &numPhysicalDevices, pPhysicalDevice), false);
        for (uint32_t i = 0; i < numPhysicalDevices; i++)
        {
            VkPhysicalDevice phDevice = pPhysicalDevice[i];
            vkGetPhysicalDeviceProperties(phDevice, &m_phdProperties);
            if (m_phdProperties.deviceType != type)
                continue;            
            uint32_t numQueueFamilies = 0;
            uint32_t queueFamily = UINT32_MAX;
            VkQueueFamilyProperties* pQueueFamily = nullptr;
            vkGetPhysicalDeviceQueueFamilyProperties(phDevice, &numQueueFamilies, pQueueFamily);
            if (numQueueFamilies)
            {
                pQueueFamily = new (std::nothrow) VkQueueFamilyProperties[numQueueFamilies];
                vkGetPhysicalDeviceQueueFamilyProperties(phDevice, &numQueueFamilies, pQueueFamily);
                for (uint32_t j = 0; j < numQueueFamilies; j++)
                {
                    VkBool32 canPresent = VK_FALSE;
                    VkQueueFlags flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
                    VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(phDevice, j, m_vkSurface, &canPresent);
                    if (result != VK_SUCCESS || canPresent != VK_TRUE)
                        continue;
                    const VkQueueFamilyProperties& qf = pQueueFamily[j];
                    if ((qf.queueFlags & flags) == flags)
                    {
                        queueFamily = j;
                        break;
                    }
                }
                delete[] pQueueFamily;
            }
            if (queueFamily != UINT32_MAX)
            {
                m_vkPhysicalDevice = phDevice;
                m_queueFamily = queueFamily;
                break;
            }
        }
        delete[] pPhysicalDevice;
        if (m_vkPhysicalDevice == VK_NULL_HANDLE)
            return false;
        return true;
    }
    return false;
}

bool vkutil::Application::DetectSwapchainExtent(VkSwapchainCreateInfoKHR& info)
{
    VkSurfaceCapabilitiesKHR caps;
    VKUTIL_CHECK_RETURN(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhysicalDevice, m_vkSurface, &caps), false);
    m_swapchainExtent.width = (caps.currentExtent.width == UINT32_MAX) ? static_cast<uint32_t>(m_windowW) : caps.currentExtent.width;
    m_swapchainExtent.height = (caps.currentExtent.height == UINT32_MAX) ? static_cast<uint32_t>(m_windowH) : caps.currentExtent.height;
    info.imageExtent = { m_swapchainExtent.width, m_swapchainExtent.height };
    info.imageArrayLayers = m_swapchainExtent.depth;
    info.preTransform = caps.currentTransform;
    uint32_t numImages = caps.minImageCount + 1;    
    info.minImageCount = (numImages > caps.maxImageCount) ? caps.maxImageCount : numImages;
    return true;
}

bool vkutil::Application::DetectSwapchainFormat(VkSwapchainCreateInfoKHR& info)
{
    uint32_t numSurfaceFormats = 0;
    VkSurfaceFormatKHR* pSurfaceFormat = nullptr;
    VKUTIL_CHECK_RETURN(vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &numSurfaceFormats, pSurfaceFormat), false);
    if (numSurfaceFormats)
    {
        uint32_t formatIdx = UINT32_MAX;
        pSurfaceFormat = new (std::nothrow) VkSurfaceFormatKHR[numSurfaceFormats];
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhysicalDevice, m_vkSurface, &numSurfaceFormats, pSurfaceFormat);
        for (uint32_t i = 0; i < numSurfaceFormats; i++)
        {
            if (pSurfaceFormat[i].format == m_swapchainFormat && pSurfaceFormat[i].colorSpace == m_swapchainColorSpace)
            {
                formatIdx = i;
                break;
            }
        }
        if (formatIdx == UINT32_MAX)
        {
            m_swapchainFormat = pSurfaceFormat[0].format;
            m_swapchainColorSpace = pSurfaceFormat[0].colorSpace;
        }
        delete[] pSurfaceFormat;
        info.imageFormat = m_swapchainFormat;
        info.imageColorSpace = m_swapchainColorSpace;
        return true;
    }
    return false;
}

bool vkutil::Application::DetectPresentMode(VkSwapchainCreateInfoKHR& info)
{
    uint32_t numPresentModes = 0;
    VkPresentModeKHR* pPresentMode = nullptr;
    VKUTIL_CHECK_RETURN(vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &numPresentModes, pPresentMode), false);
    if (numPresentModes)
    {
        uint32_t modeIdx = UINT32_MAX;
        pPresentMode = new (std::nothrow) VkPresentModeKHR[numPresentModes];
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhysicalDevice, m_vkSurface, &numPresentModes, pPresentMode);
        for (uint32_t i = 0; i < numPresentModes; i++)
        {
            if (pPresentMode[i] == m_presentMode)
            {
                modeIdx = i;
                break;
            }
        }
        if (modeIdx == UINT32_MAX)
            m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
        delete[] pPresentMode;
        return true;
    }
    return false;
}

bool vkutil::Application::CreateOrUpdateSwapchain()
{
    VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
    if (DetectSwapchainExtent(info) && DetectSwapchainFormat(info) && DetectPresentMode(info))
    { 
        info.surface = m_vkSurface;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.clipped = VK_TRUE;
        info.oldSwapchain = m_vkSwapchain;
        VKUTIL_CHECK_RETURN(vkCreateSwapchainKHR(m_vkDevice, &info, m_pVkAlloc, &m_vkSwapchain), false);
        m_numSwapchainFrames = 0;
        VkImage* pSwapchainImage = nullptr;
        VKUTIL_CHECK_RETURN(vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &m_numSwapchainFrames, pSwapchainImage), false);
        if (m_numSwapchainFrames)
        {
            pSwapchainImage = new (std::nothrow) VkImage[m_numSwapchainFrames];
            m_pSwapchainImageView = new (std::nothrow) VkImageView[m_numSwapchainFrames];
            vkGetSwapchainImagesKHR(m_vkDevice, m_vkSwapchain, &m_numSwapchainFrames, pSwapchainImage);
            for (uint32_t i = 0; i < m_numSwapchainFrames; i++)
            {
                VkImageViewCreateInfo view{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
                view.image = pSwapchainImage[i];
                view.viewType = VK_IMAGE_VIEW_TYPE_2D;
                view.format = m_swapchainFormat;
                view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                view.subresourceRange.layerCount = info.imageArrayLayers;
                view.subresourceRange.levelCount = 1;
                VKUTIL_CHECK_RETURN(vkCreateImageView(m_vkDevice, &view, m_pVkAlloc, &m_pSwapchainImageView[i]), false);
            }
            delete[] pSwapchainImage;
            return true;
        }
    }
    return false;
}

bool vkutil::Application::SetupCommandContext()
{
    if (m_numSwapchainFrames)
    {
        {
            VkCommandPoolCreateInfo info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            info.queueFamilyIndex = m_queueFamily;
            VKUTIL_CHECK_RETURN(vkCreateCommandPool(m_vkDevice, &info, m_pVkAlloc, &m_vkCommandPool), false);
        }
        { 
            m_pCommandBuffer = new (std::nothrow) VkCommandBuffer[m_numSwapchainFrames];
            VkCommandBufferAllocateInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            info.commandPool = m_vkCommandPool;
            info.commandBufferCount = m_numSwapchainFrames;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;            
            VKUTIL_CHECK_RETURN(vkAllocateCommandBuffers(m_vkDevice, &info, m_pCommandBuffer), false);
        }
        {
            m_pImageReadySem = new (std::nothrow) VkSemaphore[m_swapchainFormat];
            m_pSubmitDoneSem = new (std::nothrow) VkSemaphore[m_swapchainFormat];
            m_pCommandBufferFence = new (std::nothrow) VkFence[m_swapchainFormat];
            VkSemaphoreCreateInfo sInfo{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            VkFenceCreateInfo fInfo{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
            fInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            for (uint32_t i = 0; i < m_numSwapchainFrames; i++)
            {
                VKUTIL_CHECK_RETURN(vkCreateFence(m_vkDevice, &fInfo, m_pVkAlloc, &m_pCommandBufferFence[i]), false);
                VKUTIL_CHECK_RETURN(vkCreateSemaphore(m_vkDevice, &sInfo, m_pVkAlloc, &m_pImageReadySem[i]), false);
                VKUTIL_CHECK_RETURN(vkCreateSemaphore(m_vkDevice, &sInfo, m_pVkAlloc, &m_pSubmitDoneSem[i]), false);
            }
        }
        m_commandContext = m_numSwapchainFrames - 1;
        return true;
    }
    return false;
}

#if !defined(VKUTIL_APP_CUSTOM_DESTROY)
void DestroyApplication(vkutil::Application* pApp)
{
    delete pApp;
}
#endif
