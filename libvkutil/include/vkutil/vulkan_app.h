#pragma once

#include "vulkan_api.h"

class MemStack;

template <typename T> struct TArray;

typedef struct GLFWwindow GLFWwindow;

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
    const VkSurfaceKHR&     surface = vkSurface;
    const VkDevice&         device = vkDevice;
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

    VkInstance       vkInstance = VK_NULL_HANDLE;
    VkPhysicalDevice vkPhDev = VK_NULL_HANDLE;
    VkDevice         vkDevice = VK_NULL_HANDLE;
    VkSurfaceKHR     vkSurface = VK_NULL_HANDLE;
    VkSwapchainKHR   vkSwapchain = VK_NULL_HANDLE;
    DeviceQueue*     pDeviceQueue = nullptr;
    VkImage*         pSwapchainImage = nullptr;
    VkImageView*     pSwapchainImageView = nullptr;

    uint32_t numDevQueues = UINT32_MAX;
    uint32_t graphicsQueue = UINT32_MAX;
    uint32_t transferQueue = UINT32_MAX;
    uint32_t presentQueue = UINT32_MAX;
    uint32_t swapchainSize = UINT32_MAX;

    static const char* pEngineName;
    static const char* pAppName;
    static VulkanApp*  pAppImpl;

};
