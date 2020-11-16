#include <vulkangui.h>

#include <GLFW/glfw3.h>

#define WIN_W 1280
#define WIN_H 800
#define APP_NAME "VulkanInfo"

static void RenderScene(const VulkanAPI& vk)
{
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    int glfwOk = glfwInit();
    if (glfwOk == GLFW_TRUE)
    {
        VulkanAPI vkApi(APP_NAME, true);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* pWindow = glfwCreateWindow(WIN_W, WIN_H, APP_NAME, nullptr, nullptr);
        glfwSetWindowSizeLimits(pWindow, WIN_W, WIN_H, GLFW_DONT_CARE, GLFW_DONT_CARE);
        if (vkApi.CreateDeviceAndSwapchain(pWindow))
        {
            VulkanImGuiRenderer imGui;
            while (!vkApi.HasError() && !glfwWindowShouldClose(pWindow))
            {
                glfwPollEvents();
                vkApi.BeginFrame();
                RenderScene(vkApi);
                //imGui.RenderAll();
                vkApi.EndFrame();
            }
            //vkApi.Finalize();
        }
    }
    glfwTerminate();
    return 0;
}
