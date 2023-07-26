#pragma once

struct FramebufferT
{
	VkFramebuffer handle;
	VkViewport viewport;
	VkRect2D scissor;
};

Framebuffer createFramebuffer(RenderPass renderPass, Image* images)
{
	if (!renderPass || !images)
	{
		return NULL;
	}

	const uint32_t numImages = renderPass->numColor + renderPass->numDepth;
	VkImageView* imageViews = malloc(numImages * sizeof(VkImageView));
	if (!imageViews)
	{
		breakIfNot(0);
		return NULL;
	}

	for (uint32_t i = 0; i < numImages; i++)
	{
		*(imageViews + i) = images[i]->view;
	}

	VkExtent3D size = (*images)->size;
	VkFramebufferCreateInfo fbci = {
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = getRenderPassHandle(renderPass),
		.attachmentCount = numImages,
		.pAttachments = imageViews,
		.width = size.width,
		.height = size.height,
		.layers = 1
	};
	VkFramebuffer handle = VK_NULL_HANDLE;
	breakIfFailed(vkCreateFramebuffer(Device, &fbci, Alloc, &handle));
	freeMem(imageViews);

	Framebuffer retval = calloc(1, sizeof(struct FramebufferT));
	if (retval)
	{
		retval->handle = handle;
		retval->scissor.extent.width = size.width;
		retval->scissor.extent.height = size.height;
		retval->viewport.width = (float)size.width;
		retval->viewport.height = (float)size.height;
		retval->viewport.maxDepth = 1.f;
	}
	return retval;
}

void destroyFramebuffer(Framebuffer framebuffer)
{
	if (framebuffer)
	{
		vkDestroyFramebuffer(Device, framebuffer->handle, Alloc);
		freeMem(framebuffer);
	}
}
