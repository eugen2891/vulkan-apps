#include "internal.h"

VkAllocationCallbacks* gVkAlloc    = NULL;
VkInstance             gVkInstance = VK_NULL_HANDLE;

PFN_vkGetInstanceProcAddr                  vkGetInstanceProcAddr                  = NULL;
PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties = NULL;
PFN_vkEnumerateInstanceLayerProperties     vkEnumerateInstanceLayerProperties     = NULL;
PFN_vkCreateInstance                       vkCreateInstance                       = NULL; 
PFN_vkDestroyInstance                      vkDestroyInstance                      = NULL;
PFN_vkEnumeratePhysicalDevices             vkEnumeratePhysicalDevices             = NULL;

#define LOAD(n) do{ n = (PFN_##n)vkGetInstanceProcAddr(gVkInstance, #n); TEST(n) }while(0)

void LoadInstanceFunctions(void)
{    
    LOAD(vkDestroyInstance);
    LOAD(vkEnumeratePhysicalDevices);
}

#undef LOAD
