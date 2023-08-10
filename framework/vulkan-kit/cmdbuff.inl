#pragma once

void beginCommandBuffer(DeviceQueue queue)
{
	struct DeviceQueueContext* queueContext = &QueueContext[queue];
	const uint32_t index = queueContext->currentIndex;
	if (!queueContext->cmdBuffer)
	{
		VkFence fence = queueContext->cbFence[index];
		switch (vkGetFenceStatus(Device, fence))
		{
		case VK_NOT_READY:
			breakIfFailed(vkWaitForFences(Device, 1, &fence, VK_TRUE, UINT64_MAX));
			break;
		case VK_SUCCESS:
			break;
		default:
			breakIfNot(0);
			break;
		}
		breakIfFailed(vkResetFences(Device, 1, &fence));
		VkCommandBuffer handle = queueContext->cbHandle[index];
		breakIfFailed(vkResetCommandBuffer(handle, 0));
		breakIfFailed(vkResetDescriptorPool(Device, queueContext->cbDesc[index], 0));
		VkCommandBufferBeginInfo cbbi = { 
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};
		breakIfFailed(vkBeginCommandBuffer(handle, &cbbi));
		queueContext->cmdBuffer = handle;
	}
	CommandBuffer = queueContext->cbHandle[index];
	DescriptorPool = queueContext->cbDesc[index];
	ActiveQueue = queue;
}

