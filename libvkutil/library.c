#include "internal.h"

void LoadVulkanLibrary(void)
{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
    HMODULE hModule = LoadLibrary("vulkan-1");
    TEST(hModule);
    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)GetProcAddress(hModule, "vkGetInstanceProcAddr");
    TEST(vkGetInstanceProcAddr);
#endif
}
