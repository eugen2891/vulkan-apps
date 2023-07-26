#pragma once

void resetSwapchain(void)
{
	if (Swapchain != VK_NULL_HANDLE)
	{
		deviceWaitIdle();
		destroySwapchain(true);
	}

	uint32_t numSurfaceFormats = 0;
	VkSurfaceFormatKHR surfaceFormat = { VK_FORMAT_UNDEFINED };
	breakIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &numSurfaceFormats, NULL));
	VkSurfaceFormatKHR* surfaceFormats = malloc(numSurfaceFormats * sizeof(VkSurfaceFormatKHR));
	breakIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(PhysicalDevice, Surface, &numSurfaceFormats, surfaceFormats));
	for (uint32_t i = 0; i < numSurfaceFormats; i++)
	{
		const VkSurfaceFormatKHR* sf = &surfaceFormats[i];
		if (sf->format == SwapchainColorTarget)
		{
			surfaceFormat = *sf;
			break;
		}
	}
	freeMem(surfaceFormats);

	breakIfNot(surfaceFormat.format != VK_FORMAT_UNDEFINED);

	uint32_t numPresentModes = 0;
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	breakIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &numPresentModes, NULL));
	VkPresentModeKHR* presentModes = malloc(numPresentModes * sizeof(VkPresentModeKHR));
	breakIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(PhysicalDevice, Surface, &numPresentModes, presentModes));
	for (uint32_t i = 0; i < numPresentModes; i++)
	{
		if (presentModes[i] == PresentMode)
		{
			presentMode = presentModes[i];
			break;
		}
	}
	freeMem(presentModes);

	VkSurfaceCapabilitiesKHR caps;
	breakIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(PhysicalDevice, Surface, &caps));
	uint32_t imageCount = (caps.minImageCount > SwapchainLength) ? caps.minImageCount : SwapchainLength;
	imageCount = (imageCount > caps.maxImageCount) ? caps.maxImageCount : imageCount;

	VkSwapchainKHR prevHandle = Swapchain;
	VkSwapchainCreateInfoKHR sci = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = Surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = caps.currentExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.preTransform = caps.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = prevHandle
	};
	breakIfFailed(vkCreateSwapchainKHR(Device, &sci, Alloc, &Swapchain));
	vkDestroySwapchainKHR(Device, prevHandle, Alloc);

	const VkExtent3D extent = { sci.imageExtent.width, sci.imageExtent.height, 1 };

	if (SwapchainDepthBuffer != VK_FORMAT_UNDEFINED)
	{
		SwapchainDepthImage = createRenderTargetImage(SwapchainDepthBuffer, &extent);
	}

	VkImage* imageHandles = NULL;
	breakIfFailed(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainLength, NULL));
	safeRealloc(imageHandles, SwapchainLength * sizeof(VkImage));
	safeRealloc(SwapchainImages, SwapchainLength * sizeof(struct ImageT));
	safeRealloc(SwapchainFramebuffers, SwapchainLength * sizeof(Framebuffer));
	safeRealloc(SwapchainSemaphores, SwapchainLength * sizeof(VkSemaphore));
	breakIfFailed(vkGetSwapchainImagesKHR(Device, Swapchain, &SwapchainLength, imageHandles));
	for (uint32_t i = 0; i < SwapchainLength; i++)
	{	
		(SwapchainImages + i)->handle = *(imageHandles + i);
		Image renderTargets[] = { &SwapchainImages[i], SwapchainDepthImage };
		initImage(&SwapchainImages[i], sci.imageFormat, &extent, 1, false, false);
		*(SwapchainFramebuffers + i) = createFramebuffer(SwapchainRenderPass, renderTargets);
		VkSemaphoreCreateInfo ci = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
		breakIfFailed(vkCreateSemaphore(Device, &ci, Alloc, &SwapchainSemaphores[i]));
	}
	freeMem(imageHandles);

	VkSemaphoreCreateInfo ci = { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	breakIfFailed(vkCreateSemaphore(Device, &ci, Alloc, &SwapchainNextSemaphore));
}

Framebuffer getSwapchainFramebuffer(void)
{
	if (!SwapchainCurrentImage)
	{
		VkSemaphore semaphore = SwapchainNextSemaphore;
		VkResult result = vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, semaphore, VK_NULL_HANDLE, &SwapchainCurrentIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			return NULL;
		}
		else
		{
			breakIfFailed(result);
		}
		//present queue
		SwapchainCurrentImage = &SwapchainImages[SwapchainCurrentIndex];
		SwapchainNextSemaphore = SwapchainSemaphores[SwapchainCurrentIndex];
		SwapchainSemaphores[SwapchainCurrentIndex] = semaphore;
	}
	return SwapchainFramebuffers[SwapchainCurrentIndex];
}

void presentImageToWindow(void)
{
	uint32_t waitCount = (SwapchainUpdateSubmit) ? 1 : 0;
	VkPresentInfoKHR pi = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, 
		.waitSemaphoreCount = waitCount, 
		.pWaitSemaphores = SwapchainUpdateSubmit,
		.swapchainCount = 1,
		.pSwapchains = &Swapchain,
		.pImageIndices = &SwapchainCurrentIndex
	};
	VkQueue queueHandle = QueueContext[PresentQueue].queueHandle;
	VkResult result = vkQueuePresentKHR(queueHandle, &pi);
	if (result != VK_SUCCESS && result != VK_ERROR_OUT_OF_DATE_KHR)
	{
		breakIfFailed(result);
	}
	SwapchainUpdateSubmit = NULL;
	SwapchainCurrentImage = NULL;
}

void destroySwapchain(bool reset)
{
	for (uint32_t i = 0; i < SwapchainLength; i++)
	{
		vkDestroySemaphore(Device, SwapchainSemaphores[i], Alloc);
		vkDestroyImageView(Device, SwapchainImages[i].view, Alloc);
		destroyFramebuffer(SwapchainFramebuffers[i]);
	}
	vkDestroySemaphore(Device, SwapchainNextSemaphore, Alloc);
	destroyImage(SwapchainDepthImage);
	if (!reset)
	{
		vkDestroySwapchainKHR(Device, Swapchain, Alloc);
		freeMem(SwapchainSemaphores);
		freeMem(SwapchainFramebuffers);
		freeMem(SwapchainImages);
	}
}

RenderPass getSwapchainRenderPass(void)
{
	return SwapchainRenderPass;
}

