#pragma once

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#ifdef _DEBUG

#define returnIfNot(expr) { if (!(expr)) { __debugbreak(); return; } }
#define retvalIfNot(expr, val) { if (!(expr)) { __debugbreak(); return (val); } }
#define breakIfNot(expr) { if (!(expr)) { __debugbreak(); abort(); } }

#define debugPrint printf

#if !defined(WITH_RENDERDOC)
#define WITH_RENDERDOC 1
#endif

#else

#define returnIfNot(expr) { if (!(expr)) return; }
#define retvalIfNot(expr, val) { if (!(expr)) return (val); }
#define breakIfNot(expr) (expr)

#define debugPrint(...)

#endif

#define returnIfFailed(expr) returnIfNot((expr) == VK_SUCCESS)
#define retvalIfFailed(expr, val) retvalIfNot((expr) == VK_SUCCESS, val)
#define breakIfFailed(expr) breakIfNot((expr) == VK_SUCCESS)

#define freeMem(ptr) do { void* tmp = ptr; if (tmp) free(tmp); ptr = NULL; } while (0)

#define safeRealloc(ptr, s) do { \
	void* tmp = realloc(ptr, s); if (tmp) ptr = tmp; else breakIfNot(tmp); \
} while (0)

#if !defined(MAX_UNIFORM_BUFFERS)
#define MAX_UNIFORM_BUFFERS 1
#endif

#if !defined(MAX_SAMPLED_IMAGES)
#define MAX_SAMPLED_IMAGES 1
#endif

#if !defined(MAX_PUSH_CONST_BYTES)
#define MAX_PUSH_CONST_BYTES 0
#endif

#if !defined(MAX_RESOURCE_BARRIERS)
#define MAX_RESOURCE_BARRIERS 8
#endif

#if !defined(MAX_DRAW_CALLS)
#define MAX_DRAW_CALLS 1024
#endif

#define UB_BINDING_OFFSET 0
#define SI_BINDING_OFFSET (VKK_MAX_UNIFORM_BUFFERS)

#define MAX_SHADER_BINDINGS ((MAX_UNIFORM_BUFFERS) + (MAX_SAMPLED_IMAGES))

#ifndef MAX_INSTANCE_EXTENSIONS
#define MAX_INSTANCE_EXTENSIONS 8
#endif

#ifndef MAX_DEVICE_EXTENSIONS
#define MAX_DEVICE_EXTENSIONS 8
#endif