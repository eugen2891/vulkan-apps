#include "internal.h"

VkUtilLayerInfo gVkLayer[VKUTIL_TOTAL_LAYERS] =
{
    { "VK_LAYER_KHRONOS_validation", 0, false },
    { "VK_LAYER_RENDERDOC_Capture", 0, false },
    { "VK_LAYER_NV_optimus", 0, false },
    { "VK_LAYER_LUNARG_standard_validation", false }
};

VkUtilExtensionInfo gVkExtension[VKUTIL_TOTAL_EXTENSIONS] =
{
    { VK_KHR_SURFACE_EXTENSION_NAME, 0, VKUTIL_KHR_SURFACE_IMPL, false }, //VKUTIL_KHR_SURFACE
    { VK_EXT_DEBUG_UTILS_EXTENSION_NAME, 0, VKUTIL_EXT_NONE, false     }, //VKUTIL_EXT_DEBUG_UTILS

    { VK_KHR_SWAPCHAIN_EXTENSION_NAME, 0, VKUTIL_EXT_NONE, false       }, //VKUTIL_KHR_SWAPCHAIN

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    { VK_KHR_WIN32_SURFACE_EXTENSION_NAME, 0, VKUTIL_EXT_NONE, false   }
#endif
};

u32 FNV1A_U32(const char* str)
{
    u32 retVal = 2166136261u;
    while (*str)
    {
        retVal ^= (u32)*str++;
        retVal *= 16777619u;
    }
    return retVal;
}

void LoadInstanceExtensionsAndLayers(void)
{
    u32 numExtensions = 0, numLayers = 0;
    for (u32 i = VKUTIL_LAYERS_BEGIN; i < VKUTIL_TOTAL_LAYERS; i++)
    {
        gVkLayer[i].layerNameHash = FNV1A_U32(gVkLayer[i].layerName);
    }
    for (u32 i = VKUTIL_INSTANCE_EXT_BEGIN; i < VKUTIL_TOTAL_EXTENSIONS; i++)
    {
        gVkExtension[i].extensionNameHash = FNV1A_U32(gVkExtension[i].extensionName);
    }
    typedef PFN_vkEnumerateInstanceExtensionProperties ProcTypeExt;
    const char* procExt = "vkEnumerateInstanceExtensionProperties";
    vkEnumerateInstanceExtensionProperties = (ProcTypeExt)vkGetInstanceProcAddr(VK_NULL_HANDLE, procExt);
    VKTEST(vkEnumerateInstanceExtensionProperties(NULL, &numExtensions, NULL));
    if (numExtensions > 0)
    {
        VkExtensionProperties* props = (VkExtensionProperties*)malloc(numExtensions * sizeof(VkExtensionProperties));
        VKTEST(vkEnumerateInstanceExtensionProperties(NULL, &numExtensions, props));
        for (u32 i = 0; i < numExtensions; i++)
        {
            u32 extNameHash = FNV1A_U32(props[i].extensionName);
            for (u32 j = VKUTIL_INSTANCE_EXT_BEGIN; j < VKUTIL_TOTAL_EXTENSIONS; j++)
            {
                if (extNameHash == gVkExtension[j].extensionNameHash)
                {
                    gVkExtension[j].isAvailable = true;
                    break;
                }
            }
        }
        free(props);
    }
    typedef PFN_vkEnumerateInstanceLayerProperties ProcTypeLayer;
    const char* procLayer = "vkEnumerateInstanceLayerProperties";
    vkEnumerateInstanceLayerProperties = (ProcTypeLayer)vkGetInstanceProcAddr(VK_NULL_HANDLE, procLayer);
    VKTEST(vkEnumerateInstanceLayerProperties(&numLayers, NULL));
    if (numLayers > 0)
    {
        VkLayerProperties* props = (VkLayerProperties*)malloc(numLayers * sizeof(VkLayerProperties));
        VKTEST(vkEnumerateInstanceLayerProperties(&numLayers, props));
        for (u32 i = 0; i < numLayers; i++)
        {
            u32 nameHash = FNV1A_U32(props[i].layerName);
            for (u32 j = VKUTIL_LAYERS_BEGIN; j < VKUTIL_TOTAL_LAYERS; j++)
            {
                if (nameHash == gVkLayer[j].layerNameHash)
                {
                    gVkLayer[j].isAvailable = true;
                    break;
                }
            }
        }
        free(props);
    }
}

u32 AddExtensions(const char** list, const bool* req, i32 from, i32 to)
{
    u32 count = 0;
    for (i32 i = from; i < to; i++)
    {
        if (req[i])
        {
            TEST(gVkExtension[i].isAvailable);
            if (gVkExtension[i].implementationExt != VKUTIL_EXT_NONE)
            {
                i32 impl = gVkExtension[i].implementationExt;
                TEST(gVkExtension[impl].isAvailable);
                list[count++] = gVkExtension[impl].extensionName;
            }
            list[count++] = gVkExtension[i].extensionName;
        }
    }
    return count;
}

u32 AddLayers(const char** list, const bool* req)
{
    u32 count = 0;
    for (i32 i = VKUTIL_LAYERS_BEGIN; i < VKUTIL_NUM_LAYERS; i++)
    {
        if (req[i] && gVkLayer[i].isAvailable)
        {
            list[count++] = gVkLayer[i].layerName;
        }
    }
    if (req[VKUTIL_KHR_VALIDATION] && !gVkLayer[VKUTIL_KHR_VALIDATION].isAvailable)
    {
        if (gVkLayer[VKUTIL_LUNARG_VALIDATION].isAvailable)
        {
            list[count++] = gVkLayer[VKUTIL_LUNARG_VALIDATION].layerName;
        }
    }
    return count;
}
