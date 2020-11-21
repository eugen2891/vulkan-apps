#include "vulkanapi.h"

#if !defined(WITHOUT_GLFW)
#include <GLFW/glfw3.h>
#else
#error Running without GLFW is not supported
#endif

#if defined(_WIN32)
#define VKAPI_NATIVE_SURFACE_EXT_NAME VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#endif

#include <new>
#define ALLOC(e) new (std::nothrow) e
#define FREE(p) delete[] p
#define SWAP(a,b) { auto t = a; a = b; b = t; }

#define NUM_FRAME_CONTEXTS 2

#define COLOR_BUFFER_FORMAT VK_FORMAT_B8G8R8A8_UNORM
#define DEPTH_BUFFER_FORMAT VK_FORMAT_D32_SFLOAT_S8_UINT

/*

#if defined(__VS)

float4 VSMain(uint input : SV_VERTEXID) : SV_POSITION
{
  float x = (input == 1) ? 3 : -1;
  float y = (input >  1) ? 3 : -1;
  return float4(x, y, 0, 1);
}

#elif defined(__FS)

[[vk::input_attachment_index(0)]]
SubpassInput inputAttachment;

float4 FSMain() : SV_TARGET
{
  return inputAttachment.SubpassLoad();
}

#endif

*/

static const uint32_t FINAL_COLOR_OUTPUT_VS[] =
{
    0x07230203, 0x00010000, 0x000E0000, 0x0000001B,
    0x00000000, 0x00020011, 0x00000001, 0x0003000E,
    0x00000000, 0x00000001, 0x0007000F, 0x00000000,
    0x00000001, 0x614D5356, 0x00006E69, 0x00000002,
    0x00000003, 0x00030003, 0x00000005, 0x00000258,
    0x00040005, 0x00000001, 0x614D5356, 0x00006E69,
    0x00040047, 0x00000002, 0x0000000B, 0x0000002A,
    0x00040047, 0x00000003, 0x0000000B, 0x00000000,
    0x00040015, 0x00000004, 0x00000020, 0x00000000,
    0x0004002B, 0x00000004, 0x00000005, 0x00000001,
    0x00040015, 0x00000006, 0x00000020, 0x00000001,
    0x0004002B, 0x00000006, 0x00000007, 0x00000003,
    0x00030016, 0x00000008, 0x00000020, 0x0004002B,
    0x00000008, 0x00000009, 0x00000000, 0x0004002B,
    0x00000008, 0x0000000A, 0x3F800000, 0x00040020,
    0x0000000B, 0x00000001, 0x00000004, 0x00040017,
    0x0000000C, 0x00000008, 0x00000004, 0x00040020,
    0x0000000D, 0x00000003, 0x0000000C, 0x00020013,
    0x0000000E, 0x00030021, 0x0000000F, 0x0000000E,
    0x00020014, 0x00000010, 0x0004003B, 0x0000000B,
    0x00000002, 0x00000001, 0x0004003B, 0x0000000D,
    0x00000003, 0x00000003, 0x0004002B, 0x00000006,
    0x00000011, 0xFFFFFFFF, 0x00050036, 0x0000000E,
    0x00000001, 0x00000000, 0x0000000F, 0x000200F8,
    0x00000012, 0x0004003D, 0x00000004, 0x00000013,
    0x00000002, 0x000500AA, 0x00000010, 0x00000014,
    0x00000013, 0x00000005, 0x000600A9, 0x00000006,
    0x00000015, 0x00000014, 0x00000007, 0x00000011,
    0x0004006F, 0x00000008, 0x00000016, 0x00000015,
    0x000500AC, 0x00000010, 0x00000017, 0x00000013,
    0x00000005, 0x000600A9, 0x00000006, 0x00000018,
    0x00000017, 0x00000007, 0x00000011, 0x0004006F,
    0x00000008, 0x00000019, 0x00000018, 0x00070050,
    0x0000000C, 0x0000001A, 0x00000016, 0x00000019,
    0x00000009, 0x0000000A, 0x0003003E, 0x00000003,
    0x0000001A, 0x000100FD, 0x00010038
};

