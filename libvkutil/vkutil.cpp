#if (_DEBUG)
#define VKUTIL_DEBUG 1
#endif

#include "vkutil.h"

#if !defined(WITHOUT_GLFW)
#include <GLFW/glfw3.h>
#else
#error Running without GLFW is not supported
#endif

#if defined(_WIN32)
#define VKAPI_NATIVE_SURFACE_EXT_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#define NUM_FRAME_CONTEXTS 3
#define VKUTIL_COLOR_BUFFER_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#define VKUTIL_DEPTH_BUFFER_FORMAT VK_FORMAT_D24_UNORM_S8_UINT

#if !defined(WITHOUT_NEW)
#include <new>
#define ALLOC(e) new (std::nothrow) e
#define FREE(p) { delete[] p; p = nullptr; }
#define SWAP(a,b) { auto t = a; a = b; b = t; }
#else
#error Running without C++ new is not supported
#endif

#if _DEBUG
#define FAIL_RETURN() { __debugbreak(); return; }
#define FAIL_RETVAL(v) { __debugbreak(); return v; }
#else
#define FAIL_RETURN() return;
#define FAIL_RETVAL(v) return v;
#endif

static VkAllocationCallbacks* g_pMAlloc = nullptr;
static VkPhysicalDevice* g_pPhysicalDevice = nullptr;
static VkQueueFamilyProperties** g_ppQueueFamilyProperties = nullptr;
static uint32_t* g_pNumQueueFamilies = nullptr;
static VkInstance g_instance = VK_NULL_HANDLE;
static VkDevice g_device = VK_NULL_HANDLE;
static VkSurfaceKHR g_surface = VK_NULL_HANDLE;
static VkSwapchainKHR g_swapchain =VK_NULL_HANDLE;
static VkQueue g_queue = VK_NULL_HANDLE;
static VkImage* g_pSwapchainImage = nullptr;
static VkImageView* g_pSwapchainImageView = nullptr;
static VkFramebuffer* g_pSwapchainFramebuffer = nullptr;
static VkPhysicalDeviceMemoryProperties g_memProps;
static VkImage g_depthImage = VK_NULL_HANDLE;
static VkDeviceMemory g_depthImageMemory = VK_NULL_HANDLE;
static VkImageView g_depthImageView = VK_NULL_HANDLE;
static VkRenderPass g_dummyRenderPass = VK_NULL_HANDLE;
static VkCommandPool g_commandPool = VK_NULL_HANDLE;
static VkExtent3D g_screenSize;
static VkFormat g_colorFormat = VK_FORMAT_UNDEFINED;
static uint32_t g_numPhysicalDevices;
static uint32_t g_physicalDevice;
static uint32_t g_queueFamily;
static uint32_t g_swapchainSize;
static uint32_t g_currentImage = UINT32_MAX;
static VkResult g_lastResult;
static VkViewport g_viewport;
static VkRect2D g_scissor;

static struct FrameData
{
    VkCommandBuffer commandBuffer;
    VkSemaphore imageReady, submitDone;
    VkFence fence;
    FrameData* pNext;
} *g_pFrameData, *g_pCurrentFrame;

static inline bool LoadVkLibrary()
{
#if !defined(WITHOUT_GLFW)
    return glfwVulkanSupported() == GLFW_TRUE;
#else
#error
#endif
}

template <typename T>
static inline bool LoadVkInstanceProc(VkInstance vkInst, const char* pName, T& out)
{
#if !defined(WITHOUT_GLFW)
    out = reinterpret_cast<T>(glfwGetInstanceProcAddress(vkInst, pName));
#else
#error
#endif
    return (out != nullptr);
}

template <typename T>
static inline bool LoadVkDeviceProc(VkDevice vkDev, const char* pName, T& out)
{
    out = reinterpret_cast<T>(vkGetDeviceProcAddr(vkDev, pName));
    return (out != nullptr);
}

