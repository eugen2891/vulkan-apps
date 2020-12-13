#pragma once

#if defined(_WIN32)

struct IUnknown;
#define VK_USE_PLATFORM_WIN32_KHR

#if (_DEBUG)
#define DBGBREAK() __debugbreak()
#else
#define DBGBREAK()
#endif

#else

#error Unsuported platform

#endif
