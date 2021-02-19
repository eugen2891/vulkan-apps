#include "application.h"

#if defined(VK_USE_PLATFORM_WIN32_KHR)

#include <stdio.h>

#include <windows.h>

#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32")

static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT ret = 0;
    switch (msg)
    {
    case WM_GETMINMAXINFO:
        break;
    case  WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        ret = DefWindowProc(hWnd, msg, wParam, lParam);
        break;
    }
    return ret;
}

bool vkutil::Application::CreateWindowSurface(void* pWindow)
{
    PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vkGetInstanceProcAddr(m_vkInstance, "vkCreateWin32SurfaceKHR"));
    if (vkCreateWin32SurfaceKHR)
    {
        VkWin32SurfaceCreateInfoKHR info{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
        info.hwnd = reinterpret_cast<HWND>(pWindow);
        info.hinstance = GetModuleHandle(nullptr);
        VKUTIL_CHECK_RETURN(vkCreateWin32SurfaceKHR(m_vkInstance, &info, m_pVkAlloc, &m_vkSurface), false);
        return true;
    }
    return false;
}

bool vkutil::CheckResult(VkResult result, const char* pFile, int lineNo)
{
    if (result != VK_SUCCESS)
    { 
        char msgBuff[64];
        snprintf(msgBuff, sizeof(msgBuff), "ERR: %d (%s:%d)\n", result, pFile, lineNo);
        OutputDebugString(msgBuff);
        if (IsDebuggerPresent())
            __debugbreak();
        return false;
    }
    return true;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
    HMODULE vkDLL = LoadLibrary("vulkan-1");
    if (!vkDLL)
        return -1;
    vkutil::Functions::vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(vkDLL, "vkGetInstanceProcAddr"));
    if (!vkutil::Functions::vkGetInstanceProcAddr)
        return -1;

    static const char* pWndClss = "VKUTIL_WNDCLSS";
    vkutil::Application* pApp = CreateApplication();
    if (!pApp)
        return -1;
    
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_CLASSDC;
    wc.lpfnWndProc = &WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = pWndClss;
    if (FAILED(RegisterClassEx(&wc)))
        return -1;

    int windowW = 0, windowH = 0;
    pApp->GetWindowSize(windowW, windowH);
    RECT wr = { 0, 0, windowW, windowH };
    DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&wr, style, FALSE);
    wr.right -= wr.left;
    wr.bottom -= wr.top;

    HWND hWnd = CreateWindow(pWndClss, pApp->GetWindowTitle(), style, wr.left, wr.top, wr.right, wr.bottom, nullptr, nullptr, hInstance, nullptr);
    if (!hWnd)
        return -1;

    MONITORINFO mInfo = {};
    mInfo.cbSize = sizeof(mInfo);
    HMONITOR monitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTOPRIMARY);
    GetMonitorInfo(monitor, &mInfo);
    wr.left = ((mInfo.rcMonitor.right - mInfo.rcMonitor.left) - wr.right) / 2;
    wr.top = ((mInfo.rcMonitor.bottom - mInfo.rcMonitor.top) - wr.bottom) / 2;
    MoveWindow(hWnd, wr.left, wr.top, wr.right, wr.bottom, TRUE);
    ShowWindow(hWnd, SW_SHOWNORMAL);

    if (pApp->Initialize(hWnd))
    {
        LARGE_INTEGER freq, tPrev;
        QueryPerformanceCounter(&tPrev);        
        QueryPerformanceFrequency(&freq); 
        const float ratio = 1.f / static_cast<float>(freq.QuadPart);
        SetWindowText(hWnd, pApp->GetWindowTitle());

        LARGE_INTEGER tStart = tPrev, tNow = {};
        for (MSG msg = {}; msg.message != WM_QUIT;)
        {
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                continue;
            }

            QueryPerformanceCounter(&tNow);
            float deltaT = (tNow.QuadPart - tPrev.QuadPart) * ratio;
            float elapsedT = (tNow.QuadPart - tStart.QuadPart) * ratio;
            pApp->NextFrame(elapsedT, deltaT);
            tPrev = tNow;
        }

        pApp->Finalize();
    }

    UnregisterClass(pWndClss, hInstance);
    DestroyApplication(pApp);

    return 0;
}

#endif