static void DestroySwapchainResources(bool destroySwapchain)
{
    for (uint32_t i = 0; i < g_swapchainSize; i++)
    {
        if (g_pSwapchainFramebuffer[i] != VK_NULL_HANDLE)
            vkDestroyFramebuffer(g_device, g_pSwapchainFramebuffer[i], g_pMAlloc);
        if (g_pSwapchainImageView[i] != VK_NULL_HANDLE)
            vkDestroyImageView(g_device, g_pSwapchainImageView[i], g_pMAlloc);
    }
    if (g_depthImageView != VK_NULL_HANDLE)
        vkDestroyImageView(g_device, g_depthImageView, g_pMAlloc);
    if (g_depthImage != VK_NULL_HANDLE)
        vkDestroyImage(g_device, g_depthImage, g_pMAlloc);
    if (g_depthImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(g_device, g_depthImageMemory, g_pMAlloc);
    if (g_dummyRenderPass != VK_NULL_HANDLE)
    {
        vkDestroyRenderPass(g_device, g_dummyRenderPass, g_pMAlloc);
        g_dummyRenderPass = VK_NULL_HANDLE;
    }
    FREE(g_pSwapchainFramebuffer);
    FREE(g_pSwapchainImageView);
    FREE(g_pSwapchainImage);
    g_swapchainSize = 0;
    if (destroySwapchain)
    {
        vkDestroySwapchainKHR(g_device, g_swapchain, g_pMAlloc);
        g_swapchain = VK_NULL_HANDLE;
    }
}

static void CreateSwapchainResources(bool depthBuffer)
{
    g_lastResult = vkDeviceWaitIdle(g_device);
    if (g_lastResult != VK_SUCCESS)
        FAIL_RETURN();
    {
        VkSurfaceCapabilitiesKHR caps;
        if (g_swapchain != VK_NULL_HANDLE)
            DestroySwapchainResources(false);
        g_lastResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(g_pPhysicalDevice[g_physicalDevice], g_surface, &caps);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();

        VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        info.surface = g_surface;
        info.minImageCount = (NUM_FRAME_CONTEXTS < caps.minImageCount)
            ? caps.minImageCount
            : ((NUM_FRAME_CONTEXTS > caps.maxImageCount) ? caps.maxImageCount : NUM_FRAME_CONTEXTS);
        {
            uint32_t count = 0;
            g_lastResult = vkGetPhysicalDeviceSurfaceFormatsKHR(g_pPhysicalDevice[g_physicalDevice], g_surface, &count, nullptr);
            if (g_lastResult != VK_SUCCESS)
                FAIL_RETURN();
            VkSurfaceFormatKHR* format = ALLOC(VkSurfaceFormatKHR[count]);
            vkGetPhysicalDeviceSurfaceFormatsKHR(g_pPhysicalDevice[g_physicalDevice], g_surface, &count, format);
            for (uint32_t i = 0; i < count; i++)
            {
                if (format[i].format == VKUTIL_COLOR_BUFFER_FORMAT)
                {
                    info.imageFormat = format[i].format;
                    info.imageColorSpace = format[i].colorSpace;
                    break;
                }
            }
            if (info.imageFormat == VK_FORMAT_UNDEFINED)
            {
                info.imageFormat = format[0].format;
                info.imageColorSpace = format[0].colorSpace;
            }
            FREE(format);
            if (info.imageFormat == VK_FORMAT_UNDEFINED)
                FAIL_RETURN();
            g_colorFormat = info.imageFormat;
        }
        info.imageExtent = caps.currentExtent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform = caps.currentTransform;
        info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        {
            uint32_t count = 0;
            g_lastResult = vkGetPhysicalDeviceSurfacePresentModesKHR(g_pPhysicalDevice[g_physicalDevice], g_surface, &count, nullptr);
            if (g_lastResult != VK_SUCCESS)
                FAIL_RETURN();
            VkPresentModeKHR* mode = ALLOC(VkPresentModeKHR[count]);
            vkGetPhysicalDeviceSurfacePresentModesKHR(g_pPhysicalDevice[g_physicalDevice], g_surface, &count, mode);
            bool isSupported = false;
            for (uint32_t i = 0; i < count; i++)
            {
                if (mode[i] == info.presentMode)
                {
                    isSupported = true;
                    break;
                }
            }
            if (!isSupported)
                FAIL_RETURN();
        }
        info.clipped = VK_TRUE;
        info.oldSwapchain = g_swapchain;

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        g_lastResult = vkCreateSwapchainKHR(g_device, &info, g_pMAlloc, &swapchain);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
        if (g_swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(g_device, g_swapchain, g_pMAlloc);
        g_swapchain = swapchain;
        g_screenSize.width = info.imageExtent.width;
        g_screenSize.height = info.imageExtent.height;
        g_screenSize.depth = 1;

        g_lastResult = vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_swapchainSize, nullptr);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
        g_pSwapchainImage = ALLOC(VkImage[g_swapchainSize]);
        vkGetSwapchainImagesKHR(g_device, g_swapchain, &g_swapchainSize, g_pSwapchainImage);

        g_viewport.x = 0.f;
        g_viewport.y = 0.f;
        g_viewport.width = static_cast<float>(info.imageExtent.width);
        g_viewport.height = static_cast<float>(info.imageExtent.height);
        g_viewport.minDepth = 0.f;
        g_viewport.maxDepth = 1.f;
        g_scissor.offset.x = 0;
        g_scissor.offset.y = 0;
        g_scissor.extent.width = info.imageExtent.width;
        g_scissor.extent.height = info.imageExtent.height;
    }
    if (depthBuffer)
    {
        VkImageCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = vkUtilGetDepthBufferFormat();
        info.extent = g_screenSize;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT;       
        g_lastResult = vkCreateImage(g_device, &info, g_pMAlloc, &g_depthImage);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
        VkMemoryAllocateInfo depthMem{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        if (!vkUtilGetImageMemoryInfo(g_depthImage, false, false, &depthMem))
            FAIL_RETURN();
        g_lastResult = vkAllocateMemory(g_device, &depthMem, g_pMAlloc, &g_depthImageMemory);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
        g_lastResult = vkBindImageMemory(g_device, g_depthImage, g_depthImageMemory, 0);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
    }
    if (g_dummyRenderPass == VK_NULL_HANDLE)
    {
        VkSubpassDependency spd[] =
        {
            {
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                0,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            },
            {
                0,
                VK_SUBPASS_EXTERNAL,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                0
            }
        };
        VkAttachmentDescription attd[] =
        {
            {
                0,
                vkUtilGetColorBufferFormat(),
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            },
            {
                0,
                vkUtilGetDepthBufferFormat(),
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            }
        };
        VkAttachmentReference color{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference depth{ 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
        VkSubpassDescription sp{ };
        sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sp.colorAttachmentCount = 1;
        sp.pColorAttachments = &color;
        sp.pDepthStencilAttachment = (depthBuffer) ? &depth : nullptr;
        VkRenderPassCreateInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        info.attachmentCount = (depthBuffer) ? 2 : 1;
        info.pAttachments = attd;
        info.subpassCount = 1;
        info.pSubpasses = &sp;
        info.dependencyCount = 2;
        info.pDependencies = spd;
        g_lastResult = vkCreateRenderPass(g_device, &info, g_pMAlloc, &g_dummyRenderPass);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
    }
    {
        VkImageViewCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
        info.image = g_depthImage;
        info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        info.format = VKUTIL_DEPTH_BUFFER_FORMAT;
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        info.subresourceRange.levelCount = 1;
        info.subresourceRange.layerCount = 1;
        if (depthBuffer)
        {
            g_lastResult = vkCreateImageView(g_device, &info, g_pMAlloc, &g_depthImageView);
            if (g_lastResult != VK_SUCCESS)
                FAIL_RETURN();
        }
        g_pSwapchainImageView = ALLOC(VkImageView[g_swapchainSize]);
        g_pSwapchainFramebuffer = ALLOC(VkFramebuffer[g_swapchainSize]);
        VkImageView pFBImage[] = { VK_NULL_HANDLE, g_depthImageView };
        VkFramebufferCreateInfo framebuff{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
        info.format = vkUtilGetColorBufferFormat();
        info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        framebuff.renderPass = g_dummyRenderPass;
        framebuff.attachmentCount = (depthBuffer) ? 2 : 1;
        framebuff.pAttachments = pFBImage;
        framebuff.width = g_screenSize.width;
        framebuff.height = g_screenSize.height;
        framebuff.layers = 1;
        for (uint32_t i = 0; i < g_swapchainSize; i++)
        {
            info.image = g_pSwapchainImage[i];
            if (g_lastResult != VK_SUCCESS)
            {
                g_pSwapchainImageView[i] = VK_NULL_HANDLE;
                g_pSwapchainFramebuffer[i] = VK_NULL_HANDLE;
            }
            else
            {
                g_lastResult = vkCreateImageView(g_device, &info, g_pMAlloc, &g_pSwapchainImageView[i]);
                if (g_lastResult == VK_SUCCESS)
                {
                    pFBImage[0] = g_pSwapchainImageView[i];
                    g_lastResult = vkCreateFramebuffer(g_device, &framebuff, g_pMAlloc, &g_pSwapchainFramebuffer[i]);
                }
            }
        }
    }
}

void vkUtilInitialize(const char* pAppName, VkAllocationCallbacks* pMAlloc, bool validation)
{
    g_pMAlloc = pMAlloc;
    const char* ppExtensions[]
    {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VKAPI_NATIVE_SURFACE_EXT_NAME
    };
    const char* ppLayers[]
    {
        "VK_LAYER_KHRONOS_validation"
    };
#if _DEBUG
#define LOAD(n) if (!LoadVkInstanceProc(g_instance, #n, n)) __debugbreak()
#else
#define LOAD(n) LoadVkInstanceProc(g_instance, #n, n)
#endif
    if (LoadVkLibrary())
    {
        LOAD(vkCreateInstance);
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
        LOAD(vkEnumerateInstanceVersion);
#endif
        VkApplicationInfo app{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        app.pApplicationName = pAppName;
        app.apiVersion = VKAPI_VERSION;
        VkInstanceCreateInfo info{ VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };
        info.pApplicationInfo = &app;
        if (validation)
        {
            info.enabledLayerCount = 1;
            info.ppEnabledLayerNames = ppLayers;
        }
        info.enabledExtensionCount = 2;
        info.ppEnabledExtensionNames = ppExtensions;
        g_lastResult = vkCreateInstance(&info, g_pMAlloc, &g_instance);
        if (g_lastResult == VK_SUCCESS)
        {
            LOAD(vkDestroyInstance);
            LOAD(vkDestroySurfaceKHR);
            LOAD(vkEnumeratePhysicalDevices);
            LOAD(vkGetPhysicalDeviceFeatures);
            LOAD(vkGetPhysicalDeviceFormatProperties);
            LOAD(vkGetPhysicalDeviceImageFormatProperties);
            LOAD(vkGetPhysicalDeviceProperties);
            LOAD(vkGetPhysicalDeviceQueueFamilyProperties);
            LOAD(vkGetPhysicalDeviceMemoryProperties);
            LOAD(vkGetInstanceProcAddr);
            LOAD(vkGetDeviceProcAddr);
            LOAD(vkCreateDevice);
            LOAD(vkDestroySurfaceKHR);
            LOAD(vkGetPhysicalDeviceSurfaceSupportKHR);
            LOAD(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
            LOAD(vkGetPhysicalDeviceSurfaceFormatsKHR);
            LOAD(vkGetPhysicalDeviceSurfacePresentModesKHR);
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
            LOAD(vkEnumeratePhysicalDeviceGroups);
            LOAD(vkGetPhysicalDeviceFeatures2);
            LOAD(vkGetPhysicalDeviceProperties2);
            LOAD(vkGetPhysicalDeviceFormatProperties2);
            LOAD(vkGetPhysicalDeviceImageFormatProperties2);
            LOAD(vkGetPhysicalDeviceQueueFamilyProperties2);
            LOAD(vkGetPhysicalDeviceMemoryProperties2);
            LOAD(vkGetPhysicalDeviceSparseImageFormatProperties2);
            LOAD(vkGetPhysicalDeviceExternalBufferProperties);
            LOAD(vkGetPhysicalDeviceExternalFenceProperties);
            LOAD(vkGetPhysicalDeviceExternalSemaphoreProperties);
#endif
        }
    }
#undef LOAD
}

bool vkUtilGetBufferMemoryInfo(VkBuffer buffer, bool cpuWrite, bool cpuRead, VkMemoryAllocateInfo* pOut)
{
    VkMemoryRequirements memReq;
    VkMemoryPropertyFlags flags = 0;
    vkGetBufferMemoryRequirements(g_device, buffer, &memReq);
    if (cpuWrite || cpuRead)
        flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (cpuRead)
        flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    if (flags == 0)
        flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    for (uint32_t i = 0; i < g_memProps.memoryTypeCount; i++)
    {
        const VkMemoryType& type = g_memProps.memoryTypes[i];
        if ((memReq.memoryTypeBits & (1 << i)) && ((type.propertyFlags & flags) == flags))
        {
            pOut->memoryTypeIndex = i;
            pOut->allocationSize = memReq.size;
            return true;
        }
    }
    FAIL_RETVAL(false)
}

bool vkUtilGetImageMemoryInfo(VkImage image, bool cpuWrite, bool cpuRead, VkMemoryAllocateInfo* pOut)
{
    VkMemoryRequirements memReq;
    VkMemoryPropertyFlags flags = 0;
    vkGetImageMemoryRequirements(g_device, image, &memReq);
    if (cpuWrite || cpuRead)
        flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (cpuRead)
        flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    if (flags == 0)
        flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    for (uint32_t i = 0; i < g_memProps.memoryTypeCount; i++)
    {
        const VkMemoryType& type = g_memProps.memoryTypes[i];
        if ((memReq.memoryTypeBits & (1 << i)) && ((type.propertyFlags & flags) == flags))
        {
            pOut->memoryTypeIndex = i;
            pOut->allocationSize = memReq.size;
            return true;
        }
    }
    FAIL_RETVAL(false)
}

void vkUtilCreateDevice(void* pWindowHandle, bool depthBuffer)

{
    if (g_lastResult != VK_SUCCESS)
        FAIL_RETURN();

    if (g_surface == VK_NULL_HANDLE)
    {
#if !defined(WITHOUT_GLFW)
        GLFWwindow* pWindow = static_cast<GLFWwindow*>(pWindowHandle);
        g_lastResult = glfwCreateWindowSurface(g_instance, pWindow, g_pMAlloc, &g_surface);
#else
#error
#endif
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
    }

    if (g_device == VK_NULL_HANDLE)
    {
        uint32_t primaryQueue = UINT32_MAX, secondaryQueue = UINT32_MAX;
        uint32_t primaryDevice = UINT32_MAX, secondaryDevice = UINT32_MAX;
        g_lastResult = vkEnumeratePhysicalDevices(g_instance, &g_numPhysicalDevices, nullptr);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
        g_pPhysicalDevice = ALLOC(VkPhysicalDevice[g_numPhysicalDevices]);
        g_lastResult = vkEnumeratePhysicalDevices(g_instance, &g_numPhysicalDevices, g_pPhysicalDevice);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN();
        g_pNumQueueFamilies = ALLOC(uint32_t[g_numPhysicalDevices]);
        g_ppQueueFamilyProperties = ALLOC(VkQueueFamilyProperties*[g_numPhysicalDevices]);
        for (uint32_t i = 0; (i < g_numPhysicalDevices) && ((primaryDevice == UINT32_MAX) || (secondaryDevice == UINT32_MAX)); i++)
        {
            uint32_t queueFamily = UINT32_MAX;
            g_pNumQueueFamilies[i] = 0;
            g_ppQueueFamilyProperties[i] = nullptr;
            vkGetPhysicalDeviceQueueFamilyProperties(g_pPhysicalDevice[i], &g_pNumQueueFamilies[i], nullptr);
            if (g_pNumQueueFamilies[i] == 0)
                continue;
            g_ppQueueFamilyProperties[i] = ALLOC(VkQueueFamilyProperties[g_pNumQueueFamilies[i]]);
            vkGetPhysicalDeviceQueueFamilyProperties(g_pPhysicalDevice[i], &g_pNumQueueFamilies[i], g_ppQueueFamilyProperties[i]);
            for (uint32_t j = 0; j < g_pNumQueueFamilies[i]; j++)
            {
                VkBool32 canPresent = VK_FALSE;
                VkQueueFlags flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
                VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(g_pPhysicalDevice[i], j, g_surface, &canPresent);
                if (result != VK_SUCCESS || canPresent != VK_TRUE)
                    continue;
                const VkQueueFamilyProperties& qf = g_ppQueueFamilyProperties[i][j];
                if ((qf.queueFlags & flags) == flags)
                {
                    queueFamily = j;
                    break;
                }
            }
            if (queueFamily != UINT32_MAX)
            {
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(g_pPhysicalDevice[i], &props);
                if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && primaryDevice == UINT32_MAX)
                {
                    primaryDevice = i;
                    primaryQueue = queueFamily;
                }
                else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && secondaryDevice == UINT32_MAX)
                {
                    secondaryDevice = i;
                    secondaryDevice = queueFamily;
                }
            }
        }
        g_physicalDevice = (primaryDevice != UINT32_MAX) ? primaryDevice : secondaryDevice;
        g_queueFamily = (primaryQueue != UINT32_MAX) ? primaryQueue : secondaryQueue;
        if (g_physicalDevice == UINT32_MAX || g_queueFamily == UINT32_MAX)
            FAIL_RETURN();
        vkGetPhysicalDeviceMemoryProperties(g_pPhysicalDevice[g_physicalDevice], &g_memProps);
        {
            const char* ppExtensions[]
            {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
            float queuePriority = 1.f;
            VkDeviceQueueCreateInfo queue{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queue.queueFamilyIndex = g_queueFamily;
            queue.queueCount = 1;
            queue.pQueuePriorities = &queuePriority;
            VkDeviceCreateInfo info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
            info.queueCreateInfoCount = 1;
            info.pQueueCreateInfos = &queue;
            info.enabledExtensionCount = 1;
            info.ppEnabledExtensionNames = ppExtensions;
            VkPhysicalDevice phd = g_pPhysicalDevice[g_physicalDevice];
            g_lastResult = vkCreateDevice(phd, &info, g_pMAlloc, &g_device);
            if (g_lastResult != VK_SUCCESS)
                FAIL_RETURN();
#if _DEBUG
#define LOAD(n) if (!LoadVkDeviceProc(g_device, #n, n)) __debugbreak()
#else
#define LOAD(n) LoadVkDeviceProc(g_device, #n, n)
#endif
            LOAD(vkDestroyDevice);
            LOAD(vkGetDeviceQueue);
            LOAD(vkQueueSubmit);
            LOAD(vkQueueWaitIdle);
            LOAD(vkDeviceWaitIdle);
            LOAD(vkAllocateMemory);
            LOAD(vkFreeMemory);
            LOAD(vkMapMemory);
            LOAD(vkUnmapMemory);
            LOAD(vkFlushMappedMemoryRanges);
            LOAD(vkInvalidateMappedMemoryRanges);
            LOAD(vkGetDeviceMemoryCommitment);
            LOAD(vkBindBufferMemory);
            LOAD(vkBindImageMemory);
            LOAD(vkGetBufferMemoryRequirements);
            LOAD(vkGetImageMemoryRequirements);
            LOAD(vkGetImageSparseMemoryRequirements);
            LOAD(vkQueueBindSparse);
            LOAD(vkCreateFence);
            LOAD(vkDestroyFence);
            LOAD(vkResetFences);
            LOAD(vkGetFenceStatus);
            LOAD(vkWaitForFences);
            LOAD(vkCreateSemaphore);
            LOAD(vkDestroySemaphore);
            LOAD(vkCreateEvent);
            LOAD(vkDestroyEvent);
            LOAD(vkGetEventStatus);
            LOAD(vkSetEvent);
            LOAD(vkResetEvent);
            LOAD(vkCreateQueryPool);
            LOAD(vkDestroyQueryPool);
            LOAD(vkGetQueryPoolResults);
            LOAD(vkCreateBuffer);
            LOAD(vkDestroyBuffer);
            LOAD(vkCreateBufferView);
            LOAD(vkDestroyBufferView);
            LOAD(vkCreateImage);
            LOAD(vkDestroyImage);
            LOAD(vkGetImageSubresourceLayout);
            LOAD(vkCreateImageView);
            LOAD(vkDestroyImageView);
            LOAD(vkCreateShaderModule);
            LOAD(vkDestroyShaderModule);
            LOAD(vkCreatePipelineCache);
            LOAD(vkDestroyPipelineCache);
            LOAD(vkGetPipelineCacheData);
            LOAD(vkMergePipelineCaches);
            LOAD(vkCreateGraphicsPipelines);
            LOAD(vkCreateComputePipelines);
            LOAD(vkDestroyPipeline);
            LOAD(vkCreatePipelineLayout);
            LOAD(vkDestroyPipelineLayout);
            LOAD(vkCreateSampler);
            LOAD(vkDestroySampler);
            LOAD(vkCreateDescriptorSetLayout);
            LOAD(vkDestroyDescriptorSetLayout);
            LOAD(vkCreateDescriptorPool);
            LOAD(vkDestroyDescriptorPool);
            LOAD(vkResetDescriptorPool);
            LOAD(vkAllocateDescriptorSets);
            LOAD(vkFreeDescriptorSets);
            LOAD(vkUpdateDescriptorSets);
            LOAD(vkCreateFramebuffer);
            LOAD(vkDestroyFramebuffer);
            LOAD(vkCreateRenderPass);
            LOAD(vkDestroyRenderPass);
            LOAD(vkGetRenderAreaGranularity);
            LOAD(vkCreateCommandPool);
            LOAD(vkDestroyCommandPool);
            LOAD(vkResetCommandPool);
            LOAD(vkAllocateCommandBuffers);
            LOAD(vkFreeCommandBuffers);
            LOAD(vkBeginCommandBuffer);
            LOAD(vkEndCommandBuffer);
            LOAD(vkResetCommandBuffer);
            LOAD(vkCmdBindPipeline);
            LOAD(vkCmdSetViewport);
            LOAD(vkCmdSetScissor);
            LOAD(vkCmdSetLineWidth);
            LOAD(vkCmdSetDepthBias);
            LOAD(vkCmdSetBlendConstants);
            LOAD(vkCmdSetDepthBounds);
            LOAD(vkCmdSetStencilCompareMask);
            LOAD(vkCmdSetStencilWriteMask);
            LOAD(vkCmdSetStencilReference);
            LOAD(vkCmdBindDescriptorSets);
            LOAD(vkCmdBindIndexBuffer);
            LOAD(vkCmdBindVertexBuffers);
            LOAD(vkCmdDraw);
            LOAD(vkCmdDrawIndexed);
            LOAD(vkCmdDrawIndirect);
            LOAD(vkCmdDrawIndexedIndirect);
            LOAD(vkCmdDispatch);
            LOAD(vkCmdDispatchIndirect);
            LOAD(vkCmdCopyBuffer);
            LOAD(vkCmdCopyImage);
            LOAD(vkCmdBlitImage);
            LOAD(vkCmdCopyBufferToImage);
            LOAD(vkCmdCopyImageToBuffer);
            LOAD(vkCmdUpdateBuffer);
            LOAD(vkCmdFillBuffer);
            LOAD(vkCmdClearColorImage);
            LOAD(vkCmdClearDepthStencilImage);
            LOAD(vkCmdClearAttachments);
            LOAD(vkCmdResolveImage);
            LOAD(vkCmdSetEvent);
            LOAD(vkCmdResetEvent);
            LOAD(vkCmdWaitEvents);
            LOAD(vkCmdPipelineBarrier);
            LOAD(vkCmdBeginQuery);
            LOAD(vkCmdEndQuery);
            LOAD(vkCmdResetQueryPool);
            LOAD(vkCmdWriteTimestamp);
            LOAD(vkCmdCopyQueryPoolResults);
            LOAD(vkCmdPushConstants);
            LOAD(vkCmdBeginRenderPass);
            LOAD(vkCmdNextSubpass);
            LOAD(vkCmdEndRenderPass);
            LOAD(vkCmdExecuteCommands);
            LOAD(vkCreateSwapchainKHR);
            LOAD(vkDestroySwapchainKHR);
            LOAD(vkGetSwapchainImagesKHR);
            LOAD(vkAcquireNextImageKHR);
            LOAD(vkQueuePresentKHR);
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
            LOAD(vkBindBufferMemory2);
            LOAD(vkBindImageMemory2);
            LOAD(vkGetDeviceGroupPeerMemoryFeatures);
            LOAD(vkCmdSetDeviceMask);
            LOAD(vkCmdDispatchBase);
            LOAD(vkGetImageMemoryRequirements2);
            LOAD(vkGetBufferMemoryRequirements2);
            LOAD(vkGetImageSparseMemoryRequirements2);
            LOAD(vkTrimCommandPool);
            LOAD(vkGetDeviceQueue2);
            LOAD(vkCreateSamplerYcbcrConversion);
            LOAD(vkDestroySamplerYcbcrConversion);
            LOAD(vkCreateDescriptorUpdateTemplate);
            LOAD(vkDestroyDescriptorUpdateTemplate);
            LOAD(vkUpdateDescriptorSetWithTemplate);
            LOAD(vkGetDescriptorSetLayoutSupport);
            LOAD(vkGetDeviceGroupPresentCapabilitiesKHR);
            LOAD(vkGetDeviceGroupSurfacePresentModesKHR);
            LOAD(vkGetPhysicalDevicePresentRectanglesKHR);
            LOAD(vkAcquireNextImage2KHR);
#endif
#if defined(VK_API_VERSION_1_2) && (VKAPI_VERSION >= VK_API_VERSION_1_2)
            LOAD(vkCmdDrawIndirectCount);
            LOAD(vkCmdDrawIndexedIndirectCount);
            LOAD(vkCreateRenderPass2);
            LOAD(vkCmdBeginRenderPass2);
            LOAD(vkCmdNextSubpass2);
            LOAD(vkCmdEndRenderPass2);
            LOAD(vkResetQueryPool);
            LOAD(vkGetSemaphoreCounterValue);
            LOAD(vkWaitSemaphores);
            LOAD(vkSignalSemaphore);
            LOAD(vkGetBufferDeviceAddress);
            LOAD(vkGetBufferOpaqueCaptureAddress);
            LOAD(vkGetDeviceMemoryOpaqueCaptureAddress);
#endif
#undef LOAD
        }
        vkGetDeviceQueue(g_device, g_queueFamily, 0, &g_queue);
    }        

    if (g_commandPool == VK_NULL_HANDLE)
    {
        {
            VkCommandPoolCreateInfo info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            info.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            info.queueFamilyIndex = g_queueFamily;
            g_lastResult = vkCreateCommandPool(g_device, &info, g_pMAlloc, &g_commandPool);
            if (g_lastResult != VK_SUCCESS)
                FAIL_RETURN();
        }
        {
            VkCommandBuffer pCBuff[NUM_FRAME_CONTEXTS];
            g_pFrameData = ALLOC(FrameData[NUM_FRAME_CONTEXTS]);
            VkCommandBufferAllocateInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            info.commandPool = g_commandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = NUM_FRAME_CONTEXTS;
            g_lastResult = vkAllocateCommandBuffers(g_device, &info, pCBuff);
            if (g_lastResult != VK_SUCCESS)
                FAIL_RETURN();
            VkFenceCreateInfo fence{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
            fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VkSemaphoreCreateInfo semaphore{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            for (uint32_t i = 0; i < NUM_FRAME_CONTEXTS; i++)
            {
                FrameData& frameData = g_pFrameData[i];
                frameData.pNext = &g_pFrameData[(i + 1) % NUM_FRAME_CONTEXTS];
                g_lastResult = vkCreateFence(g_device, &fence, g_pMAlloc, &frameData.fence);
                if (g_lastResult != VK_SUCCESS)
                    FAIL_RETURN();
                g_lastResult = vkCreateSemaphore(g_device, &semaphore, g_pMAlloc, &frameData.imageReady);
                if (g_lastResult != VK_SUCCESS)
                    FAIL_RETURN();
                g_lastResult = vkCreateSemaphore(g_device, &semaphore, g_pMAlloc, &frameData.submitDone);
                if (g_lastResult != VK_SUCCESS)
                    FAIL_RETURN();
                frameData.commandBuffer = pCBuff[i];
            }
            g_pCurrentFrame = g_pFrameData;
        }
    }

    CreateSwapchainResources(depthBuffer);
    if (g_lastResult != VK_SUCCESS)
        FAIL_RETURN();
}

void vkUtilSetError(VkResult result, const char*)
{
    g_lastResult = result;
}

VkCommandBuffer vkUtilGetCommandBuffer()
{
    return g_pCurrentFrame->commandBuffer;
}

VkFramebuffer vkUtilGetFramebuffer()
{
    if (g_currentImage == UINT32_MAX)
    {
        VkSemaphore imageReady = g_pCurrentFrame->imageReady;
        g_lastResult = vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX, imageReady, VK_NULL_HANDLE, &g_currentImage);
        if (g_lastResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            g_lastResult = VK_SUCCESS;
            CreateSwapchainResources(g_depthImage != VK_NULL_HANDLE);
            if (g_lastResult != VK_SUCCESS)
                FAIL_RETVAL(VK_NULL_HANDLE);
            g_lastResult = vkAcquireNextImageKHR(g_device, g_swapchain, UINT64_MAX, imageReady, VK_NULL_HANDLE, &g_currentImage);
            if (g_lastResult != VK_SUCCESS)
                FAIL_RETVAL(VK_NULL_HANDLE);
        }
        else if (g_lastResult != VK_SUCCESS)
            FAIL_RETVAL(VK_NULL_HANDLE);
    }
    return g_pSwapchainFramebuffer[g_currentImage];
}

VkFormat vkUtilGetColorBufferFormat()
{
    return g_colorFormat;
}

VkFormat vkUtilGetDepthBufferFormat()
{
    return VKUTIL_DEPTH_BUFFER_FORMAT;
}

uint32_t vkUtilGetViewWidth()
{
    return g_screenSize.width;
}

uint32_t vkUtilGetViewHeight()
{
    return g_screenSize.height;
}

VkDevice vkUtilGetDevice()
{
    return g_device;
}

bool vkUtilHasError()
{
    return (g_lastResult != VK_SUCCESS);
}

void vkUtilBeginFrame()
{
    const FrameData& frameData = *g_pCurrentFrame;
    VKCHK_RTRN(vkWaitForFences(g_device, 1, &frameData.fence, VK_TRUE, UINT64_MAX));
    VKCHK_RTRN(vkResetFences(g_device, 1, &frameData.fence));
    g_lastResult = vkResetCommandBuffer(frameData.commandBuffer, 0);
    if (g_lastResult != VK_SUCCESS)
        FAIL_RETURN();
    VkCommandBufferBeginInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    g_lastResult = vkBeginCommandBuffer(frameData.commandBuffer, &info);
    if (g_lastResult != VK_SUCCESS)
        FAIL_RETURN();
}

void vkUtilEndFrame()
{
    g_lastResult = vkEndCommandBuffer(g_pCurrentFrame->commandBuffer);
    if (g_lastResult != VK_SUCCESS)
        FAIL_RETURN()
    {
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        info.waitSemaphoreCount = (g_currentImage < g_swapchainSize) ? 1 : 0;
        info.pWaitSemaphores = &g_pCurrentFrame->imageReady;
        info.pWaitDstStageMask = &waitStage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &g_pCurrentFrame->commandBuffer;
        info.signalSemaphoreCount = (g_currentImage < g_swapchainSize) ? 1 : 0;
        info.pSignalSemaphores = &g_pCurrentFrame->submitDone;
        g_lastResult = vkQueueSubmit(g_queue, 1, &info, g_pCurrentFrame->fence);
        if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN()
    }
    if (g_currentImage < g_swapchainSize)
    {
        VkPresentInfoKHR info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &g_pCurrentFrame->submitDone;
        info.swapchainCount = 1;
        info.pSwapchains = &g_swapchain;
        info.pImageIndices = &g_currentImage;
        g_lastResult = vkQueuePresentKHR(g_queue, &info);
        g_currentImage = UINT32_MAX;
        if (g_lastResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            g_lastResult = VK_SUCCESS;
            return;
        }
        else if (g_lastResult != VK_SUCCESS)
            FAIL_RETURN()
    }
    g_pCurrentFrame = g_pCurrentFrame->pNext;
}

void vkUtilFinalize()
{
    vkDeviceWaitIdle(g_device);
    DestroySwapchainResources(true);
    if (g_dummyRenderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(g_device, g_dummyRenderPass, g_pMAlloc);
    for (uint32_t i = 0; i < NUM_FRAME_CONTEXTS; i++)
    {
        FrameData& frameData = g_pFrameData[i];
        if (frameData.fence != VK_NULL_HANDLE)
            vkDestroyFence(g_device, frameData.fence, g_pMAlloc);
        if (frameData.imageReady != VK_NULL_HANDLE)
            vkDestroySemaphore(g_device, frameData.imageReady, g_pMAlloc);
        if (frameData.submitDone != VK_NULL_HANDLE)
            vkDestroySemaphore(g_device, frameData.submitDone, g_pMAlloc);
        if (frameData.commandBuffer != VK_NULL_HANDLE)
            vkFreeCommandBuffers(g_device, g_commandPool, 1, &frameData.commandBuffer);
    }
    g_pCurrentFrame = nullptr;
    FREE(g_pFrameData);    
    if (g_commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(g_device, g_commandPool, g_pMAlloc);
        g_commandPool = VK_NULL_HANDLE;
    }
    if (g_device != VK_NULL_HANDLE)
    {
        vkDestroyDevice(g_device, g_pMAlloc);
        g_device = VK_NULL_HANDLE;
    }
    if (g_surface != VK_NULL_HANDLE)
    {
        vkDestroySurfaceKHR(g_instance, g_surface, g_pMAlloc);
        g_surface = VK_NULL_HANDLE;
    }
    if (g_instance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(g_instance, g_pMAlloc);
        g_instance = VK_NULL_HANDLE;
    }
    for (uint32_t i = 0; i < g_numPhysicalDevices; i++)
        FREE(g_ppQueueFamilyProperties[i]);
    FREE(g_ppQueueFamilyProperties);
    FREE(g_pNumQueueFamilies);
    FREE(g_pPhysicalDevice);
}

/* VK_VERSION_1_0 */
PFN_vkCreateInstance vkCreateInstance = nullptr;
PFN_vkDestroyInstance vkDestroyInstance = nullptr;
PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices = nullptr;
PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures = nullptr;
PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties = nullptr;
PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties = nullptr;
PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties = nullptr;
PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = nullptr;
PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = nullptr;
PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = nullptr;
PFN_vkCreateDevice vkCreateDevice = nullptr;
PFN_vkDestroyDevice vkDestroyDevice = nullptr;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = nullptr;
PFN_vkEnumerateDeviceExtensionProperties vkEnumerateDeviceExtensionProperties = nullptr;
PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties = nullptr;
PFN_vkEnumerateDeviceLayerProperties vkEnumerateDeviceLayerProperties = nullptr;
PFN_vkGetDeviceQueue vkGetDeviceQueue = nullptr;
PFN_vkQueueSubmit vkQueueSubmit = nullptr;
PFN_vkQueueWaitIdle vkQueueWaitIdle = nullptr;
PFN_vkDeviceWaitIdle vkDeviceWaitIdle = nullptr;
PFN_vkAllocateMemory vkAllocateMemory = nullptr;
PFN_vkFreeMemory vkFreeMemory = nullptr;
PFN_vkMapMemory vkMapMemory = nullptr;
PFN_vkUnmapMemory vkUnmapMemory = nullptr;
PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges = nullptr;
PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges = nullptr;
PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment = nullptr;
PFN_vkBindBufferMemory vkBindBufferMemory = nullptr;
PFN_vkBindImageMemory vkBindImageMemory = nullptr;
PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = nullptr;
PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = nullptr;
PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements = nullptr;
PFN_vkQueueBindSparse vkQueueBindSparse = nullptr;
PFN_vkCreateFence vkCreateFence = nullptr;
PFN_vkDestroyFence vkDestroyFence = nullptr;
PFN_vkResetFences vkResetFences = nullptr;
PFN_vkGetFenceStatus vkGetFenceStatus = nullptr;
PFN_vkWaitForFences vkWaitForFences = nullptr;
PFN_vkCreateSemaphore vkCreateSemaphore = nullptr;
PFN_vkDestroySemaphore vkDestroySemaphore = nullptr;
PFN_vkCreateEvent vkCreateEvent = nullptr;
PFN_vkDestroyEvent vkDestroyEvent = nullptr;
PFN_vkGetEventStatus vkGetEventStatus = nullptr;
PFN_vkSetEvent vkSetEvent = nullptr;
PFN_vkResetEvent vkResetEvent = nullptr;
PFN_vkCreateQueryPool vkCreateQueryPool = nullptr;
PFN_vkDestroyQueryPool vkDestroyQueryPool = nullptr;
PFN_vkGetQueryPoolResults vkGetQueryPoolResults = nullptr;
PFN_vkCreateBuffer vkCreateBuffer = nullptr;
PFN_vkDestroyBuffer vkDestroyBuffer = nullptr;
PFN_vkCreateBufferView vkCreateBufferView = nullptr;
PFN_vkDestroyBufferView vkDestroyBufferView = nullptr;
PFN_vkCreateImage vkCreateImage = nullptr;
PFN_vkDestroyImage vkDestroyImage = nullptr;
PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout = nullptr;
PFN_vkCreateImageView vkCreateImageView = nullptr;
PFN_vkDestroyImageView vkDestroyImageView = nullptr;
PFN_vkCreateShaderModule vkCreateShaderModule = nullptr;
PFN_vkDestroyShaderModule vkDestroyShaderModule = nullptr;
PFN_vkCreatePipelineCache vkCreatePipelineCache = nullptr;
PFN_vkDestroyPipelineCache vkDestroyPipelineCache = nullptr;
PFN_vkGetPipelineCacheData vkGetPipelineCacheData = nullptr;
PFN_vkMergePipelineCaches vkMergePipelineCaches = nullptr;
PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines = nullptr;
PFN_vkCreateComputePipelines vkCreateComputePipelines = nullptr;
PFN_vkDestroyPipeline vkDestroyPipeline = nullptr;
PFN_vkCreatePipelineLayout vkCreatePipelineLayout = nullptr;
PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout = nullptr;
PFN_vkCreateSampler vkCreateSampler = nullptr;
PFN_vkDestroySampler vkDestroySampler = nullptr;
PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout = nullptr;
PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout = nullptr;
PFN_vkCreateDescriptorPool vkCreateDescriptorPool = nullptr;
PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool = nullptr;
PFN_vkResetDescriptorPool vkResetDescriptorPool = nullptr;
PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets = nullptr;
PFN_vkFreeDescriptorSets vkFreeDescriptorSets = nullptr;
PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets = nullptr;
PFN_vkCreateFramebuffer vkCreateFramebuffer = nullptr;
PFN_vkDestroyFramebuffer vkDestroyFramebuffer = nullptr;
PFN_vkCreateRenderPass vkCreateRenderPass = nullptr;
PFN_vkDestroyRenderPass vkDestroyRenderPass = nullptr;
PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity = nullptr;
PFN_vkCreateCommandPool vkCreateCommandPool = nullptr;
PFN_vkDestroyCommandPool vkDestroyCommandPool = nullptr;
PFN_vkResetCommandPool vkResetCommandPool = nullptr;
PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers = nullptr;
PFN_vkFreeCommandBuffers vkFreeCommandBuffers = nullptr;
PFN_vkBeginCommandBuffer vkBeginCommandBuffer = nullptr;
PFN_vkEndCommandBuffer vkEndCommandBuffer = nullptr;
PFN_vkResetCommandBuffer vkResetCommandBuffer = nullptr;
PFN_vkCmdBindPipeline vkCmdBindPipeline = nullptr;
PFN_vkCmdSetViewport vkCmdSetViewport = nullptr;
PFN_vkCmdSetScissor vkCmdSetScissor = nullptr;
PFN_vkCmdSetLineWidth vkCmdSetLineWidth = nullptr;
PFN_vkCmdSetDepthBias vkCmdSetDepthBias = nullptr;
PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants = nullptr;
PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds = nullptr;
PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask = nullptr;
PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask = nullptr;
PFN_vkCmdSetStencilReference vkCmdSetStencilReference = nullptr;
PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets = nullptr;
PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer = nullptr;
PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers = nullptr;
PFN_vkCmdDraw vkCmdDraw = nullptr;
PFN_vkCmdDrawIndexed vkCmdDrawIndexed = nullptr;
PFN_vkCmdDrawIndirect vkCmdDrawIndirect = nullptr;
PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect = nullptr;
PFN_vkCmdDispatch vkCmdDispatch = nullptr;
PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect = nullptr;
PFN_vkCmdCopyBuffer vkCmdCopyBuffer = nullptr;
PFN_vkCmdCopyImage vkCmdCopyImage = nullptr;
PFN_vkCmdBlitImage vkCmdBlitImage = nullptr;
PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage = nullptr;
PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer = nullptr;
PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer = nullptr;
PFN_vkCmdFillBuffer vkCmdFillBuffer = nullptr;
PFN_vkCmdClearColorImage vkCmdClearColorImage = nullptr;
PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage = nullptr;
PFN_vkCmdClearAttachments vkCmdClearAttachments = nullptr;
PFN_vkCmdResolveImage vkCmdResolveImage = nullptr;
PFN_vkCmdSetEvent vkCmdSetEvent = nullptr;
PFN_vkCmdResetEvent vkCmdResetEvent = nullptr;
PFN_vkCmdWaitEvents vkCmdWaitEvents = nullptr;
PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier = nullptr;
PFN_vkCmdBeginQuery vkCmdBeginQuery = nullptr;
PFN_vkCmdEndQuery vkCmdEndQuery = nullptr;
PFN_vkCmdResetQueryPool vkCmdResetQueryPool = nullptr;
PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp = nullptr;
PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults = nullptr;
PFN_vkCmdPushConstants vkCmdPushConstants = nullptr;
PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass = nullptr;
PFN_vkCmdNextSubpass vkCmdNextSubpass = nullptr;
PFN_vkCmdEndRenderPass vkCmdEndRenderPass = nullptr;
PFN_vkCmdExecuteCommands vkCmdExecuteCommands = nullptr;
/* VK_VERSION_1_0 */

/* VK_VERSION_1_1 */
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion = nullptr;
PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups = nullptr;
PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2 = nullptr;
PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2 = nullptr;
PFN_vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2 = nullptr;
PFN_vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2 = nullptr;
PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2 = nullptr;
PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2 = nullptr;
PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2 = nullptr;
PFN_vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferProperties = nullptr;
PFN_vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFenceProperties = nullptr;
PFN_vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphoreProperties = nullptr;
PFN_vkBindBufferMemory2 vkBindBufferMemory2 = nullptr;
PFN_vkBindImageMemory2 vkBindImageMemory2 = nullptr;
PFN_vkGetDeviceGroupPeerMemoryFeatures vkGetDeviceGroupPeerMemoryFeatures = nullptr;
PFN_vkCmdSetDeviceMask vkCmdSetDeviceMask = nullptr;
PFN_vkCmdDispatchBase vkCmdDispatchBase = nullptr;
PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2 = nullptr;
PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2 = nullptr;
PFN_vkGetImageSparseMemoryRequirements2 vkGetImageSparseMemoryRequirements2 = nullptr;
PFN_vkTrimCommandPool vkTrimCommandPool = nullptr;
PFN_vkGetDeviceQueue2 vkGetDeviceQueue2 = nullptr;
PFN_vkCreateSamplerYcbcrConversion vkCreateSamplerYcbcrConversion = nullptr;
PFN_vkDestroySamplerYcbcrConversion vkDestroySamplerYcbcrConversion = nullptr;
PFN_vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplate = nullptr;
PFN_vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplate = nullptr;
PFN_vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplate = nullptr;
PFN_vkGetDescriptorSetLayoutSupport vkGetDescriptorSetLayoutSupport = nullptr;
#endif
/* VK_VERSION_1_1 */


/* VK_VERSION_1_2 */
#if defined(VK_API_VERSION_1_2) && (VKAPI_VERSION >= VK_API_VERSION_1_2)
PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount = nullptr;
PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount = nullptr;
PFN_vkCreateRenderPass2 vkCreateRenderPass2 = nullptr;
PFN_vkCmdBeginRenderPass2 vkCmdBeginRenderPass2 = nullptr;
PFN_vkCmdNextSubpass2 vkCmdNextSubpass2 = nullptr;
PFN_vkCmdEndRenderPass2 vkCmdEndRenderPass2 = nullptr;
PFN_vkResetQueryPool vkResetQueryPool = nullptr;
PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue = nullptr;
PFN_vkWaitSemaphores vkWaitSemaphores = nullptr;
PFN_vkSignalSemaphore vkSignalSemaphore = nullptr;
PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress = nullptr;
PFN_vkGetBufferOpaqueCaptureAddress vkGetBufferOpaqueCaptureAddress = nullptr;
PFN_vkGetDeviceMemoryOpaqueCaptureAddress vkGetDeviceMemoryOpaqueCaptureAddress = nullptr;
#endif
/* VK_VERSION_1_2 */

/* VK_KHR_surface */
PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = nullptr;
/* VK_KHR_surface */

/* VK_KHR_swapchain */
PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR = nullptr;
PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR = nullptr;
PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR = nullptr;
PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR = nullptr;
PFN_vkQueuePresentKHR vkQueuePresentKHR = nullptr;
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
PFN_vkGetDeviceGroupPresentCapabilitiesKHR vkGetDeviceGroupPresentCapabilitiesKHR = nullptr;
PFN_vkGetDeviceGroupSurfacePresentModesKHR vkGetDeviceGroupSurfacePresentModesKHR = nullptr;
PFN_vkGetPhysicalDevicePresentRectanglesKHR vkGetPhysicalDevicePresentRectanglesKHR = nullptr;
PFN_vkAcquireNextImage2KHR vkAcquireNextImage2KHR = nullptr;
#endif
/* VK_KHR_swapchain */
