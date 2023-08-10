#pragma once

struct BufferContext
{
	VkBuffer handle;
	VkDeviceMemory memory;
	void* mapped;
};

struct BufferT
{
	size_t size;
	DeviceQueue queue;
	struct BufferContext* context;
};

static Buffer createBuffer(size_t size, VkBufferUsageFlags usage, DeviceQueue queue, bool forceCpuWritable)
{
	Buffer retval = calloc(1, sizeof(struct BufferT));
	breakIfNot(retval);
	if (!retval)
	{
		return NULL;
	}

	retval->size = size;
	retval->queue = queue;
	size_t numObjects = (queue == eDeviceQueue_Invalid) ? 1 : QueueContext[queue].numCommandBuffers;
	retval->context = calloc(numObjects, sizeof(struct BufferContext));
	breakIfNot(retval->context);
	if (!retval->context)
	{
		freeMem(retval);
		return NULL;
	}

	VkMemoryPropertyFlags memReqired = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkMemoryPropertyFlags memExcluded = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memMaybe = 0;
	usage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	if (queue != eDeviceQueue_Invalid || forceCpuWritable)
	{
		memReqired = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		if (usage != VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
		{
			memExcluded = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
		}
		else
		{
			memMaybe = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
			memExcluded = 0;
		}
	}

	for (size_t i = 0; i < numObjects; i++)
	{
		VkBufferCreateInfo bci = {
			.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			.size = size,
			.usage = usage
		};
		breakIfFailed(vkCreateBuffer(Device, &bci, Alloc, &retval->context[i].handle));
		VkMemoryRequirements memReq;
		vkGetBufferMemoryRequirements(Device, retval->context[i].handle, &memReq);
		VkMemoryAllocateInfo mai = {
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memReq.size,
			.memoryTypeIndex = findMemoryType(&memReq, memReqired, memExcluded, memMaybe)
		};
		breakIfFailed(vkAllocateMemory(Device, &mai, Alloc, &retval->context[i].memory));
		vkBindBufferMemory(Device, retval->context[i].handle, retval->context[i].memory, 0);
		if (queue != eDeviceQueue_Invalid || forceCpuWritable)
		{
			breakIfFailed(vkMapMemory(Device, retval->context[i].memory, 0, VK_WHOLE_SIZE, 0, &retval->context[i].mapped));
		}
	}

	return retval;
}

Buffer createVertexArray(size_t bytes, DeviceQueue queue)
{
	return createBuffer(bytes, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, queue, false);
}

Buffer createUniformBuffer(size_t bytes, DeviceQueue queue)
{
	return createBuffer(bytes, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, queue, false);
}

Buffer createUploadBuffer(size_t bytes, DeviceQueue queue)
{
	return createBuffer(bytes, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, queue, true);
}

void* getBufferMappedPtr(Buffer buffer)
{
	if (buffer)
	{
		const uint32_t index = (buffer->queue == eDeviceQueue_Invalid) ? 0 : QueueContext[buffer->queue].currentIndex;
		return buffer->context[index].mapped;
	}
	return NULL;
}

void destroyBuffer(Buffer buffer)
{
	uint32_t count = (buffer->queue == eDeviceQueue_Invalid) ? 1 : QueueContext[buffer->queue].numCommandBuffers;
	for (uint32_t i = 0; i < count; i++)
	{
		vkFreeMemory(Device, buffer->context[i].memory, Alloc);
		vkDestroyBuffer(Device, buffer->context[i].handle, Alloc);
	}
	freeMem(buffer->context);
	freeMem(buffer);
}

VkBuffer getBufferHandle(Buffer buffer)
{
	const uint32_t index = (buffer->queue == eDeviceQueue_Invalid) ? 0 : QueueContext[buffer->queue].currentIndex;
	return buffer->context[index].handle;
}
