#pragma once

#include "VulkanApi.hpp"

#include "../Utilities/Array.hpp"

struct SDL_Window;

namespace vulkan
{

class EventHandler;

class Window : public APIClient
{
public:
	bool pollEvents();
	size_t numBuffers() const;
	bool swapchainValid() const;
	VkSurfaceKHR surface() const;
	void setEventHandler(EventHandler* handler);
	explicit Window(const APIState& vk, const char* title, VkFormat pixelFormat, VkExtent2D size);
	void present(const Range<VkSemaphore>& waitFor);
	VkSemaphore currentSemaphore();
	VkImageView currentImageView();
	VkFormat pixelFormat() const;
	float aspectRatio() const;
	SDL_Window* sdlWindow();
	VkImage currentImage();
	VkRect2D rect() const;
private:
	friend class Application;
	void createWindowAndSurface();
	void setPresentQueue(VkQueue presentQueue);
	void destroySwapchain();
	void destroyWindowAndSurface();
	void updateSwapchain();
	uint32_t currentIndex();
	SDL_Window* m_window = nullptr;
	EventHandler* m_eventHandler = nullptr;
	VkSurfaceKHR m_surface = VK_NULL_HANDLE;
	VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
	VkQueue m_presentQueue = VK_NULL_HANDLE;
	uint32_t m_currentImage = UINT32_MAX;
	uint32_t m_currentSemaphore = 0;
	bool m_shouldRecreate = true;
	std::vector<VkSemaphore> m_semaphores;
	std::vector<VkImageView> m_imageViews;
	std::vector<VkImage> m_images;
	const char* m_title;
	VkFormat m_pixelFormat;
	VkExtent2D m_size;
};

}

