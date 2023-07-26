#pragma once

struct PipelineT
{
	VkPipeline handle;
	VkPipelineBindPoint bindPoint;
};

struct GraphicsPipeline
{
	struct PipelineT base;
	RenderPass renderPass;
	VkPipelineShaderStageCreateInfo* shaderStages;
	VkPipelineColorBlendAttachmentState* blendAttachment;
	VkPipelineDepthStencilStateCreateInfo depthStencil;
	VkPipelineRasterizationStateCreateInfo rasterizer;
	VkVertexInputAttributeDescription* vertexAttrs;
	uint32_t numShaderStages;
	uint32_t numVertexAttrs;
	uint32_t vertexStride;
};

struct ComputePipeline
{
	struct PipelineT base;
	VkComputePipelineCreateInfo createInfo;
};

static uint32_t findShaderStage(VkShaderStageFlags stageFlag, VkPipelineShaderStageCreateInfo* stages, uint32_t count)
{
	for (uint32_t i = 0; i < count; i++)
	{
		if (stages[i].flags == stageFlag)
		{
			return (int)i;
		}
	}
	return count;
}

static void setShaderStage(struct GraphicsPipeline* gp, VkShaderStageFlags stageFlag, VkShaderModule module)
{
	uint32_t index = findShaderStage(stageFlag, gp->shaderStages, gp->numShaderStages);
	if (index == gp->numShaderStages)
	{
		gp->numShaderStages++;
	}
	VkPipelineShaderStageCreateInfo* ss = gp->shaderStages + index;
	ss->sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	ss->stage = stageFlag;
	ss->module = module;
	ss->pName = kShaderMain;
}

