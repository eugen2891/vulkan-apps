#include "global.h"

#include "renderer.h"

#include <vkutil.h>
#include <vecmath.h>

#include <GLFW/glfw3.h>

#define WIN_W 1280
#define WIN_H 800
#define APP_NAME "SBRenderer"

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    int glfwOk = glfwInit();
    if (glfwOk == GLFW_TRUE)
    {
        vkUtilInitialize(APP_NAME, nullptr, true);
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        GLFWwindow* pWindow = glfwCreateWindow(WIN_W, WIN_H, APP_NAME, nullptr, nullptr);
        glfwSetWindowSizeLimits(pWindow, WIN_W, WIN_H, GLFW_DONT_CARE, GLFW_DONT_CARE);
        vkUtilCreateDevice(pWindow, false /*VKUTIL_COMMAND_CONTEXT | VKUTIL_DEPTH_STENCIL_BUFFER*/);
        if (!vkUtilHasError())
        {
            sb::Initialize();
            while (!vkUtilHasError() && !glfwWindowShouldClose(pWindow))
            {
                glfwPollEvents();
                vkUtilBeginFrame();
                sb::RenderMainUI();
                sb::RenderScene();
                sb::RenderOverlay();
                vkUtilEndFrame();
            }
            sb::Finalize();
        }
        vkUtilFinalize();
    }
    glfwTerminate();
    return 0;
}
