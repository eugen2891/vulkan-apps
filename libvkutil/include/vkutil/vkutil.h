#pragma once

#include <stdbool.h>

#if !defined(__cplusplus)
#define VKUTIL_API extern
#else
#define VKUTIL_API extern "C"
#endif

#define VK_NO_PROTOTYPES

#if defined(_WIN32)
struct IUnknown;
#define VK_USE_PLATFORM_WIN32_KHR
#else
#error
#endif

#include <vulkan/vulkan.h>

typedef uint8_t  u8;
typedef int32_t  i32;
typedef uint32_t u32;
typedef int64_t  i64;
typedef uint64_t u64;

enum
{
    VKUTIL_KHR_VALIDATION,
    VKUTIL_RDOC_CAPTURE,
    VKUTIL_NV_OPTIMUS,
    VKUTIL_NUM_LAYERS
};

enum
{
    VKUTIL_INSTANCE_EXT_BEGIN = 0,
    
    VKUTIL_KHR_SURFACE = VKUTIL_INSTANCE_EXT_BEGIN,
    VKUTIL_EXT_DEBUG_UTILS,
    VKUTIL_INSTANCE_EXT_END,
    
    VKUTIL_DEVICE_EXT_BEGIN = VKUTIL_INSTANCE_EXT_END,

    VKUTIL_KHR_SWAPCHAIN = VKUTIL_DEVICE_EXT_BEGIN,
    VKUTIL_DEVICE_EXT_END,

    VKUTIL_NUM_EXTENSIONS = VKUTIL_DEVICE_EXT_END,
    VKUTIL_NUM_DEVICE_EXT = VKUTIL_NUM_EXTENSIONS - VKUTIL_INSTANCE_EXT_END    
};

enum 
{
    VKUTIL_WINDOW_CENTERED  = 1,
    VKUTIL_WINDOW_MAXIMIZED = 2
};

typedef struct VkUtilInitOptions
{
    const char*             programName;
    const char*             engineName;
    u32                     programVersion;
    u32                     engineVersion;
    u32                     vulkanApiVersion;
    bool                    vulkanLayer[VKUTIL_NUM_LAYERS];
    bool                    vulkanExtension[VKUTIL_NUM_EXTENSIONS];
    bool                    noDisplay;
    VkAllocationCallbacks*  vulkanAlloc;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    HWND                    win32HWnd;
    HINSTANCE               win32HInstance;
#endif
    VkFormat                windowFormat;
    i32                     windowWidth;
    i32                     windowHeight;
    u32                     windowFlags;
} VkUtilInitOptions;

VKUTIL_API PFN_vkGetInstanceProcAddr                     vkGetInstanceProcAddr;
VKUTIL_API PFN_vkEnumerateInstanceExtensionProperties    vkEnumerateInstanceExtensionProperties;
VKUTIL_API PFN_vkEnumerateInstanceLayerProperties        vkEnumerateInstanceLayerProperties;
VKUTIL_API PFN_vkCreateInstance                          vkCreateInstance;
VKUTIL_API PFN_vkDestroyInstance                         vkDestroyInstance;
VKUTIL_API PFN_vkEnumeratePhysicalDevices                vkEnumeratePhysicalDevices;

VKUTIL_API PFN_vkDestroySurfaceKHR                       vkDestroySurfaceKHR;
VKUTIL_API PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
VKUTIL_API PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      vkGetPhysicalDeviceSurfaceFormatsKHR;
VKUTIL_API PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
VKUTIL_API PFN_vkGetPhysicalDeviceSurfaceSupportKHR      vkGetPhysicalDeviceSurfaceSupportKHR;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKUTIL_API PFN_vkCreateWin32SurfaceKHR                   vkCreateWin32SurfaceKHR;
#endif

VKUTIL_API PFN_vkDebugUtilsMessengerCallbackEXT          vkDebugUtilsMessengerCallbackEXT;
VKUTIL_API PFN_vkCmdBeginDebugUtilsLabelEXT              vkCmdBeginDebugUtilsLabelEXT;
VKUTIL_API PFN_vkCmdEndDebugUtilsLabelEXT                vkCmdEndDebugUtilsLabelEXT;
VKUTIL_API PFN_vkCmdInsertDebugUtilsLabelEXT             vkCmdInsertDebugUtilsLabelEXT;
VKUTIL_API PFN_vkCreateDebugUtilsMessengerEXT            vkCreateDebugUtilsMessengerEXT;
VKUTIL_API PFN_vkDestroyDebugUtilsMessengerEXT           vkDestroyDebugUtilsMessengerEXT;
VKUTIL_API PFN_vkQueueBeginDebugUtilsLabelEXT            vkQueueBeginDebugUtilsLabelEXT;
VKUTIL_API PFN_vkQueueEndDebugUtilsLabelEXT              vkQueueEndDebugUtilsLabelEXT;
VKUTIL_API PFN_vkQueueInsertDebugUtilsLabelEXT           vkQueueInsertDebugUtilsLabelEXT;
VKUTIL_API PFN_vkSetDebugUtilsObjectNameEXT              vkSetDebugUtilsObjectNameEXT;
VKUTIL_API PFN_vkSetDebugUtilsObjectTagEXT               vkSetDebugUtilsObjectTagEXT;
VKUTIL_API PFN_vkSubmitDebugUtilsMessageEXT              vkSubmitDebugUtilsMessageEXT;

VKUTIL_API void vkUtilInitialize(const VkUtilInitOptions* pOptions);
VKUTIL_API void vkUtilFinalize(void);