static const uint32_t FINAL_COLOR_OUTPUT_FS[] =
{
    0x07230203, 0x00010000, 0x000E0000, 0x00000012,
    0x00000000, 0x00020011, 0x00000001, 0x00020011,
    0x00000028, 0x0003000E, 0x00000000, 0x00000001,
    0x0006000F, 0x00000004, 0x00000001, 0x614D5346,
    0x00006E69, 0x00000002, 0x00030010, 0x00000001,
    0x00000007, 0x00030003, 0x00000005, 0x00000258,
    0x00070005, 0x00000003, 0x65707974, 0x6275732E,
    0x73736170, 0x616D692E, 0x00006567, 0x00060005,
    0x00000004, 0x75706E69, 0x74744174, 0x6D686361,
    0x00746E65, 0x00070005, 0x00000002, 0x2E74756F,
    0x2E726176, 0x545F5653, 0x45475241, 0x00000054,
    0x00040005, 0x00000001, 0x614D5346, 0x00006E69,
    0x00040047, 0x00000004, 0x0000002B, 0x00000000,
    0x00040047, 0x00000002, 0x0000001E, 0x00000000,
    0x00040047, 0x00000004, 0x00000022, 0x00000000,
    0x00040047, 0x00000004, 0x00000021, 0x00000000,
    0x00040015, 0x00000005, 0x00000020, 0x00000001,
    0x0004002B, 0x00000005, 0x00000006, 0x00000000,
    0x00040017, 0x00000007, 0x00000005, 0x00000002,
    0x0005002C, 0x00000007, 0x00000008, 0x00000006,
    0x00000006, 0x00030016, 0x00000009, 0x00000020,
    0x00090019, 0x00000003, 0x00000009, 0x00000006,
    0x00000002, 0x00000000, 0x00000000, 0x00000002,
    0x00000000, 0x00040020, 0x0000000A, 0x00000000,
    0x00000003, 0x00040017, 0x0000000B, 0x00000009,
    0x00000004, 0x00040020, 0x0000000C, 0x00000003,
    0x0000000B, 0x00020013, 0x0000000D, 0x00030021,
    0x0000000E, 0x0000000D, 0x0004003B, 0x0000000A,
    0x00000004, 0x00000000, 0x0004003B, 0x0000000C,
    0x00000002, 0x00000003, 0x00050036, 0x0000000D,
    0x00000001, 0x00000000, 0x0000000E, 0x000200F8,
    0x0000000F, 0x0004003D, 0x00000003, 0x00000010,
    0x00000004, 0x00060062, 0x0000000B, 0x00000011,
    0x00000010, 0x00000008, 0x00000000, 0x0003003E,
    0x00000002, 0x00000011, 0x000100FD, 0x00010038
};

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

#define LOAD_VK_INSTANCE_PROC(i,n) if (!LoadVkInstanceProc(i, #n, n)) return false
#define LOAD_VK_DEVICE_PROC(d,n)   if (!LoadVkDeviceProc(d, #n, n)) return false

VulkanAPI* VulkanAPI::m_pVkApi = nullptr;

VulkanAPI::VulkanAPI(const char* pAppName, bool validation)
    : m_pMAlloc(nullptr)
    , m_pPhysicalDevice(nullptr)
    , m_ppQueueFamilyProperties(nullptr)
    , m_pNumQueueFamilies(nullptr)
    , m_instance(VK_NULL_HANDLE)
    , m_device(VK_NULL_HANDLE)
    , m_surface(VK_NULL_HANDLE)
    , m_swapchain(VK_NULL_HANDLE)
    , m_queue(VK_NULL_HANDLE)
    , m_pSwapchainImage(nullptr)
    , m_pSwapchainImageView(nullptr)
    , m_pSwapchainFramebuffer(nullptr)
    , m_colorImage(VK_NULL_HANDLE)
    , m_depthImage(VK_NULL_HANDLE)
    , m_colorImageMemory(VK_NULL_HANDLE)
    , m_depthImageMemory(VK_NULL_HANDLE)
    , m_colorImageView(VK_NULL_HANDLE)
    , m_depthImageView(VK_NULL_HANDLE)
    , m_descriptorPool(VK_NULL_HANDLE)
    , m_descriptorSetLayout(VK_NULL_HANDLE)
    , m_finalColorDescriptorSet(VK_NULL_HANDLE)
    , m_finalColorVS(VK_NULL_HANDLE)
    , m_finalColorFS(VK_NULL_HANDLE)
    , m_finalColorPass(VK_NULL_HANDLE)
    , m_finalColorPipeline(VK_NULL_HANDLE)
    , m_finalColorPipelineLayout(VK_NULL_HANDLE)
    , m_commandPool(VK_NULL_HANDLE)
    , m_screenSize({ 0, 0, 0 })
    , m_numPhysicalDevices(0)
    , m_physicalDevice(0)
    , m_queueFamily(UINT32_MAX)
    , m_swapchainSize(0)
    , m_lastResult(VK_ERROR_INITIALIZATION_FAILED)
    , m_clearColor({})
    , m_swapchainReset(false)
{
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
#define LOAD(n) if (!LoadVkInstanceProc(m_instance, #n, n)) __debugbreak()
#else
#define LOAD(n) LoadVkInstanceProc(m_instance, #n, n)
#endif
    if (LoadVkLibrary())
    {
        LOAD(vkCreateInstance);
#if (VKAPI_VERSION >= VK_API_VERSION_1_1)
        LOAD(vkEnumerateInstanceVersion);
#endif
        VkApplicationInfo app{ VK_STRUCTURE_TYPE_APPLICATION_INFO };
        app.pApplicationName = pAppName;
        app.pEngineName = "VulkanAPIContext";
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
        m_lastResult = vkCreateInstance(&info, m_pMAlloc, &m_instance);
        if (m_lastResult == VK_SUCCESS)
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
            m_pVkApi = this;
        }
    }    
