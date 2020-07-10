#include "internal.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)

static const char* VKUTIL_WNDCLASS = "VKUTIL_WNDLASS";

extern int main(int agrc, const char* argv[]);

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE prev, LPSTR cmd, int show)
{
    char name[256] = { 0 };
    const char* argv[] = { name };
    GetModuleFileName(NULL, name, sizeof(name));
    return main(1, argv);
}

static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT retVal = 0;
    switch (msg)
    {
    case WM_CLOSE:
        DestroyWindow(hWnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_GETMINMAXINFO:
        {
            MONITORINFO monitorInfo = { 0 };
            HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
            monitorInfo.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(monitor, &monitorInfo);
            ((MINMAXINFO *)lParam)->ptMaxSize.x = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
            ((MINMAXINFO *)lParam)->ptMaxSize.y = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;
            ((MINMAXINFO *)lParam)->ptMaxPosition.x = 0;
            ((MINMAXINFO *)lParam)->ptMaxPosition.y = 0;
        }
        break;
    default:
        retVal = DefWindowProc(hWnd, msg, wParam, lParam);
        break;
    }
    return retVal;
}

void CreateSurface(const VkUtilInitOptions* opts)
{
    HINSTANCE inst = (opts->win32HInstance) ? opts->win32HInstance : GetModuleHandle(NULL);
    if (!opts->win32HWnd)
    {
        WNDCLASSEX wc = { 0 };
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_CLASSDC;
        wc.lpfnWndProc = (WNDPROC)&WndProc;
        wc.hInstance = inst;
        wc.lpszClassName = VKUTIL_WNDCLASS;
        TEST(RegisterClassEx(&wc));
        DWORD style = (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX);
        int width = (opts->windowWidth < VKUTIL_DEFAULT_WINDOW_W) ? VKUTIL_DEFAULT_WINDOW_W : opts->windowWidth;
        int height = (opts->windowHeight < VKUTIL_DEFAULT_WINDOW_H) ? VKUTIL_DEFAULT_WINDOW_H : opts->windowHeight;
        HWND hWnd = CreateWindow(VKUTIL_WNDCLASS, opts->programName, style, 0, 0, width, height, NULL, NULL, inst, NULL);
        TEST(hWnd);
        if (opts->windowFlags & VKUTIL_WINDOW_CENTERED)
        {
            MONITORINFO monitorInfo = { 0 };
            HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
            monitorInfo.cbSize = sizeof(MONITORINFO);
            GetMonitorInfo(monitor, &monitorInfo);
            i32 x = (monitorInfo.rcMonitor.right - monitorInfo.rcMonitor.left - width) / 2;
            i32 y = (monitorInfo.rcMonitor.bottom - monitorInfo.rcMonitor.top - height) / 2;
            SetWindowPos(hWnd, HWND_TOP, x, y, width, height, SWP_NOSIZE);
        }
        VkWin32SurfaceCreateInfoKHR info = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
        info.hinstance = inst;
        info.hwnd = hWnd;
        VKTEST(vkCreateWin32SurfaceKHR(gVkInstance, &info, gVkAlloc, &gVkSurface));
        ShowWindow(hWnd, (opts->windowFlags & VKUTIL_WINDOW_MAXIMIZED) ? SW_SHOWMAXIMIZED : SW_SHOWNORMAL);
        UpdateWindow(hWnd);
    }
    else
    {
        VkWin32SurfaceCreateInfoKHR info = { VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
        info.hinstance = inst;
        info.hwnd = opts->win32HWnd;
        VKTEST(vkCreateWin32SurfaceKHR(gVkInstance, &info, gVkAlloc, &gVkSurface));
    }
}

bool vkUtilIsWindowClosed(void)
{
    MSG msg;
    if (gVkOptions.win32HWnd)
    {
        return false;
    }
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        if (msg.message == WM_QUIT)
            return true;
        else
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    return false;
}

void DestroySurface()
{
    vkDestroySurfaceKHR(gVkInstance, gVkSurface, gVkAlloc);
    if (!gVkOptions.win32HWnd)
    {
        UnregisterClass(VKUTIL_WNDCLASS, GetModuleHandle(NULL));
    }
}

#endif
