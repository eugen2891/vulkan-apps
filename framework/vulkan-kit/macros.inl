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

#define ImageSubset uint32_t

#define makeImageSubset(fromMip, numMips, fromLayer, numLayers) \
	(((fromMip) & 0x0000000Fu) | (((numMips) << 4) & 0x000000F0u) \
	| (((fromLayer) << 8) & 0x000FFF00u) | (((numLayers) << 20) & 0xFFF00000u))
#define imageSubsetNumLayers(subset) (((subset) >> 20) & 0x0000000FFFu)
#define imageSubsetFromLayer(subset) (((subset) >> 8) & 0x0000000FFFu)
#define imageSubsetNumMips(subset) (((subset) >> 4) & 0x0000000Fu)
#define imageSubsetFromMip(subset) ((subset) & 0x0000000Fu)

#if !defined(MAX_SAMPLER_STATES)
#define MAX_SAMPLER_STATES 1
#endif

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

#define SS_BINDING_OFFSET (0)
#define UB_BINDING_OFFSET (SS_BINDING_OFFSET) + (MAX_SAMPLER_STATES)
#define SI_BINDING_OFFSET (UB_BINDING_OFFSET) + (MAX_UNIFORM_BUFFERS)

#define MAX_SHADER_BINDINGS ((MAX_SAMPLER_STATES) + (MAX_UNIFORM_BUFFERS) + (MAX_SAMPLED_IMAGES))

#ifndef MAX_INSTANCE_EXTENSIONS
#define MAX_INSTANCE_EXTENSIONS 8
#endif

#ifndef MAX_DEVICE_EXTENSIONS
#define MAX_DEVICE_EXTENSIONS 8
#endif
