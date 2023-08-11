#pragma once

struct ImageT
{
	VkImage handle;
	VkDeviceMemory memory;
	VkImageView view;
	VkFormat format;
	VkExtent3D size;
	uint32_t mips;
	VkImageAspectFlags aspect;
};

static void initImage(Image image, VkFormat format, const VkExtent3D* size, uint32_t numMips, bool isCube, bool alloc)
{
	breakIfNot(image->handle);
	if (alloc)
	{
		VkMemoryRequirements memReq;
		vkGetImageMemoryRequirements(Device, image->handle, &memReq);
		VkMemoryPropertyFlags memReqired = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		VkMemoryPropertyFlags memExcluded = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		VkMemoryAllocateInfo mai = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memReq.size,
			.memoryTypeIndex = findMemoryType(&memReq, memReqired, memExcluded, 0)
		};
		breakIfFailed(vkAllocateMemory(Device, &mai, Alloc, &image->memory));
		vkBindImageMemory(Device, image->handle, image->memory, 0);
	}
	else
	{
		image->memory = VK_NULL_HANDLE;
	}

	VkImageAspectFlags aspect = 0;
	switch (format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
	case VK_FORMAT_D32_SFLOAT:
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		break;
	default:
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		break;
	}

	VkImageViewType type = (isCube) ? VK_IMAGE_VIEW_TYPE_CUBE : VK_IMAGE_VIEW_TYPE_2D;
	if (size->depth > 1)
	{
		type = VK_IMAGE_VIEW_TYPE_3D;
	}

	VkImageViewCreateInfo ivci = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image->handle,
		.viewType = type,
		.format = format,
		.subresourceRange = { 
			.aspectMask = aspect,
			.levelCount = numMips,
			.layerCount = 1
		}
	};
	breakIfFailed(vkCreateImageView(Device, &ivci, Alloc, &image->view));

	image->aspect = aspect;
	image->format = format;
	image->mips = numMips;
	image->size = *size;
}

Image createRenderTargetImage(VkFormat format, const VkExtent3D* size)
{
	VkImageUsageFlags usage = 0;
	switch (format)
	{
	case VK_FORMAT_D16_UNORM:
	case VK_FORMAT_D16_UNORM_S8_UINT:
	case VK_FORMAT_D24_UNORM_S8_UINT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
	case VK_FORMAT_D32_SFLOAT:
		usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	default:
		usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	}
	VkImageCreateInfo ici = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = *size,
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage
	};

	VkImage handle = VK_NULL_HANDLE;
	breakIfFailed(vkCreateImage(Device, &ici, Alloc, &handle));

	Image retval = calloc(1, sizeof(struct ImageT));
	breakIfNot(retval);
	if (!retval)
	{
		vkDestroyImage(Device, handle, Alloc);
		return NULL;
	}

	retval->handle = handle;
	initImage(retval, format, size, 1, false, true);
	return retval;
}

Image createSampledImage(VkFormat format, const VkExtent3D* size, uint32_t numMips)
{
	VkImageCreateInfo ici = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = *size,
		.mipLevels = numMips,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
	};

	VkImage handle = VK_NULL_HANDLE;
	breakIfFailed(vkCreateImage(Device, &ici, Alloc, &handle));

	Image retval = calloc(1, sizeof(struct ImageT));
	breakIfNot(retval);
	if (!retval)
	{
		vkDestroyImage(Device, handle, Alloc);
		return NULL;
	}

	retval->handle = handle;
	initImage(retval, format, size, 1, false, true);
	return retval;
}

void destroyImage(Image image)
{
	if (image)
	{
		vkDestroyImageView(Device, image->view, Alloc);
		vkDestroyImage(Device, image->handle, Alloc);
		vkFreeMemory(Device, image->memory, Alloc);
		freeMem(image);
	}
}