#undef LOAD
}    

VulkanAPI::~VulkanAPI()
{
    vkDeviceWaitIdle(m_device);
    if (m_finalColorPipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(m_device, m_finalColorPipeline, m_pMAlloc);
    if (m_finalColorPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(m_device, m_finalColorPipelineLayout, m_pMAlloc);
    if (m_finalColorVS != VK_NULL_HANDLE)
        vkDestroyShaderModule(m_device, m_finalColorVS, m_pMAlloc);
    if (m_finalColorFS != VK_NULL_HANDLE)
        vkDestroyShaderModule(m_device, m_finalColorFS, m_pMAlloc);
    if (m_descriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, m_pMAlloc);
    if (m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(m_device, m_descriptorPool, m_pMAlloc);
    DestroySwapchainResources();
    m_frameData.Reset();
    for (uint32_t i = 0; i < m_swapchainSize; i++)
    {
        if (m_frameData.Get()->fence != VK_NULL_HANDLE)
            vkDestroyFence(m_device, m_frameData.Get()->fence, m_pMAlloc);
        if (m_frameData.Get()->imageReady != VK_NULL_HANDLE)
            vkDestroySemaphore(m_device, m_frameData.Get()->imageReady, m_pMAlloc);
        if (m_frameData.Get()->submitDone != VK_NULL_HANDLE)
            vkDestroySemaphore(m_device, m_frameData.Get()->submitDone, m_pMAlloc);
        if (m_frameData.Get()->commandBuffer != VK_NULL_HANDLE)
            vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_frameData.Get()->commandBuffer);
        m_frameData.Next();
    }
    if (m_finalColorPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(m_device, m_finalColorPass, m_pMAlloc);
    if (m_commandPool != VK_NULL_HANDLE)
        vkDestroyCommandPool(m_device, m_commandPool, m_pMAlloc);
    if (m_swapchain != VK_NULL_HANDLE)
        vkDestroySwapchainKHR(m_device, m_swapchain, m_pMAlloc);
    if (m_device != VK_NULL_HANDLE)
        vkDestroyDevice(m_device, m_pMAlloc);
    if (m_surface != VK_NULL_HANDLE)
        vkDestroySurfaceKHR(m_instance, m_surface, m_pMAlloc);
    if (m_instance != VK_NULL_HANDLE)
        vkDestroyInstance(m_instance, m_pMAlloc);
    for (uint32_t i = 0; i < m_numPhysicalDevices; i++)
        FREE(m_ppQueueFamilyProperties[i]);
    FREE(m_ppQueueFamilyProperties);
    FREE(m_pNumQueueFamilies);
    FREE(m_pPhysicalDevice);
    m_pVkApi = nullptr;
}

bool VulkanAPI::GetAllocInfo(VkImage image, bool cpuWrite, bool cpuRead, VkMemoryAllocateInfo& out)
{
    VkMemoryRequirements memReq;
    VkMemoryPropertyFlags flags = 0;
    VkPhysicalDeviceMemoryProperties memProps;
    vkGetImageMemoryRequirements(m_device, image, &memReq);
    vkGetPhysicalDeviceMemoryProperties(m_pPhysicalDevice[m_physicalDevice], &memProps);
    if (cpuWrite || cpuRead)
        flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    if (cpuRead)
        flags |= VK_MEMORY_PROPERTY_HOST_CACHED_BIT;
    if (flags == 0)
        flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
    {
        const VkMemoryType& type = memProps.memoryTypes[i];
        if ((memReq.memoryTypeBits & (1 << i)) && ((type.propertyFlags & flags) == flags))
        {
            out.memoryTypeIndex = i;
            out.allocationSize = memReq.size;
            return true;
        }
    }    
    FAIL_RETVAL(false)
}

bool VulkanAPI::CreateDeviceAndSwapchain(Window window)
{
    if (m_lastResult != VK_SUCCESS)
        FAIL_RETVAL(false)

    if (m_surface == VK_NULL_HANDLE)
    {
#if !defined(WITHOUT_GLFW)
        m_lastResult = glfwCreateWindowSurface(m_instance, window, m_pMAlloc, &m_surface);
#else
#error
#endif
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    if (m_device == VK_NULL_HANDLE)
    {
        uint32_t primaryQueue = UINT32_MAX, secondaryQueue = UINT32_MAX;
        uint32_t primaryDevice = UINT32_MAX, secondaryDevice = UINT32_MAX;
        m_lastResult = vkEnumeratePhysicalDevices(m_instance, &m_numPhysicalDevices, nullptr);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        m_pPhysicalDevice = ALLOC(VkPhysicalDevice[m_numPhysicalDevices]);
        m_lastResult = vkEnumeratePhysicalDevices(m_instance, &m_numPhysicalDevices, m_pPhysicalDevice);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        m_pNumQueueFamilies = ALLOC(uint32_t[m_numPhysicalDevices]);
        m_ppQueueFamilyProperties = ALLOC(VkQueueFamilyProperties*[m_numPhysicalDevices]);
        for (uint32_t i = 0; (i < m_numPhysicalDevices) && ((primaryDevice == UINT32_MAX) || (secondaryDevice == UINT32_MAX)); i++)
        {
            uint32_t queueFamily = UINT32_MAX;
            m_pNumQueueFamilies[i] = 0;
            m_ppQueueFamilyProperties[i] = nullptr;
            vkGetPhysicalDeviceQueueFamilyProperties(m_pPhysicalDevice[i], &m_pNumQueueFamilies[i], nullptr);
            if (m_pNumQueueFamilies[i] == 0)
                continue;
            m_ppQueueFamilyProperties[i] = ALLOC(VkQueueFamilyProperties[m_pNumQueueFamilies[i]]);
            vkGetPhysicalDeviceQueueFamilyProperties(m_pPhysicalDevice[i], &m_pNumQueueFamilies[i], m_ppQueueFamilyProperties[i]);
            for (uint32_t j = 0; j < m_pNumQueueFamilies[i]; j++)
            {
                VkBool32 canPresent = VK_FALSE;
                VkQueueFlags flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT;
                VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(m_pPhysicalDevice[i], j, m_surface, &canPresent);
                if (result != VK_SUCCESS || canPresent != VK_TRUE)
                    continue;
                const VkQueueFamilyProperties& qf = m_ppQueueFamilyProperties[i][j];
                if ((qf.queueFlags & flags) == flags)
                {
                    queueFamily = j;
                    break;
                }
            }
            if (queueFamily != UINT32_MAX)
            {
                VkPhysicalDeviceProperties props;
                vkGetPhysicalDeviceProperties(m_pPhysicalDevice[i], &props);
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
        m_physicalDevice = (primaryDevice != UINT32_MAX) ? primaryDevice : secondaryDevice;
        m_queueFamily = (primaryQueue != UINT32_MAX) ? primaryQueue : secondaryQueue;
        if (m_physicalDevice == UINT32_MAX || m_queueFamily == UINT32_MAX)
            FAIL_RETVAL(false)
        {
            const char* ppExtensions[]
            {
                VK_KHR_SWAPCHAIN_EXTENSION_NAME
            };
            float queuePriority = 1.f;
            VkDeviceQueueCreateInfo queue{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
            queue.queueFamilyIndex = m_queueFamily;
            queue.queueCount = 1;
            queue.pQueuePriorities = &queuePriority;
            VkDeviceCreateInfo info{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
            info.queueCreateInfoCount = 1;
            info.pQueueCreateInfos = &queue;
            info.enabledExtensionCount = 1;
            info.ppEnabledExtensionNames = ppExtensions;
            VkPhysicalDevice phd = m_pPhysicalDevice[m_physicalDevice];
            m_lastResult = vkCreateDevice(phd, &info, m_pMAlloc, &m_device);
            if (m_lastResult != VK_SUCCESS)
                FAIL_RETVAL(false)
#if _DEBUG
#define LOAD(n) if (!LoadVkDeviceProc(m_device, #n, n)) __debugbreak()
#else
#define LOAD(n) LoadVkDeviceProc(m_device, #n, n)
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
        vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);
    }

    /* Create or re-create swapchain and related image views */

    m_lastResult = vkDeviceWaitIdle(m_device);
    if (m_lastResult != VK_SUCCESS)
        FAIL_RETVAL(false)
    {
        VkSurfaceCapabilitiesKHR caps;
        if (m_swapchain != VK_NULL_HANDLE)       
            DestroySwapchainResources();        
        m_lastResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_pPhysicalDevice[m_physicalDevice], m_surface, &caps);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        
        VkSwapchainCreateInfoKHR info{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
        info.surface = m_surface;
        info.minImageCount = (NUM_FRAME_CONTEXTS < caps.minImageCount) 
            ? caps.minImageCount 
            : ((NUM_FRAME_CONTEXTS > caps.maxImageCount) ? caps.maxImageCount : NUM_FRAME_CONTEXTS);        
        {
            uint32_t count = 0;
            m_lastResult = vkGetPhysicalDeviceSurfaceFormatsKHR(m_pPhysicalDevice[m_physicalDevice], m_surface, &count, nullptr);
            if (m_lastResult != VK_SUCCESS)
                FAIL_RETVAL(false)
            VkSurfaceFormatKHR* format = ALLOC(VkSurfaceFormatKHR[count]);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_pPhysicalDevice[m_physicalDevice], m_surface, &count, format);
            for (uint32_t i = 0; i < count; i++)
            {
                if (format[i].format == COLOR_BUFFER_FORMAT)
                {
                    info.imageFormat = format[i].format;
                    info.imageColorSpace = format[i].colorSpace;
                }
            }
            FREE(format);
            if (info.imageFormat == VK_FORMAT_UNDEFINED)
                FAIL_RETVAL(false)
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
            m_lastResult = vkGetPhysicalDeviceSurfacePresentModesKHR(m_pPhysicalDevice[m_physicalDevice], m_surface, &count, nullptr);
            if (m_lastResult != VK_SUCCESS)
                FAIL_RETVAL(false)
            VkPresentModeKHR* mode = ALLOC(VkPresentModeKHR[count]);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_pPhysicalDevice[m_physicalDevice], m_surface, &count, mode);
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
                FAIL_RETVAL(false)
        }
        info.clipped = VK_TRUE;
        info.oldSwapchain = m_swapchain;

        VkSwapchainKHR swapchain = VK_NULL_HANDLE;
        m_lastResult = vkCreateSwapchainKHR(m_device, &info, m_pMAlloc, &swapchain);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        if (m_swapchain != VK_NULL_HANDLE)
            vkDestroySwapchainKHR(m_device, m_swapchain, m_pMAlloc);
        m_swapchain = swapchain;
        m_screenSize.width = info.imageExtent.width;
        m_screenSize.height = info.imageExtent.height;
        m_screenSize.depth = 1;

        m_lastResult = vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainSize, nullptr);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        m_pSwapchainImage = ALLOC(VkImage[m_swapchainSize]);
        vkGetSwapchainImagesKHR(m_device, m_swapchain, &m_swapchainSize, m_pSwapchainImage);        
        
        m_viewport.x = 0.f;
        m_viewport.y = 0.f;
        m_viewport.width = static_cast<float>(info.imageExtent.width);
        m_viewport.height = static_cast<float>(info.imageExtent.height);
        m_viewport.minDepth = 0.f;
        m_viewport.maxDepth = 1.f;
        m_scissor.offset.x = 0;
        m_scissor.offset.y = 0;
        m_scissor.extent.width = info.imageExtent.width;
        m_scissor.extent.height = info.imageExtent.height;
    }

    /* Create final output render pass */

    if (m_finalColorPass == VK_NULL_HANDLE)
    {
        VkSubpassDependency spd[] =
        {
            {
                VK_SUBPASS_EXTERNAL,
                0,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_ACCESS_SHADER_READ_BIT,
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
                COLOR_BUFFER_FORMAT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            },
            {
                0,
                COLOR_BUFFER_FORMAT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_LOAD,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
            }
        };
        VkAttachmentReference dst{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
        VkAttachmentReference src{ 1, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL };
        VkSubpassDescription sp{ 0, VK_PIPELINE_BIND_POINT_GRAPHICS, 1, &src, 1, &dst };
        VkRenderPassCreateInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
        info.attachmentCount = 2;
        info.pAttachments = attd;
        info.subpassCount = 1;
        info.pSubpasses = &sp;
        info.dependencyCount = 2;
        info.pDependencies = spd;
        m_lastResult = vkCreateRenderPass(m_device, &info, m_pMAlloc, &m_finalColorPass);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)        
    }

    /* Create descriptor set layout and Allocate descriptor set */
    
    if (m_descriptorSetLayout == VK_NULL_HANDLE)
    {
        VkDescriptorSetLayoutBinding binding{};
        binding.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        binding.descriptorCount = 1;
        binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        VkDescriptorSetLayoutCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
        info.bindingCount = 1;
        info.pBindings = &binding;
        m_lastResult = vkCreateDescriptorSetLayout(m_device, &info, m_pMAlloc, &m_descriptorSetLayout);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    if (m_descriptorPool == VK_NULL_HANDLE)
    {
        VkDescriptorPoolSize size{};
        size.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
        size.descriptorCount = 1;
        VkDescriptorPoolCreateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
        info.maxSets = 1;
        info.poolSizeCount = 1;
        info.pPoolSizes = &size;
        m_lastResult = vkCreateDescriptorPool(m_device, &info, m_pMAlloc, &m_descriptorPool);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    if (m_finalColorDescriptorSet == VK_NULL_HANDLE)
    {
        VkDescriptorSetAllocateInfo info{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
        info.descriptorPool = m_descriptorPool;
        info.descriptorSetCount = 1;
        info.pSetLayouts = &m_descriptorSetLayout;
        m_lastResult = vkAllocateDescriptorSets(m_device, &info, &m_finalColorDescriptorSet);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    if (!CreateSwapchainResources()) /*write descriptor set*/
        return false;

    /* Allocate per-frame command buffers and sync primitives */

    if (m_commandPool == VK_NULL_HANDLE)
    {
        {
            VkCommandPoolCreateInfo info{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
            info.flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            info.queueFamilyIndex = m_queueFamily;
            m_lastResult = vkCreateCommandPool(m_device, &info, m_pMAlloc, &m_commandPool);
            if (m_lastResult != VK_SUCCESS)
                FAIL_RETVAL(false)
        }
        {
            if (!m_frameData.Init(m_swapchainSize))
                FAIL_RETVAL(false)
            VkCommandBuffer* cb = New<VkCommandBuffer>(m_swapchainSize);
            VkCommandBufferAllocateInfo info{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
            info.commandPool = m_commandPool;
            info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            info.commandBufferCount = m_swapchainSize;
            m_lastResult = vkAllocateCommandBuffers(m_device, &info, cb);
            if (m_lastResult != VK_SUCCESS)
                FAIL_RETVAL(false)
            VkFenceCreateInfo fence{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
            fence.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            VkSemaphoreCreateInfo semaphore{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
            for (uint32_t i = 0; i < m_swapchainSize; i++)
            {
                FrameData& frameData = *m_frameData.Get();
                m_lastResult = vkCreateFence(m_device, &fence, m_pMAlloc, &frameData.fence);
                if (m_lastResult != VK_SUCCESS)
                    FAIL_RETVAL(false)
                m_lastResult = vkCreateSemaphore(m_device, &semaphore, m_pMAlloc, &frameData.imageReady);
                if (m_lastResult != VK_SUCCESS)
                    FAIL_RETVAL(false)
                m_lastResult = vkCreateSemaphore(m_device, &semaphore, m_pMAlloc, &frameData.submitDone);
                if (m_lastResult != VK_SUCCESS)
                    FAIL_RETVAL(false)
                frameData.commandBuffer = cb[i];
                m_frameData.Next();
            }
            Delete(cb);
        }
    }    

    /* Create default shader modules */

    if (m_finalColorVS == VK_NULL_HANDLE)
    {
        VkShaderModuleCreateInfo info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        info.pCode = FINAL_COLOR_OUTPUT_VS;
        info.codeSize = sizeof(FINAL_COLOR_OUTPUT_VS);
        m_lastResult = vkCreateShaderModule(m_device, &info, m_pMAlloc, &m_finalColorVS);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    if (m_finalColorFS == VK_NULL_HANDLE)
    {
        VkShaderModuleCreateInfo info{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
        info.pCode = FINAL_COLOR_OUTPUT_FS;
        info.codeSize = sizeof(FINAL_COLOR_OUTPUT_FS);
        m_lastResult = vkCreateShaderModule(m_device, &info, m_pMAlloc, &m_finalColorFS);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    /* Create final color output pipeline */

    if (m_finalColorPipelineLayout == VK_NULL_HANDLE)
    {
        VkPipelineLayoutCreateInfo info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
        info.setLayoutCount = 1;
        info.pSetLayouts = &m_descriptorSetLayout;
        m_lastResult = vkCreatePipelineLayout(m_device, &info, m_pMAlloc, &m_finalColorPipelineLayout);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    if (m_finalColorPipeline == VK_NULL_HANDLE)
    {
        VkPipelineShaderStageCreateInfo pStages[] = { 
            { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO }, 
            { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO }
        };
        VkPipelineVertexInputStateCreateInfo vbState{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
        VkPipelineInputAssemblyStateCreateInfo iaState{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
        VkPipelineViewportStateCreateInfo vpState{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
        VkPipelineRasterizationStateCreateInfo rsState{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
        VkPipelineMultisampleStateCreateInfo msState{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
        VkPipelineColorBlendStateCreateInfo cbState{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
        VkPipelineDynamicStateCreateInfo dyState{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };

        pStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        pStages[0].module = m_finalColorVS;
        pStages[0].pName = "VSMain";
        pStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        pStages[1].module = m_finalColorFS;
        pStages[1].pName = "FSMain";

        iaState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        vpState.viewportCount = 1;
        vpState.scissorCount = 1;

        rsState.lineWidth = 1.f;

        msState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState cbaState{};
        cbaState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT 
            | VK_COLOR_COMPONENT_G_BIT 
            | VK_COLOR_COMPONENT_B_BIT 
            | VK_COLOR_COMPONENT_A_BIT;
        cbState.attachmentCount = 1;
        cbState.pAttachments = &cbaState;
        cbState.blendConstants[0] = 1.f;
        cbState.blendConstants[1] = 1.f;
        cbState.blendConstants[2] = 1.f;
        cbState.blendConstants[3] = 1.f;

        VkDynamicState pDynamicState[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        dyState.dynamicStateCount = 2;
        dyState.pDynamicStates = pDynamicState;

        VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
        info.stageCount = 2;
        info.pStages = pStages;
        info.pVertexInputState = &vbState;
        info.pInputAssemblyState = &iaState;
        info.pViewportState = &vpState;
        info.pRasterizationState = &rsState;
        info.pMultisampleState = &msState;
        info.pColorBlendState = &cbState;
        info.pDynamicState = &dyState;
        info.layout = m_finalColorPipelineLayout;
        info.renderPass = m_finalColorPass;
        m_lastResult = vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &info, m_pMAlloc, &m_finalColorPipeline);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    return true;
}

void VulkanAPI::BeginFrame()
{
    const FrameData& frameData = *m_frameData.Get();
    m_lastResult = vkWaitForFences(m_device, 1, &frameData.fence, VK_TRUE, UINT64_MAX);
    if (m_lastResult != VK_SUCCESS) 
        FAIL_RETURN()
    m_lastResult = vkResetFences(m_device, 1, &frameData.fence);
    if (m_lastResult != VK_SUCCESS) 
        FAIL_RETURN()
    m_lastResult = vkResetCommandBuffer(frameData.commandBuffer, 0);
    if (m_lastResult != VK_SUCCESS) 
        FAIL_RETURN()
    VkCommandBufferBeginInfo info{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    info.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    m_lastResult = vkBeginCommandBuffer(frameData.commandBuffer, &info);
    if (m_lastResult != VK_SUCCESS)
        FAIL_RETURN()
    else if (m_swapchainReset)
    {
        VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = m_colorImage;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.layerCount = 1;
        VkCommandBuffer cb = frameData.commandBuffer;
        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        vkCmdPipelineBarrier(cb, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        vkCmdClearColorImage(cb, barrier.image, barrier.newLayout, &m_clearColor, 1, &barrier.subresourceRange);
        srcStage = dstStage;
        dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        SWAP(barrier.srcAccessMask, barrier.dstAccessMask);
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        vkCmdPipelineBarrier(cb, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        m_swapchainReset = false;
    }
}

void VulkanAPI::EndFrame()
{
    uint32_t imageIdx = m_swapchainSize;
    VkFence fence = m_frameData.Get()->fence;
    VkCommandBuffer cb = m_frameData.Get()->commandBuffer;
    VkSemaphore imageReady = m_frameData.Get()->imageReady;
    VkSemaphore submitDone = m_frameData.Get()->submitDone;

    m_lastResult = vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, imageReady, VK_NULL_HANDLE, &imageIdx);
    if (m_lastResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_lastResult = VK_SUCCESS;
        CreateDeviceAndSwapchain(nullptr);
        return;
    }
    else if (m_lastResult != VK_SUCCESS)
        FAIL_RETURN()

    {
        VkRenderPassBeginInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        info.renderPass = m_finalColorPass;
        info.framebuffer = m_pSwapchainFramebuffer[imageIdx];
        info.renderArea.extent = { m_screenSize.width, m_screenSize.height };
        vkCmdBeginRenderPass(cb, &info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_finalColorPipeline);
        vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_finalColorPipelineLayout, 0, 1, &m_finalColorDescriptorSet, 0, nullptr);
        vkCmdSetViewport(cb, 0, 1, &m_viewport);
        vkCmdSetScissor(cb, 0, 1, &m_scissor);
        vkCmdDraw(cb, 3, 1, 0, 0);
        vkCmdEndRenderPass(cb);
    }

    m_lastResult = vkEndCommandBuffer(cb);
    if (m_lastResult != VK_SUCCESS)
        FAIL_RETURN()

    {
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo info{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &imageReady;
        info.pWaitDstStageMask = &waitStage;
        info.commandBufferCount = 1;
        info.pCommandBuffers = &cb;
        info.signalSemaphoreCount = 1;
        info.pSignalSemaphores = &submitDone;
        m_lastResult = vkQueueSubmit(m_queue, 1, &info, fence);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETURN()
    }

    {
        VkPresentInfoKHR info{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
        info.waitSemaphoreCount = 1;
        info.pWaitSemaphores = &submitDone;
        info.swapchainCount = 1;
        info.pSwapchains = &m_swapchain;
        info.pImageIndices = &imageIdx;
        m_lastResult = vkQueuePresentKHR(m_queue, &info);
        if (m_lastResult == VK_ERROR_OUT_OF_DATE_KHR)
        {
            m_lastResult = VK_SUCCESS;
            CreateDeviceAndSwapchain(nullptr);
            return;
        }
        else if (m_lastResult != VK_SUCCESS)
            FAIL_RETURN()
    }

    m_frameData.Next();
}

bool VulkanAPI::CreateSwapchainResources()
{
    {
        VkImageCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
        info.imageType = VK_IMAGE_TYPE_2D;
        info.format = COLOR_BUFFER_FORMAT;
        info.extent = m_screenSize;
        info.mipLevels = 1;
        info.arrayLayers = 1;
        info.samples = VK_SAMPLE_COUNT_1_BIT;
        info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        m_lastResult = vkCreateImage(m_device, &info, m_pMAlloc, &m_colorImage);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
            info.format = DEPTH_BUFFER_FORMAT;
        info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
            | VK_IMAGE_USAGE_SAMPLED_BIT;
        m_lastResult = vkCreateImage(m_device, &info, m_pMAlloc, &m_depthImage);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        VkMemoryAllocateInfo colorMem{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        if (!GetAllocInfo(m_colorImage, false, false, colorMem))
            return false;
        m_lastResult = vkAllocateMemory(m_device, &colorMem, m_pMAlloc, &m_colorImageMemory);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        m_lastResult = vkBindImageMemory(m_device, m_colorImage, m_colorImageMemory, 0);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        VkMemoryAllocateInfo depthMem{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
        if (!GetAllocInfo(m_depthImage, false, false, depthMem))
            return false;
        m_lastResult = vkAllocateMemory(m_device, &depthMem, m_pMAlloc, &m_depthImageMemory);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
        m_lastResult = vkBindImageMemory(m_device, m_depthImage, m_depthImageMemory, 0);
        if (m_lastResult != VK_SUCCESS)
            FAIL_RETVAL(false)
    }

    VkImageViewCreateInfo info{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
    info.image = m_depthImage;
    info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    info.format = DEPTH_BUFFER_FORMAT;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    info.subresourceRange.levelCount = 1;
    info.subresourceRange.layerCount = 1;
    m_lastResult = vkCreateImageView(m_device, &info, m_pMAlloc, &m_depthImageView);
    if (m_lastResult != VK_SUCCESS)
        FAIL_RETVAL(false)
    info.image = m_colorImage;
    info.format = COLOR_BUFFER_FORMAT;
    info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    m_lastResult = vkCreateImageView(m_device, &info, m_pMAlloc, &m_colorImageView);
    if (m_lastResult != VK_SUCCESS)
        FAIL_RETVAL(false)
    m_pSwapchainImageView = ALLOC(VkImageView[m_swapchainSize]);
    m_pSwapchainFramebuffer = ALLOC(VkFramebuffer[m_swapchainSize]);
    VkImageView pFBImage[] = { VK_NULL_HANDLE, m_colorImageView };
    VkFramebufferCreateInfo framebuff{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
    framebuff.renderPass = m_finalColorPass;
    framebuff.attachmentCount = 2;
    framebuff.pAttachments = pFBImage;
    framebuff.width = m_screenSize.width;
    framebuff.height = m_screenSize.height;
    framebuff.layers = 1;
    for (uint32_t i = 0; i < m_swapchainSize; i++)
    { 
        info.image = m_pSwapchainImage[i];
        if (m_lastResult != VK_SUCCESS)
        {
            m_pSwapchainImageView[i] = VK_NULL_HANDLE;
            m_pSwapchainFramebuffer[i] = VK_NULL_HANDLE;
        }
        else
        {
            m_lastResult = vkCreateImageView(m_device, &info, m_pMAlloc, &m_pSwapchainImageView[i]);
            if (m_lastResult == VK_SUCCESS)
            {
                pFBImage[0] = m_pSwapchainImageView[i];
                m_lastResult = vkCreateFramebuffer(m_device, &framebuff, m_pMAlloc, &m_pSwapchainFramebuffer[i]);
            }
        }        
    }
    if (m_lastResult != VK_SUCCESS)
        FAIL_RETVAL(false)

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageView = m_colorImageView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    VkWriteDescriptorSet write{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
    write.dstSet = m_finalColorDescriptorSet;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
    write.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(m_device, 1, &write, 0, nullptr);
    m_swapchainReset = true;

    return true;
}

void VulkanAPI::DestroySwapchainResources()
{
    for (uint32_t i = 0; i < m_swapchainSize; i++)
    {
        if (m_pSwapchainFramebuffer[i] != VK_NULL_HANDLE)
            vkDestroyFramebuffer(m_device, m_pSwapchainFramebuffer[i], m_pMAlloc);
        if (m_pSwapchainImageView[i] != VK_NULL_HANDLE)
            vkDestroyImageView(m_device, m_pSwapchainImageView[i], m_pMAlloc);
    }
    if (m_colorImageView != VK_NULL_HANDLE)
        vkDestroyImageView(m_device, m_colorImageView, m_pMAlloc);
    if (m_colorImage != VK_NULL_HANDLE)
        vkDestroyImage(m_device, m_colorImage, m_pMAlloc);
    if (m_colorImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_colorImageMemory, m_pMAlloc);
    if (m_depthImageView != VK_NULL_HANDLE)
        vkDestroyImageView(m_device, m_depthImageView, m_pMAlloc);
    if (m_depthImage != VK_NULL_HANDLE)
        vkDestroyImage(m_device, m_depthImage, m_pMAlloc);
    if (m_depthImageMemory != VK_NULL_HANDLE)
        vkFreeMemory(m_device, m_depthImageMemory, m_pMAlloc);
    FREE(m_pSwapchainFramebuffer);
    FREE(m_pSwapchainImageView);
}
