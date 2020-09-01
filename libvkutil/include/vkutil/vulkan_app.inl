#pragma once

#if !defined(VKAPP_IMPL)
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32"  )
#pragma comment(lib, "gdi32"   )
#pragma comment(lib, "shell32" )
#endif
#else
#define VKAPI_INL
#endif 

#include <vkutil/mem_stack.h>
#include <vkutil/vulkan_api.h>

#if !defined(VKAPP_IMPL)
typedef struct GLFWwindow GLFWwindow;
#else
#include <GLFW/glfw3.h>
#endif

#if !defined(VKAPI_TMP_MEM) || (VKAPI_TMP_MEM < 1048576)
#if defined(VKAPI_TMP_MEM)
#undef VKAPI_TMP_MEM
#endif
#define VKAPI_TMP_MEM 1048576
#endif

template <typename T> struct TArray;

class VulkanApp
{

public:

    static int main();

protected:

    enum PhDevUsability
    {
        PHDEV_UNUSABLE = -1,
        PHDEV_BEST_FIT = 0,
        PHDEV_FALLBACK = 1
    };

    VulkanApp(MemStack& stack);
    virtual VkAllocationCallbacks* getAllocator();
    virtual void addInstanceExtensions(TArray<const char*>& list);
    virtual void addValidationLayers(TArray<const char*>& list);
    virtual PhDevUsability canUsePhysicalDevice(VkPhysicalDevice phDev);
    virtual void addDeviceExtensions(TArray<const char*>& list);
    virtual VkSurfaceFormatKHR getSurfaceFormat();
    virtual VkPresentModeKHR getPresentMode();
    virtual bool assignDeviceQueues();
    virtual bool initialize();

    static void setImpl(VulkanApp* pImpl, const char* pName);

    const VkInstance&       instance = vkInstance;
    const VkSurfaceKHR&     surface  = vkSurface;
    const VkDevice&         device   = vkDevice;
    const VkPhysicalDevice& phDevice = vkPhDev;
    MemStack&               memStack;

private:

    struct DeviceQueue
    {
        VkQueueFamilyProperties properties;
        VkQueue                 handle;
        VkBool32                canPresent;
    };

    static bool startup();
    static bool createDeviceAndSwapchain(GLFWwindow* window);
    static void shutdown();

    VkInstance       vkInstance          = VK_NULL_HANDLE;
    VkPhysicalDevice vkPhDev             = VK_NULL_HANDLE;
    VkDevice         vkDevice            = VK_NULL_HANDLE;
    VkSurfaceKHR     vkSurface           = VK_NULL_HANDLE;
    VkSwapchainKHR   vkSwapchain         = VK_NULL_HANDLE;
    DeviceQueue*     pDeviceQueue        = nullptr;
    VkImage*         pSwapchainImage     = nullptr;
    VkImageView*     pSwapchainImageView = nullptr;

    uint32_t numDevQueues  = UINT32_MAX;
    uint32_t graphicsQueue = UINT32_MAX;
    uint32_t transferQueue = UINT32_MAX;
    uint32_t presentQueue  = UINT32_MAX;
    uint32_t swapchainSize = UINT32_MAX;

    static const char* pEngineName;
    static const char* pAppName;
    static VulkanApp*  pAppImpl;

};

#if !defined(VKAPP_IMPL)

static TMemStack<VKAPI_TMP_MEM> gVkMemStack;

#define VULKAN_APP(n) static n g##n##;
#define VULKAN_APP_INITIALIZER(n) \
    public:n():VulkanApp(gVkMemStack){setImpl(this,#n);}private:

#if !defined(_WIN32)
int main(int argc, const char** argv)
#else
int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prev, LPSTR cmd, int show)
#endif
{
    return VulkanApp::main();
}

#endif
