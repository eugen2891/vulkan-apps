#include "Application.hpp"

#if !defined(_WIN32)
#if defined(WIN32_GUI_ONLY)
#undef WIN32_GUI_ONLY
#endif
#else
#include <windows.h>
#endif

#if !defined(WIN32_GUI_ONLY)

#include <cstdio>

#define dbgPrint printf

#else

#include <cstdarg>

void dbgPrint(const char* format, ...)
{
#error Not implemented yet
	if (IsDebuggerPresent())
	{
		OutputDebugString(nullptr);
	}
}

#endif

namespace vulkan
{
extern Application& GetApplication();
}

#if !defined(WIN32_GUI_ONLY)
int main(int argc, char** argv)
#else
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#endif
{
	vulkan::GetApplication().main();
	return 0;
}
