#pragma once

#include <vkutil/vkutil.h>

#include "config.h"

enum
{
    VKUTIL_LUNARG_VALIDATION = VKUTIL_NUM_LAYERS,
    VKUTIL_TOTAL_LAYERS,
    VKUTIL_LAYERS_BEGIN = 0
};

enum
{
    VKUTIL_KHR_SURFACE_IMPL = VKUTIL_NUM_EXTENSIONS,
    VKUTIL_TOTAL_EXTENSIONS,
    VKUTIL_EXT_NONE = -1
};

#if defined(VK_USE_PLATFORM_WIN32_KHR)
#define VERIFY_TRUE(C) if (!(C)) { \
    OutputDebugString("FAILED: " #C "\r\n"); \
    if (MessageBox(NULL,"FAILED: " #C "\r\nDEBUG?","ERROR",MB_YESNO)==IDYES) DebugBreak(); \
}
#define ASSERT_TRUE(C) if (!(C)) { \
    OutputDebugString("FAILED: " #C "\r\n"); \
    if (MessageBox(NULL,"FAILED: " #C "\r\nDEBUG?","ERROR",MB_YESNO)==IDYES) DebugBreak(); else ExitProcess(-1); \
}
#endif

#if _DEBUG
#define VKTEST(C) ASSERT_TRUE((C)==VK_SUCCESS)
#define TEST(C)   ASSERT_TRUE((C))
#define VERIFY(C) VERIFY_TRUE((C))
#else
#define VKTEST(C) (C)
#define TEST(C)   (C)
#define VERIFY(C) 
#endif

typedef struct VkUtilLayerInfo
{
    const char* layerName;
    u32         layerNameHash;
    bool        isAvailable;
} VkUtilLayerInfo;

typedef struct VkUtilExtensionInfo
{
    const char* extensionName;
    u32         extensionNameHash;
    i32         implementationExt;
    bool        isAvailable;
} VkUtilExtensionInfo;

VKUTIL_API VkUtilLayerInfo        gVkLayer[];
VKUTIL_API VkUtilExtensionInfo    gVkExtension[];

VKUTIL_API VkAllocationCallbacks* gVkAlloc;
VKUTIL_API VkInstance             gVkInstance;

VKUTIL_API u32  FNV1A_U32(const char* str);
VKUTIL_API void LoadVulkanLibrary(void);
VKUTIL_API void LoadInstanceExtensionsAndLayers(void);
VKUTIL_API u32  AddExtensions(const char** list, const bool* req, i32 from, i32 to);
VKUTIL_API u32  AddLayers(const char** list, const bool* req);
VKUTIL_API void LoadInstanceFunctions(void);
