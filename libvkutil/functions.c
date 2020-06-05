#include "internal.h"

VkAllocationCallbacks* gVkAlloc    = NULL;
VkInstance             gVkInstance = VK_NULL_HANDLE;

PFN_vkGetInstanceProcAddr                     vkGetInstanceProcAddr                     = NULL;
PFN_vkEnumerateInstanceExtensionProperties    vkEnumerateInstanceExtensionProperties    = NULL;
PFN_vkEnumerateInstanceLayerProperties        vkEnumerateInstanceLayerProperties        = NULL;
PFN_vkCreateInstance                          vkCreateInstance                          = NULL; 
PFN_vkDestroyInstance                         vkDestroyInstance                         = NULL;
PFN_vkEnumeratePhysicalDevices                vkEnumeratePhysicalDevices                = NULL;

PFN_vkDestroySurfaceKHR                       vkDestroySurfaceKHR                       = NULL;
PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      vkGetPhysicalDeviceSurfaceFormatsKHR      = NULL;
PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR = NULL;
PFN_vkGetPhysicalDeviceSurfaceSupportKHR      vkGetPhysicalDeviceSurfaceSupportKHR      = NULL;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
PFN_vkCreateWin32SurfaceKHR                   vkCreateWin32SurfaceKHR                   = NULL;
#endif

PFN_vkDebugUtilsMessengerCallbackEXT          vkDebugUtilsMessengerCallbackEXT          = NULL;
PFN_vkCmdBeginDebugUtilsLabelEXT              vkCmdBeginDebugUtilsLabelEXT              = NULL;
PFN_vkCmdEndDebugUtilsLabelEXT                vkCmdEndDebugUtilsLabelEXT                = NULL;
PFN_vkCmdInsertDebugUtilsLabelEXT             vkCmdInsertDebugUtilsLabelEXT             = NULL;
PFN_vkCreateDebugUtilsMessengerEXT            vkCreateDebugUtilsMessengerEXT            = NULL;
PFN_vkDestroyDebugUtilsMessengerEXT           vkDestroyDebugUtilsMessengerEXT           = NULL;
PFN_vkQueueBeginDebugUtilsLabelEXT            vkQueueBeginDebugUtilsLabelEXT            = NULL;
PFN_vkQueueEndDebugUtilsLabelEXT              vkQueueEndDebugUtilsLabelEXT              = NULL;
PFN_vkQueueInsertDebugUtilsLabelEXT           vkQueueInsertDebugUtilsLabelEXT           = NULL;
PFN_vkSetDebugUtilsObjectNameEXT              vkSetDebugUtilsObjectNameEXT              = NULL;
PFN_vkSetDebugUtilsObjectTagEXT               vkSetDebugUtilsObjectTagEXT               = NULL;
PFN_vkSubmitDebugUtilsMessageEXT              vkSubmitDebugUtilsMessageEXT              = NULL;

#define LOAD(n) do{ n = (PFN_##n)vkGetInstanceProcAddr(gVkInstance, #n); TEST(n) }while(0)

void LoadInstanceFunctions(const bool* ext)
{    
    LOAD(vkDestroyInstance);
    LOAD(vkEnumeratePhysicalDevices);   
    if (ext[VKUTIL_KHR_SURFACE])
    {
        LOAD(vkDestroySurfaceKHR);
        LOAD(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
        LOAD(vkGetPhysicalDeviceSurfaceFormatsKHR);
        LOAD(vkGetPhysicalDeviceSurfacePresentModesKHR);
        LOAD(vkGetPhysicalDeviceSurfaceSupportKHR);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
        LOAD(vkCreateWin32SurfaceKHR);
#endif
    }
    if (ext[VKUTIL_EXT_DEBUG_UTILS])
    {
        LOAD(vkDebugUtilsMessengerCallbackEXT);
        LOAD(vkCmdBeginDebugUtilsLabelEXT);
        LOAD(vkCmdEndDebugUtilsLabelEXT);
        LOAD(vkCmdInsertDebugUtilsLabelEXT);
        LOAD(vkCreateDebugUtilsMessengerEXT);
        LOAD(vkDestroyDebugUtilsMessengerEXT);
        LOAD(vkQueueBeginDebugUtilsLabelEXT);
        LOAD(vkQueueEndDebugUtilsLabelEXT);
        LOAD(vkQueueInsertDebugUtilsLabelEXT);
        LOAD(vkSetDebugUtilsObjectNameEXT);
        LOAD(vkSetDebugUtilsObjectTagEXT);
        LOAD(vkSubmitDebugUtilsMessageEXT);
    }
}

#undef LOAD
