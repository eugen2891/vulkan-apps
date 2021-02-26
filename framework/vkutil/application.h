#pragma once

#include "functions.h"

namespace vkutil
{
    class Application;
}

vkutil::Application* CreateApplication(int argc = 0, const char** argv = nullptr);

void DestroyApplication(vkutil::Application*);

namespace vkutil
{

    struct IHLSLCompiler;

    class Application : public Functions
    {

    public:

        bool Initialize(void* pWindow);
        void NextFrame(float elapsedT, float deltaT);
        void Finalize();

        const char* GetWindowTitle();        
        const char* GetApplicationName() const;
        void GetWindowSize(int& w, int& h) const;
        virtual ~Application() = default;            

    protected:

        Application() = default;

        virtual bool OnInitialized();
        virtual void UpdateState(float elapsedT, float deltaT);
        virtual bool RenderFrame(uint32_t contextIdx);
        virtual void OnFinalized();

        virtual void SetInstanceExtensions(VkInstanceCreateInfo& info);
        virtual void SetDeviceExtensions(VkDeviceCreateInfo& info);

        uint32_t GetSwapchainImageIndex();

        const char* m_pAppName = nullptr;
        const char* m_pWndTitle = nullptr;        

        VkApplicationInfo m_appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };

        VkPhysicalDeviceProperties m_phdProperties;

        VkAllocationCallbacks* m_pVkAlloc = nullptr;
        VkInstance m_vkInstance = VK_NULL_HANDLE;
        VkDevice m_vkDevice = VK_NULL_HANDLE;
        VkSurfaceKHR m_vkSurface = VK_NULL_HANDLE;
        VkPhysicalDevice m_vkPhysicalDevice = VK_NULL_HANDLE;
        VkSwapchainKHR m_vkSwapchain = VK_NULL_HANDLE;
        VkImageView* m_pSwapchainImageView = nullptr;
        VkCommandBuffer m_vkCommandBuffer = VK_NULL_HANDLE;

        IHLSLCompiler* m_pHLSLCompiler = nullptr;
        
        VkFormat m_swapchainFormat = VK_FORMAT_B8G8R8A8_UNORM;
        VkColorSpaceKHR m_swapchainColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        VkPresentModeKHR m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
        VkExtent3D m_swapchainExtent = { 0, 0, 1 };
        uint32_t m_numSwapchainFrames = 0;
        
        int m_windowW = VKUTIL_WINDOW_W;
        int m_windowH = VKUTIL_WINDOW_H;

    private:        

        bool CreateWindowSurface(void* pWindow);

        bool SelectPhysicalDevice(VkPhysicalDeviceType type);

        bool DetectSwapchainExtent(VkSwapchainCreateInfoKHR& info);
        bool DetectSwapchainFormat(VkSwapchainCreateInfoKHR& info);
        bool DetectPresentMode(VkSwapchainCreateInfoKHR& info);

        bool CreateOrUpdateSwapchain();
        bool SetupCommandContext();

        char m_wndTitleText[256] = {};   

        VkQueue m_vkQueue = VK_NULL_HANDLE;
        VkCommandPool m_vkCommandPool = VK_NULL_HANDLE;
        VkCommandBuffer* m_pCommandBuffer = nullptr;
        VkFence* m_pCommandBufferFence = nullptr;
        VkSemaphore* m_pImageReadySem = nullptr;
        VkSemaphore* m_pSubmitDoneSem = nullptr;

        uint32_t m_queueFamily = UINT32_MAX;
        
        uint32_t m_swapchainImageIndex = UINT32_MAX;
        uint32_t m_commandContext = 0;

    };

}

inline void vkutil::Application::GetWindowSize(int& w, int& h) const
{
    w = (m_windowW > VKUTIL_WINDOW_W) ? m_windowW : VKUTIL_WINDOW_W;
    h = (m_windowH > VKUTIL_WINDOW_H) ? m_windowH : VKUTIL_WINDOW_H;
}

inline const char* vkutil::Application::GetApplicationName() const
{
    return (m_pAppName) ? m_pAppName : nullptr;
}
