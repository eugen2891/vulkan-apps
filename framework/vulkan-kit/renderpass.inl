#pragma once

struct RenderPassT
{
	VkAttachmentDescription* attachment;
	VkSubpassDependency dependencies[2];
	VkAttachmentReference* reference;
	VkClearValue* clearValue;
	uint32_t depthTestWrite;
	uint32_t numColor;
	uint32_t numDepth;
	VkRenderPass handle;
	int numClearValues;
};

RenderPass createRenderPass(uint32_t numColor, uint32_t numDepth)
{
	RenderPass retval = calloc(1, sizeof(struct RenderPassT));
	VkAttachmentReference* attachmentRefs = calloc(numColor + numDepth, sizeof(VkAttachmentReference));
	VkAttachmentDescription* attachmentDesc = calloc(numColor + numDepth, sizeof(VkAttachmentDescription));
	VkClearValue* clearVals = calloc(2 * (numColor + numDepth), sizeof(VkClearValue));
	if (!retval || !attachmentRefs || !attachmentDesc || !clearVals)
	{
		freeMem(attachmentRefs);
		freeMem(attachmentDesc);
		freeMem(clearVals);
		freeMem(retval);
		return NULL;
	}
	else
	{
		retval->attachment = attachmentDesc;
		retval->reference = attachmentRefs;
		retval->clearValue = clearVals;
		retval->numColor = numColor;
		retval->numDepth = numDepth;
		retval->numClearValues = -1;
	}

	for (uint32_t i = 0; i < (numColor + numDepth); i++)
	{
		VkAttachmentReference* ar = attachmentRefs + i;
		VkAttachmentDescription* ad = attachmentDesc + i;
		ar->attachment = i;
		ar->layout = (i < numColor)? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		ad->flags = 0;
		ad->format = VK_FORMAT_UNDEFINED;
		ad->samples = VK_SAMPLE_COUNT_1_BIT;
		ad->loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ad->storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		ad->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		ad->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		ad->initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		ad->finalLayout = ar->layout;
	}

	retval->dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	retval->dependencies[0].dstSubpass = 0;
	retval->dependencies[0].srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	retval->dependencies[0].dstStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	retval->dependencies[0].srcAccessMask = 0;
	retval->dependencies[0].dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	retval->dependencies[0].dependencyFlags = 0;
	retval->dependencies[1].srcSubpass = 0;
	retval->dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	retval->dependencies[1].srcStageMask = VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
	retval->dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	retval->dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	retval->dependencies[1].dstAccessMask = 0;
	retval->dependencies[1].dependencyFlags = 0;
	
	return retval;
}

void setRenderPassClearColor(RenderPass renderPass, uint32_t colorTarget, const float value[4])
{
	if (colorTarget < renderPass->numColor)
	{
		memcpy(renderPass->clearValue[colorTarget].color.float32, value, 4 * sizeof(float));
		renderPass->numClearValues = -1;
	}
}

void setRenderPassClearDepth(RenderPass renderPass, float value)
{
	if (renderPass->numDepth)
	{
		renderPass->clearValue[renderPass->numColor].depthStencil.depth = value;
		renderPass->numClearValues = -1;
	}
}

VkAttachmentDescription* getRenderPassColorTarget(RenderPass renderPass, uint32_t colorTarget)
{
	if (renderPass && renderPass->numColor > colorTarget)
	{
		return (renderPass->attachment + colorTarget);
	}
	return NULL;
}

VkAttachmentDescription* getRenderPassDepthStencilTarget(RenderPass renderPass)
{
	if (renderPass && renderPass->numDepth)
	{
		return (renderPass->attachment + renderPass->numColor);
	}
	return NULL;
}

const VkClearValue* getRenderPassClearValues(RenderPass renderPass, uint32_t* count)
{
	VkClearValue* compacted = renderPass->clearValue + renderPass->numColor + renderPass->numDepth;
	
	if (renderPass->numClearValues == -1)
	{
		renderPass->numClearValues = 0;
		for (uint32_t i = 0; i < renderPass->numColor; i++)
		{
			if (renderPass->attachment[i].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
			{
				compacted[renderPass->numClearValues] = renderPass->clearValue[i];
				++renderPass->numClearValues;
			}
		}
		for (uint32_t i = 0; i < renderPass->numDepth; i++)
		{
			if (renderPass->attachment[i + renderPass->numColor].loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR
				|| renderPass->attachment[i + renderPass->numColor].stencilLoadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
			{
				compacted[renderPass->numClearValues] = renderPass->clearValue[i + renderPass->numColor];
				++renderPass->numClearValues;
			}
		}
	}
	
	*count = (uint32_t)renderPass->numClearValues;
	return compacted;
}

VkRenderPass getRenderPassHandle(RenderPass renderPass)
{
	if (!renderPass)
	{
		return VK_NULL_HANDLE;
	}

	if (!renderPass->handle)
	{
		const VkAttachmentReference* dsRef = (renderPass->numDepth) ? &renderPass->reference[renderPass->numColor] : NULL;
		VkSubpassDescription subpass = {
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = renderPass->numColor,
			.pColorAttachments = renderPass->reference,
			.pDepthStencilAttachment = dsRef
		};
		VkRenderPassCreateInfo rpci = {
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = renderPass->numColor + renderPass->numDepth,
			.pAttachments = renderPass->attachment,
			.subpassCount = 1,
			.pSubpasses = &subpass,
			.dependencyCount = 2,
			.pDependencies = renderPass->dependencies
		};
		breakIfFailed(vkCreateRenderPass(Device, &rpci, Alloc, &renderPass->handle));
	}

	return renderPass->handle;
}

void destroyRenderPass(RenderPass renderPass)
{
	if (renderPass)
	{
		vkDestroyRenderPass(Device, renderPass->handle, Alloc);
		freeMem(renderPass->attachment);
		freeMem(renderPass->reference);
		freeMem(renderPass->clearValue);
		freeMem(renderPass);
	}
}
