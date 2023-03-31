#include <GlobalPCH.hpp>

#include "Window.hpp"
#include "InitHelpers.hpp"
#include "EventHandler.hpp"

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#if defined(_MSC_VER)
#pragma comment(lib, "SDL2")
#endif

#if !defined(VULKAN_SWAPCHAIN_MIN_SIZE)
#define VULKAN_SWAPCHAIN_MIN_SIZE 3
#endif

#define DispatchEvent(expr) \
	for (EventHandler* h = m_eventHandler; h; h = h->m_next) \
	if (h->m_enabled) { if (!(h->##expr)) break; }

bool vulkan::Window::pollEvents()
{
	updateSwapchain();
	SDL_Event event;
	bool isActive = true;
	if (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
			isActive = SDL_FALSE;
			break;
		default:
			DispatchEvent(onWindowEvent(*this, event));
			break;
		}
	}
	return isActive;
}

size_t vulkan::Window::numBuffers() const
{
	BreakIfNot(m_images.size() > 0);
	return m_images.size();
}

bool vulkan::Window::swapchainValid() const
{
	return !m_shouldRecreate;
}

vulkan::Window::Window(const APIState& vk, const char* title, VkFormat pixelFormat, VkExtent2D size)
	: APIClient(vk), m_title(title), m_pixelFormat(pixelFormat), m_size(size)
{
}

VkSurfaceKHR vulkan::Window::surface() const
{
	return m_surface;
}

void vulkan::Window::setEventHandler(EventHandler* handler)
{
	if (!m_eventHandler) m_eventHandler = handler;
}

void vulkan::Window::present(const Range<VkSemaphore>& waitFor)
{
	VkPresentInfoKHR pi
	{
		VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, nullptr, uint32_t(waitFor.num()), waitFor.get(), 1,& m_swapchain,& m_currentImage
	};
	VkResult presentResult = vkQueuePresentKHR(m_presentQueue, &pi);
	switch (presentResult)
	{
	case VK_ERROR_OUT_OF_DATE_KHR:
		m_shouldRecreate = true;
		[[fallthrough]];
	case VK_SUCCESS:
		m_currentImage = UINT32_MAX;
		break;
	default:
		BreakIfFailed(presentResult);
		break;
	}
}

VkSemaphore vulkan::Window::currentSemaphore()
{
	return m_semaphores[m_currentSemaphore];
}

VkImageView vulkan::Window::currentImageView()
{
	return m_imageViews[currentIndex()];
}

VkFormat vulkan::Window::pixelFormat() const
{
	return m_pixelFormat;
}

float vulkan::Window::aspectRatio() const
{
	return float(m_size.width) / float(m_size.height);
}

SDL_Window* vulkan::Window::sdlWindow()
{
	return m_window;
}

VkImage vulkan::Window::currentImage()
{
	return m_images[currentIndex()];
}

VkRect2D vulkan::Window::rect() const
{
	return { { 0, 0 }, m_size };
}

void vulkan::Window::createWindowAndSurface()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	uint32_t windowFlags = SDL_WINDOW_VULKAN;
	m_window = SDL_CreateWindow(m_title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_size.width, m_size.height, windowFlags);
	ReturnIfNot(m_window);

	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(m_window, &wmInfo);

#if defined(_WIN32)	
	const VkWin32SurfaceCreateInfoKHR sci
	{
		VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR, nullptr, 0, wmInfo.info.win.hinstance, wmInfo.info.win.window
};
	BreakIfFailed(vkCreateWin32SurfaceKHR(m_vk.instance(), &sci, m_vk.alloc(), &m_surface));
#else
#error Unsupported platform
#endif
}

void vulkan::Window::setPresentQueue(VkQueue presentQueue)
{
	m_presentQueue = presentQueue;
}

void vulkan::Window::destroySwapchain()
{
	for (auto i = 0; i < m_images.size(); i++)
	{
		vkDestroySemaphore(m_vk.device(), m_semaphores[i], m_vk.alloc());
		vkDestroyImageView(m_vk.device(), m_imageViews[i], m_vk.alloc());
	}
	vkDestroySwapchainKHR(m_vk.device(), m_swapchain, m_vk.alloc());
}

void vulkan::Window::destroyWindowAndSurface()
{
	vkDestroySurfaceKHR(m_vk.instance(), m_surface, m_vk.alloc());
	SDL_DestroyWindow(m_window);
}

void vulkan::Window::updateSwapchain()
{
	if (!m_shouldRecreate) return;

	VkSwapchainCreateInfoKHR sci
	{
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR, nullptr, 0, m_surface, 0, VK_FORMAT_UNDEFINED
	};

	VkSurfaceCapabilitiesKHR caps;
	ReturnIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vk.physicalDevice(), m_surface, &caps));
	sci.minImageCount = Min(Max(VULKAN_SWAPCHAIN_MIN_SIZE, caps.minImageCount), caps.maxImageCount);
	sci.preTransform = caps.currentTransform;
	sci.imageExtent = caps.currentExtent;
	m_size = caps.currentExtent;

	for (VkSurfaceFormatKHR& format : m_vk.enumSurfaceFormats(m_surface))
	{
		if (format.format == m_pixelFormat)
		{
			sci.imageFormat = format.format;
			sci.imageColorSpace = format.colorSpace;
			break;
		}
	}

	ReturnIfNot(sci.imageFormat != VK_FORMAT_UNDEFINED);

	sci.imageArrayLayers = 1;
	sci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	sci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	sci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	for (VkPresentModeKHR presentMode : m_vk.enumPresentModes(m_surface))
	{
		if (presentMode == VK_PRESENT_MODE_FIFO_KHR)
		{
			sci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
			break;
		}
	}

	sci.oldSwapchain = m_swapchain;

	ReturnIfFailed(vkCreateSwapchainKHR(m_vk.device(), &sci, m_vk.alloc(), &m_swapchain));
	m_images = m_vk.getSwapchainImages(m_swapchain);
	m_semaphores.resize(m_images.size());
	m_imageViews.resize(m_images.size());
	for (auto i = 0; i < m_images.size(); i++)
	{
		VkSemaphoreCreateInfo sci{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0 };
		ReturnIfFailed(vkCreateSemaphore(m_vk.device(), &sci, m_vk.alloc(), &m_semaphores[i]));
		
		vulkan::Image2DViewCreateInfo ivci{ m_images[i], m_pixelFormat, {}, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 } };
		ReturnIfFailed(vkCreateImageView(m_vk.device(), &ivci, m_vk.alloc(), &m_imageViews[i]));
	}
	
	DispatchEvent(onSwapchainReset(*this));
	m_currentSemaphore = uint32_t(m_images.size() - 1);
	m_currentImage = UINT32_MAX;
	m_shouldRecreate = false;
}

uint32_t vulkan::Window::currentIndex()
{
	if (m_currentImage == UINT32_MAX)
	{
		m_currentSemaphore = (m_currentSemaphore + 1) % m_semaphores.size();
		VkResult acquireResult = vkAcquireNextImageKHR(m_vk.device(), m_swapchain, UINT64_MAX, m_semaphores[m_currentSemaphore], VK_NULL_HANDLE, &m_currentImage);
		switch (acquireResult)
		{
		case VK_ERROR_OUT_OF_DATE_KHR:
			m_shouldRecreate = true;
			m_currentImage = 0;
			break;
		case VK_SUCCESS:
			break;
		default:
			BreakIfFailed(acquireResult);
			break;
		}
	}
	return m_currentImage;
}
