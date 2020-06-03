#include "internal.h"

static const char* VKUTIL_LIB_NAME = "VKUTIL";

static const char* VKUTIL_APP_NAME = "VKUTIL_APPLICATION";

static void ApplyDefaults(VkUtilInitOptions* pOptions);

void vkUtilInitialize(const VkUtilInitOptions* pOptions)
{
    TEST(pOptions);
    LoadVulkanLibrary();
    LoadInstanceExtensionsAndLayers();
    VkUtilInitOptions options = *pOptions;
    ApplyDefaults(&options);
    {
        VkApplicationInfo app = { 0 };
        app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName = options.programName;
        app.applicationVersion = options.programVersion;
        app.pEngineName = options.engineName;
        app.engineVersion = options.engineVersion;
        app.apiVersion = options.vulkanApiVersion;
        VkInstanceCreateInfo info = { 0 };
        const char* layers[VKUTIL_TOTAL_LAYERS] = { 0 };
        const char* extensions[VKUTIL_TOTAL_EXTENSIONS] = { 0 };
        info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        info.pApplicationInfo = &app;
        info.enabledExtensionCount = AddExtensions(extensions, options.vulkanExtension, VKUTIL_INSTANCE_EXT_BEGIN, VKUTIL_INSTANCE_EXT_END);
        info.enabledLayerCount = AddLayers(layers, options.vulkanLayer);
        info.ppEnabledExtensionNames = extensions;
        info.ppEnabledLayerNames = layers;
        vkCreateInstance = (PFN_vkCreateInstance)vkGetInstanceProcAddr(VK_NULL_HANDLE, "vkCreateInstance");
        TEST(vkCreateInstance);
        VKTEST(vkCreateInstance(&info, options.vulkanAlloc, &gVkInstance));
        LoadInstanceFunctions();
        gVkAlloc = options.vulkanAlloc;
    }
}

void vkUtilFinalize(void)
{
    vkDestroyInstance(gVkInstance, gVkAlloc);
}

void ApplyDefaults(VkUtilInitOptions* pOptions)
{
    if (!pOptions->programName)
    {
        pOptions->programName = VKUTIL_APP_NAME;
    }
    if (!pOptions->engineName)
    {
        pOptions->engineName = VKUTIL_LIB_NAME;
    }
    if (!pOptions->vulkanApiVersion)
    {
        pOptions->vulkanApiVersion = VKUTIL_DEFAULT_VULKAN_API;
    }
#if _DEBUG && VKUTIL_FORCE_DEBUG_VALIDATION
    pOptions->vulkanLayer[VKUTIL_KHR_VALIDATION] = true;
#endif
    if (!pOptions->noDisplay)
    {
        pOptions->vulkanExtension[VKUTIL_KHR_SURFACE] = true;
        pOptions->vulkanExtension[VKUTIL_KHR_SWAPCHAIN] = true;
    }
}

#if defined(VK_USE_PLATFORM_WIN32_KHR)

extern int main(int agrc, const char* argv[]);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prev, LPSTR cmd, int show)
{
    char name[256] = { 0 };
    const char* argv[] = { name };
    GetModuleFileName(NULL, name, sizeof(name));
    return main(1, argv);
}

#endif
