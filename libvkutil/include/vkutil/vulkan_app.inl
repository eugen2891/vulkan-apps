#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "kernel32")
#pragma comment(lib, "user32"  )
#pragma comment(lib, "gdi32"   )
#pragma comment(lib, "shell32" )
#endif

#include <vkutil/mem_stack.h>
#include <vkutil/vulkan_app.h>
#include <GLFW/glfw3.h>

/* Minimum 64KB internal stack memory */
#if !defined(VKAPI_TMP_MEM) || (VKAPI_TMP_MEM < 65536)
#if defined(VKAPI_TMP_MEM)
#undef VKAPI_TMP_MEM
#endif
#define VKAPI_TMP_MEM 65536
#endif

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
