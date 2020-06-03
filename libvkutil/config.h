#pragma once

/* Force default validation layer in debug mode */
#define VKUTIL_FORCE_DEBUG_VALIDATION 1

/* Default version of Vulkan API */
#define VKUTIL_DEFAULT_VULKAN_API  VK_MAKE_VERSION(1, 0, 0)

/* Maximum number of GPU queues per application */
#define VKUTIL_MAX_DEVICE_QUEUES  8

/* Minimum number of images in a swapchain */
#define VKUTIL_MIN_SWAPCHAIN_SIZE 2

/* Maximum number of images in a swapchain */
#define VKUTIL_MAX_SWAPCHAIN_SIZE 4

/* Default number of images in a swapchain */
#define VKUTIL_DEFAULT_SWAPCHAIN_SIZE 3

/* Default width of a window, if not specified */
#define VKUTIL_DEFAULT_WINDOW_W 1024

/* Default height of a window, if not specified */
#define VKUTIL_DEFAULT_WINDOW_H 768

/* Default format of window surface */
#define VKUTIL_DEFAULT_WINDOW_FORMAT VK_FORMAT_B8G8R8A8_UNORM
