#include "Application.hpp"

#include <windows.h>

namespace vulkan
{
extern Application& GetApplication();
}

#ifdef _DEBUG
int main(int argc, char** argv)
#else
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
#endif
{
	vulkan::GetApplication().main();
	return 0;
}