void submitCommandBuffer(DeviceQueue queue, bool writeSwapchainImage)
{
	struct DeviceQueueContext* queueContext = &QueueContext[queue];
	const uint32_t index = queueContext->currentIndex;
	if (queueContext->cmdBuffer != VK_NULL_HANDLE)
	{
		VkSemaphore waits[1];
		VkPipelineStageFlags waitStage[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		uint32_t numWaits = 0;
		if (writeSwapchainImage)
		{
			waits[0] = SwapchainSemaphores[SwapchainCurrentIndex];
			++numWaits;
		}
		VkSemaphore semaphore = queueContext->cbSemaphore[index];
		breakIfFailed(vkEndCommandBuffer(queueContext->cbHandle[index]));
		
		VkSubmitInfo si = {
			.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
			.waitSemaphoreCount = numWaits,
			.pWaitSemaphores = waits,
			.pWaitDstStageMask = waitStage,
			.commandBufferCount = 1,
			.pCommandBuffers = &queueContext->cbHandle[index],
			.signalSemaphoreCount = 1,
			.pSignalSemaphores = &semaphore
		};
		
		breakIfFailed(vkQueueSubmit(queueContext->queueHandle, 1, &si, queueContext->cbFence[index]));

		queueContext->currentIndex = (index + 1) % queueContext->numCommandBuffers;
		queueContext->cmdBuffer = VK_NULL_HANDLE;
		queueContext->numDescriptorWrites = 0;
		queueContext->numBufferBarriers = 0;
		queueContext->numImageBarriers = 0;

		if (writeSwapchainImage)
		{
			breakIfNot(SwapchainUpdateSubmit == NULL);
			SwapchainUpdateSubmit = &queueContext->cbSemaphore[index];
		}

		if (queue == ActiveQueue)
		{
			CommandBuffer = VK_NULL_HANDLE;
			DescriptorPool = VK_NULL_HANDLE;
			ActiveQueue = eDeviceQueue_Invalid;
		}
	}
}

void bufferMemoryBarrier(Buffer buffer, VkAccessFlags from, VkAccessFlags to)
{
	struct DeviceQueueContext* queueContext = &QueueContext[ActiveQueue];
	breakIfNot(queueContext->numBufferBarriers < MAX_RESOURCE_BARRIERS);
	VkBufferMemoryBarrier* barrier = queueContext->bufferBarriers + (queueContext->numBufferBarriers++);
	barrier->sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
	barrier->pNext = NULL;
	barrier->srcAccessMask = from;
	barrier->dstAccessMask = to;
	barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier->buffer = getBufferHandle(buffer);
	barrier->offset = 0;
	barrier->size = buffer->size;
}

void imageMemoryBarrier(Image image, VkImageLayout fromLayout, VkAccessFlags fromAccess, VkImageLayout toLayout, VkAccessFlags toAccess, ImageSubset subset)
{
	struct DeviceQueueContext* queueContext = &QueueContext[ActiveQueue];
	breakIfNot(queueContext->numImageBarriers < MAX_RESOURCE_BARRIERS);
	VkImageMemoryBarrier* barrier = queueContext->imageBarriers + (queueContext->numImageBarriers++);
	barrier->sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier->pNext = NULL;
	barrier->srcAccessMask = fromAccess;
	barrier->dstAccessMask = toAccess;
	barrier->oldLayout = fromLayout;
	barrier->newLayout = toLayout;
	barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier->image = image->handle;
	barrier->subresourceRange.aspectMask = image->aspect;
	barrier->subresourceRange.baseMipLevel = imageSubsetFromMip(subset);
	barrier->subresourceRange.levelCount = imageSubsetNumMips(subset);
	barrier->subresourceRange.baseArrayLayer = imageSubsetFromLayer(subset);
	barrier->subresourceRange.layerCount = imageSubsetNumLayers(subset);
}

void pipelineBarrier(VkPipelineStageFlags from, VkPipelineStageFlags to)
{
	struct DeviceQueueContext* queueContext = &QueueContext[ActiveQueue];
	vkCmdPipelineBarrier(CommandBuffer, from, to, 0, 0, NULL, queueContext->numBufferBarriers, queueContext->bufferBarriers, queueContext->numImageBarriers, queueContext->imageBarriers);
	queueContext->numBufferBarriers = 0;
	queueContext->numImageBarriers = 0;
}

void updateBuffer(Buffer buffer, const void* data, size_t dstOffset, size_t bytes)
{
	const uint32_t index = (buffer->queue == eDeviceQueue_Invalid) ? 0 : QueueContext[buffer->queue].currentIndex;
	vkCmdUpdateBuffer(CommandBuffer, buffer->context[index].handle, dstOffset, bytes, data);
}

void beginRenderPass(RenderPass renderPass, Framebuffer framebuffer)
{
	uint32_t numClears = 0;
	const VkClearValue* clears = getRenderPassClearValues(renderPass, &numClears);
	VkRenderPassBeginInfo rpbi = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = getRenderPassHandle(renderPass),
		.framebuffer = framebuffer->handle,
		.renderArea = framebuffer->scissor,
		.clearValueCount = numClears,
		.pClearValues = clears
	};
	vkCmdBeginRenderPass(CommandBuffer, &rpbi, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(CommandBuffer, 0, 1, &framebuffer->viewport);
	vkCmdSetScissor(CommandBuffer, 0, 1, &framebuffer->scissor);
}

void bindUniformBuffer(uint32_t binding, Buffer buffer)
{
	breakIfNot(binding < MAX_UNIFORM_BUFFERS);
	struct DeviceQueueContext* queueContext = &QueueContext[ActiveQueue];
	VkDescriptorBufferInfo* info = &queueContext->uniformBuffers[binding];
	info->buffer = getBufferHandle(buffer);
	info->offset = 0;
	info->range = VK_WHOLE_SIZE;
	VkWriteDescriptorSet* descriptorWrite = queueContext->descriptorWrites + (queueContext->numDescriptorWrites++);
	descriptorWrite->sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite->pNext = NULL;
	descriptorWrite->dstSet = VK_NULL_HANDLE;
	descriptorWrite->dstBinding = binding + UB_BINDING_OFFSET;
	descriptorWrite->dstArrayElement = 0;
	descriptorWrite->descriptorCount = 1;
	descriptorWrite->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite->pImageInfo = NULL;
	descriptorWrite->pBufferInfo = info;
	descriptorWrite->pTexelBufferView = NULL;
}

void bindVertexBufferRange(uint32_t binding, Buffer buffer, size_t offset)
{
	VkBuffer bufferHandle = getBufferHandle(buffer);
	vkCmdBindVertexBuffers(CommandBuffer, binding, 1, &bufferHandle, &offset);
}

void bindIndexBufferRange(VkIndexType indexType, Buffer buffer, size_t offset)
{
	vkCmdBindIndexBuffer(CommandBuffer, getBufferHandle(buffer), offset, indexType);
}

void bindGraphicsPipeline(Pipeline pipeline)
{
	vkCmdBindPipeline(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, getPipelineHandle(pipeline));
}

static VkDescriptorSet applyPendingDescriptorUpdates()
{
	struct DeviceQueueContext* queueContext = &QueueContext[ActiveQueue];
	if (queueContext->numDescriptorWrites)
	{
		VkDescriptorSet retval = VK_NULL_HANDLE;
		VkDescriptorSetAllocateInfo dsai = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = DescriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &DescriptorSetLayout
		};
		breakIfFailed(vkAllocateDescriptorSets(Device, &dsai, &retval));
		for (uint32_t i = 0; i < queueContext->numDescriptorWrites; i++)
		{
			queueContext->descriptorWrites[i].dstSet = retval;
		}
		vkUpdateDescriptorSets(Device, queueContext->numDescriptorWrites, queueContext->descriptorWrites, 0, NULL);
		queueContext->numDescriptorWrites = 0;
		return retval;
	}
	return VK_NULL_HANDLE;
}

void drawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t firstIndex, uint32_t firstVertex, uint32_t firstInstance)
{
	VkDescriptorSet descriptorSet = applyPendingDescriptorUpdates();
	if (descriptorSet != VK_NULL_HANDLE)
	{
		vkCmdBindDescriptorSets(CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PipelineLayout, 0, 1, &descriptorSet, 0, NULL);
	}
	vkCmdDrawIndexed(CommandBuffer, numIndices, numInstances, firstIndex, firstVertex, firstInstance);
}

void endRenderPass(void)
{
	vkCmdEndRenderPass(CommandBuffer);
}