Pipeline createGraphicsPipeline(const char* shaderFile, VkShaderStageFlags stageFlags, RenderPass renderPass)
{
	breakIfNot(renderPass);
	struct GraphicsPipeline* retval = calloc(1, sizeof(struct GraphicsPipeline));
	VkPipelineColorBlendAttachmentState* blendAttachment = calloc(renderPass->numColor, sizeof(VkPipelineColorBlendAttachmentState));
	VkPipelineShaderStageCreateInfo* shaderStages = calloc(2, sizeof(VkPipelineShaderStageCreateInfo));
	if (!retval || !blendAttachment || !shaderStages)
	{
		breakIfNot(0);
		freeMem(shaderStages);
		freeMem(blendAttachment);
		freeMem(retval);
		return NULL;
	}

	retval->renderPass = renderPass;
	retval->shaderStages = shaderStages;
	retval->blendAttachment = blendAttachment;
	retval->depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	retval->rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	retval->rasterizer.lineWidth = 1.f;
	retval->base.bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	for (uint32_t i = 0; i < renderPass->numColor; i++)
	{
		blendAttachment[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	}

	if (stageFlags & VK_SHADER_STAGE_VERTEX_BIT)
	{
		VkShaderModule module = compileShader(VK_SHADER_STAGE_VERTEX_BIT, shaderFile, &retval->vertexAttrs, &retval->numVertexAttrs, &retval->vertexStride);
		if (module != VK_NULL_HANDLE)
		{
			setShaderStage(retval, VK_SHADER_STAGE_VERTEX_BIT, module);
		}
	}

	if (stageFlags & VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		VkShaderModule module = compileShader(VK_SHADER_STAGE_FRAGMENT_BIT, shaderFile, NULL, NULL, NULL);
		if (module != VK_NULL_HANDLE)
		{
			setShaderStage(retval, VK_SHADER_STAGE_FRAGMENT_BIT, module);
		}
	}

	return &retval->base;
}

void setGraphicsPipelineDepthTest(Pipeline pipeline, bool write, bool test, VkCompareOp compareOp)
{
	if (pipeline->bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		struct GraphicsPipeline* gp = (struct GraphicsPipeline*)pipeline;
		gp->depthStencil.depthWriteEnable = (write) ? VK_TRUE : VK_FALSE;
		gp->depthStencil.depthTestEnable = (test) ? VK_TRUE : VK_FALSE;
		gp->depthStencil.depthCompareOp = compareOp;
	}
}

void setGraphicsPipelineFaceCulling(Pipeline pipeline, VkCullModeFlags mode)
{
	if (pipeline->bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS)
	{
		struct GraphicsPipeline* gp = (struct GraphicsPipeline*)pipeline;
		gp->rasterizer.cullMode = mode;
	}
}

void destroyPipeline(Pipeline pipeline)
{
	if (pipeline)
	{
		switch (pipeline->bindPoint)
		{
		case VK_PIPELINE_BIND_POINT_GRAPHICS:
			{
				struct GraphicsPipeline* gp = (struct GraphicsPipeline*)pipeline;
				for (uint32_t i = 0; i < gp->numShaderStages; i++)
				{
					vkDestroyShaderModule(Device, gp->shaderStages[i].module, Alloc);
				}
				freeMem(gp->blendAttachment);
				freeMem(gp->vertexAttrs);
			}
			break;
		case VK_PIPELINE_BIND_POINT_COMPUTE:
			break;
		default:
			breakIfNot(0);
		}
		vkDestroyPipeline(Device, pipeline->handle, Alloc);
		freeMem(pipeline);
	}
}

static void buildGraphicsPipeline(struct GraphicsPipeline* gp)
{
	VkVertexInputBindingDescription inputBinding = {
		.binding = 0,
		.stride = gp->vertexStride,
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
	};
	VkPipelineVertexInputStateCreateInfo pvisci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &inputBinding,
		.vertexAttributeDescriptionCount = gp->numVertexAttrs,
		.pVertexAttributeDescriptions = gp->vertexAttrs
	};
	VkPipelineInputAssemblyStateCreateInfo piasci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology =  VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	};
	VkPipelineTessellationStateCreateInfo ptsci = {.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
	VkPipelineViewportStateCreateInfo pvsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};
	VkPipelineMultisampleStateCreateInfo pmsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
	};
	VkPipelineColorBlendStateCreateInfo pcbsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOp = VK_LOGIC_OP_NO_OP,
		.attachmentCount = gp->renderPass->numColor,
		.pAttachments = gp->blendAttachment
	};
	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	VkPipelineDynamicStateCreateInfo pdsci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = _countof(dynamicStates),
		.pDynamicStates = dynamicStates
	};
	VkGraphicsPipelineCreateInfo gpci = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = gp->numShaderStages,
		.pStages = gp->shaderStages,
		.pVertexInputState = &pvisci,
		.pInputAssemblyState = &piasci,
		.pTessellationState = &ptsci,
		.pViewportState = &pvsci,
		.pRasterizationState = &gp->rasterizer,
		.pMultisampleState = &pmsci,
		.pDepthStencilState = &gp->depthStencil,
		.pColorBlendState = &pcbsci,
		.pDynamicState = &pdsci,
		.layout = PipelineLayout,
		.renderPass =  getRenderPassHandle(gp->renderPass)
	};
	breakIfFailed(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &gpci, Alloc, &gp->base.handle));
}

static void buildComputePipeline(struct ComputePipeline* cp)
{
	breakIfFailed(vkCreateComputePipelines(Device, VK_NULL_HANDLE, 1, &cp->createInfo, Alloc, &cp->base.handle));
}

VkPipeline getPipelineHandle(Pipeline pipeline)
{
	if (pipeline)
	{
		if (pipeline->handle == VK_NULL_HANDLE)
		{
			switch (pipeline->bindPoint)
			{
			case VK_PIPELINE_BIND_POINT_GRAPHICS:
				buildGraphicsPipeline((struct GraphicsPipeline*)pipeline);
				break;
			case VK_PIPELINE_BIND_POINT_COMPUTE:
				buildComputePipeline((struct ComputePipeline*)pipeline);
				break;
			default:
				breakIfNot(0);
			}
		}
		return pipeline->handle;
	}
	return NULL;
}
