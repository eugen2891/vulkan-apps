#include "vkk.h"

#include <stdio.h>
#include <Volk/volk.c>
#include <SDL2/SDL_syswm.h>

#pragma comment(lib, "SDL2")

#if _DEBUG
#pragma comment(lib, "spirv-cross-cd")
#pragma comment(lib, "spirv-cross-cppd")
#pragma comment(lib, "spirv-cross-msld")
#pragma comment(lib, "spirv-cross-cored")
#pragma comment(lib, "spirv-cross-glsld")
#pragma comment(lib, "spirv-cross-hlsld")
#pragma comment(lib, "spirv-cross-reflectd")
#pragma comment(lib, "shaderc_combinedd")
#else
#pragma comment(lib, "spirv-cross-c")
#pragma comment(lib, "spirv-cross-cpp")
#pragma comment(lib, "spirv-cross-msl")
#pragma comment(lib, "spirv-cross-core")
#pragma comment(lib, "spirv-cross-glsl")
#pragma comment(lib, "spirv-cross-hlsl")
#pragma comment(lib, "spirv-cross-reflect")
#pragma comment(lib, "shaderc_combined")
#endif

static VkAllocationCallbacks* Alloc = NULL;
static VkInstance Instance = VK_NULL_HANDLE;
static VkDevice Device = VK_NULL_HANDLE;
static VkSurfaceKHR Surface = VK_NULL_HANDLE;
static VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
static VkDescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
static VkPipelineLayout PipelineLayout = VK_NULL_HANDLE;

static VkCommandBuffer CommandBuffer = VK_NULL_HANDLE;
static VkDescriptorPool DescriptorPool = VK_NULL_HANDLE;
static DeviceQueue ActiveQueue = eDeviceQueue_Invalid;

static struct SDL_Window* Window = NULL;
static DeviceQueue PresentQueue = eDeviceQueue_Invalid;
static VkPresentModeKHR PresentMode = VK_PRESENT_MODE_FIFO_KHR;

static Image SwapchainImages = NULL;
static Image SwapchainDepthImage = NULL;
static VkSemaphore* SwapchainSemaphores = NULL;
static Framebuffer* SwapchainFramebuffers = NULL;
static VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
static VkFormat SwapchainColorTarget = VK_FORMAT_UNDEFINED;
static VkFormat SwapchainDepthBuffer = VK_FORMAT_UNDEFINED;
static VkImageAspectFlags SwapchainPreserve = 0;
static VkImageAspectFlags SwapchainClear = 0;
static RenderPass SwapchainRenderPass = NULL;
static VkSemaphore SwapchainNextSemaphore = VK_NULL_HANDLE;
static uint32_t SwapchainCurrentIndex = 0;
static VkSemaphore* SwapchainUpdateSubmit;
static Image SwapchainCurrentImage = NULL;
static uint32_t SwapchainLength = 0;

static const char* kShaderMain = "main";

static const char* InstanceExt[MAX_INSTANCE_EXTENSIONS];
static uint32_t NumInstanceExt = 0;

static const char* DeviceExt[MAX_INSTANCE_EXTENSIONS];
static uint32_t NumDeviceExt = 0;

struct ShaderMacro
{
	size_t nameLength;
	size_t valLength;
	char name[32];
	char val[16];
};

static struct ShaderMacro ShaderMacros[MAX_SHADER_BINDINGS];
static uint32_t NumShaderMacros = 0;

struct DeviceQueueContext
{
	VkImageMemoryBarrier imageBarriers[MAX_RESOURCE_BARRIERS];
	VkBufferMemoryBarrier bufferBarriers[MAX_RESOURCE_BARRIERS];
#if MAX_SAMPLER_STATES
	VkDescriptorImageInfo samplerStates[MAX_SAMPLER_STATES];
#endif
#if MAX_UNIFORM_BUFFERS
	VkDescriptorBufferInfo uniformBuffers[MAX_UNIFORM_BUFFERS];
#endif
#if MAX_SAMPLED_IMAGES
	VkDescriptorImageInfo sampledImages[MAX_SAMPLED_IMAGES];
#endif
#if MAX_SHADER_BINDINGS
	VkWriteDescriptorSet descriptorWrites[MAX_SHADER_BINDINGS];
#else
	VkWriteDescriptorSet* descriptorWrites;
#endif
	VkQueueFlags requiredFlags;
	VkQueueFlags excludedFlags;
	VkCommandPool cmdPool;
	VkCommandBuffer cmdBuffer;
	VkCommandBuffer* cbHandle;
	VkSemaphore* cbSemaphore;
	VkDescriptorPool* cbDesc;
	VkFence* cbFence;
	VkSemaphore lastSubmit;
	VkQueue queueHandle;
	uint32_t numBufferBarriers;
	uint32_t numImageBarriers;
	uint32_t numDescriptorWrites;
	uint32_t numCommandBuffers;
	uint32_t currentIndex;
	uint32_t queueFamily;
};

static struct DeviceQueueContext QueueContext[eDeviceQueue_EnumMax] = {
	{.requiredFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT },
	{.requiredFlags = VK_QUEUE_TRANSFER_BIT,.excludedFlags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT },
	{.requiredFlags = VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,.excludedFlags = VK_QUEUE_GRAPHICS_BIT }
};

static void initImage(Image, VkFormat, const VkExtent3D*, uint32_t, bool, bool);
static uint32_t findMemoryType(const VkMemoryRequirements*, VkMemoryPropertyFlags, VkMemoryPropertyFlags, VkMemoryPropertyFlags);
static VkShaderModule compileShader(VkShaderStageFlags, const char*, VkVertexInputAttributeDescription**, uint32_t*, uint32_t*);
static const VkClearValue* getRenderPassClearValues(RenderPass, uint32_t*);
static VkRenderPass getRenderPassHandle(RenderPass);
static VkPipeline getPipelineHandle(Pipeline);
static VkBuffer getBufferHandle(Buffer);
static void destroySwapchain(bool);

#include "buffer.inl"
#include "image.inl"
#include "sampler.inl"
#include "renderdoc.inl"
#include "renderpass.inl"
#include "shaders.inl"
#include "swapchain.inl"
#include "framebuff.inl"
#include "pipeline.inl"
#include "cmdbuff.inl"

uint32_t findMemoryType(const VkMemoryRequirements* reqs, VkMemoryPropertyFlags flags, VkMemoryPropertyFlags exclude, VkMemoryPropertyFlags maybe)
{
	VkPhysicalDeviceMemoryProperties props;
	vkGetPhysicalDeviceMemoryProperties(PhysicalDevice, &props);
	uint32_t retval = props.memoryTypeCount, fallback = retval;
	for (uint32_t i = 0; i < props.memoryTypeCount; i++)
	{
		const VkMemoryPropertyFlags typeFlags = props.memoryTypes[i].propertyFlags;
		if ((reqs->memoryTypeBits & (1 << i)) && (typeFlags & flags) && !(typeFlags & exclude))
		{
			if (typeFlags & maybe)
			{
				retval = fallback = i;
				break;
			}
			else if (fallback == props.memoryTypeCount)
			{
				fallback = i;
			}
		}
	}
	if (retval == props.memoryTypeCount)
	{
		retval = fallback;
	}
	breakIfNot(retval < props.memoryTypeCount);
	return retval;
}

void requestWindowSurface(struct SDL_Window* window)
{
	Window = window;
	InstanceExt[NumInstanceExt++] = VK_KHR_SURFACE_EXTENSION_NAME;
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	InstanceExt[NumInstanceExt++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#else
#error This platform is not supported yet
#endif
	DeviceExt[NumDeviceExt++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
}

static void requestDeviceQueue(DeviceQueue queue, uint32_t numCommandBuffers, bool present)
{
	QueueContext[queue].cbDesc = calloc(numCommandBuffers, sizeof(VkDescriptorPool));
	QueueContext[queue].cbHandle = calloc(numCommandBuffers, sizeof(VkCommandBuffer));
	QueueContext[queue].cbSemaphore = calloc(numCommandBuffers, sizeof(VkSemaphore));
	QueueContext[queue].cbFence = calloc(numCommandBuffers, sizeof(VkFence));
	QueueContext[queue].numCommandBuffers = numCommandBuffers;
	if (present)
	{
		PresentQueue = queue;
	}
}

void requestDefaultCommandQueue(uint32_t numCommandBuffers, bool present)
{
	requestDeviceQueue(eDeviceQueue_Universal, numCommandBuffers, present);
}

void requestSwapchainColorTarget(VkFormat format)
{
	SwapchainColorTarget = format;
}

void requestSwapchainDepthBuffer(VkFormat format)
{
	SwapchainDepthBuffer = format;
}

void requestPresentMode(VkPresentModeKHR presentMode)
{
	PresentMode = presentMode;
}

void requestSwapchainImageCount(uint32_t numImages)
{
	SwapchainLength = numImages;
}

void requestSwapchainPreserve(VkImageAspectFlags flags)
{
	SwapchainPreserve = flags;
}

void requestSwapchainClear(VkImageAspectFlags flags)
{
	SwapchainClear = flags;
}

static int findQueueFamily(VkPhysicalDevice phd, const VkQueueFamilyProperties* qfs, uint32_t numQfs, const struct DeviceQueueContext* ctx, bool present)
{
	int retval = -1;
	for (int i = 0; i < (int)numQfs; i++)
	{
		if (qfs[i].queueCount > 0 
			&& (qfs[i].queueFlags & ctx->requiredFlags) == ctx->requiredFlags
			&& (qfs[i].queueFlags & ctx->excludedFlags) == 0)
		{
			if (present)
			{
				VkBool32 canPresent = VK_FALSE;
				breakIfFailed(vkGetPhysicalDeviceSurfaceSupportKHR(phd, (uint32_t)i, Surface, &canPresent));
				if (canPresent)
				{
					retval = i;
					break;
				}
			}
			else
			{
				retval = i;
				break;
			}
		}
	}
	return retval;
}

void createDevice(void)
{
	breakIfFailed(volkInitialize());

	const VkApplicationInfo app = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.apiVersion = VK_API_VERSION_1_1
	};
	const VkInstanceCreateInfo ici = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &app,
		.enabledExtensionCount = NumInstanceExt,
		.ppEnabledExtensionNames = InstanceExt
	};
	breakIfFailed(vkCreateInstance(&ici, Alloc, &Instance));
	volkLoadInstanceOnly(Instance);

	if (Window)
	{
		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(Window, &wmInfo);
#ifdef VK_USE_PLATFORM_WIN32_KHR
		const VkWin32SurfaceCreateInfoKHR sci = {
			.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = wmInfo.info.win.hinstance,
			.hwnd = wmInfo.info.win.window
		};
		breakIfFailed(vkCreateWin32SurfaceKHR(Instance, &sci, Alloc, &Surface));
#else
#error Unsupported platform
#endif
	}

	uint32_t numPhysicalDevices = 0;
	breakIfFailed(vkEnumeratePhysicalDevices(Instance, &numPhysicalDevices, NULL));
	VkPhysicalDevice* physicalDevices = malloc(numPhysicalDevices * sizeof(VkPhysicalDevice));
	breakIfFailed(vkEnumeratePhysicalDevices(Instance, &numPhysicalDevices, physicalDevices));

	int discreet = -1, integrated = -1;
	uint32_t qfDiscreet[eDeviceQueue_EnumMax], qfIntegrated[eDeviceQueue_EnumMax];
	for (uint32_t i = 0; i < numPhysicalDevices; i++)
	{
		int* storedIndex = NULL;
		uint32_t* storedQueues = NULL;
		VkPhysicalDeviceProperties properties;
		VkPhysicalDevice physicalDevice = *(physicalDevices + i);
		vkGetPhysicalDeviceProperties(physicalDevice, &properties);
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU && integrated == -1)
		{
			storedQueues = qfIntegrated;
			storedIndex = &integrated;
		}
		else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && discreet == -1)
		{
			storedQueues = qfDiscreet;
			storedIndex = &discreet;
		}
		else
		{
			continue;
		}
		*storedIndex = (int)i;

		uint32_t numQueueFamilies = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, NULL);
		if (numQueueFamilies == 0)
		{
			continue;
		}

		VkQueueFamilyProperties* queueFamilies = malloc(numQueueFamilies * sizeof(VkQueueFamilyProperties));
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, queueFamilies);
		
		for (int i = 0; i < eDeviceQueue_EnumMax; i++)
		{
			const struct DeviceQueueContext* queueContext = &QueueContext[i];
			if (queueContext->numCommandBuffers > 0)
			{
				const bool present = (PresentQueue == i);
				int familyIndex = findQueueFamily(physicalDevice, queueFamilies, numQueueFamilies, queueContext, present);
				if (familyIndex > -1)
				{
					storedQueues[i] = (uint32_t)familyIndex;
				}
				else
				{
					*storedIndex = -1;
					break;
				}
			}
		}

		freeMem(queueFamilies);
	}

	static const float prio = 1.f;
	uint32_t numDqci = 0, *queueFamilyIndices = NULL;
	VkDeviceQueueCreateInfo dqci[eDeviceQueue_EnumMax] = { 0 };
	if (discreet == -1)
	{
		breakIfNot(integrated != -1);
		PhysicalDevice = physicalDevices[integrated];
		queueFamilyIndices = qfIntegrated;
	}
	else
	{
		PhysicalDevice = physicalDevices[discreet];
		queueFamilyIndices = qfDiscreet;
	}

	freeMem(physicalDevices);

	for (uint32_t i = 0, j = 0; i < eDeviceQueue_EnumMax; i++)
	{
		struct DeviceQueueContext* queueContext = &QueueContext[i];
		if (queueContext->numCommandBuffers > 0)
		{
			VkDeviceQueueCreateInfo* info = &dqci[numDqci++];
			info->sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			info->queueFamilyIndex = queueFamilyIndices[j++];
			info->queueCount = 1;
			info->pQueuePriorities = &prio;
			queueContext->queueFamily = info->queueFamilyIndex;
		}
	}

	VkDeviceCreateInfo dci = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = numDqci,
		.pQueueCreateInfos = dqci,
		.enabledExtensionCount = NumDeviceExt,
		.ppEnabledExtensionNames = DeviceExt
	};
	breakIfFailed(vkCreateDevice(PhysicalDevice, &dci, Alloc, &Device));
	volkLoadDevice(Device);

	SwapchainRenderPass = createRenderPass(1, (SwapchainDepthBuffer != VK_FORMAT_UNDEFINED) ? 1 : 0);

	getRenderPassColorTarget(SwapchainRenderPass, 0)->format = SwapchainColorTarget;
	getRenderPassColorTarget(SwapchainRenderPass, 0)->finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	if (SwapchainPreserve & VK_IMAGE_ASPECT_COLOR_BIT)
	{
		getRenderPassColorTarget(SwapchainRenderPass, 0)->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	}
	else if (SwapchainClear & VK_IMAGE_ASPECT_COLOR_BIT)
	{
		getRenderPassColorTarget(SwapchainRenderPass, 0)->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	}
	
	if (SwapchainDepthBuffer != VK_FORMAT_UNDEFINED)
	{
		getRenderPassDepthStencilTarget(SwapchainRenderPass)->format = SwapchainDepthBuffer;
		if (SwapchainPreserve & VK_IMAGE_ASPECT_DEPTH_BIT)
		{
			getRenderPassDepthStencilTarget(SwapchainRenderPass)->loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		}
		else if (SwapchainClear & VK_IMAGE_ASPECT_DEPTH_BIT)
		{
			getRenderPassDepthStencilTarget(SwapchainRenderPass)->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
		if (SwapchainPreserve & VK_IMAGE_ASPECT_STENCIL_BIT)
		{
			getRenderPassDepthStencilTarget(SwapchainRenderPass)->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		}
		else if (SwapchainClear & VK_IMAGE_ASPECT_STENCIL_BIT)
		{
			getRenderPassDepthStencilTarget(SwapchainRenderPass)->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		}
	}

	resetSwapchain();

	uint32_t bindIndex = 0;
	VkDescriptorSetLayoutBinding bindings[MAX_SHADER_BINDINGS] = { 0 };
#if MAX_SAMPLER_STATES
	for (uint32_t i = 0; i < MAX_SAMPLER_STATES; i++)
	{
		struct ShaderMacro* macro = &ShaderMacros[NumShaderMacros++];
		macro->nameLength = snprintf(macro->name, sizeof(macro->name), "sampler_state_%u", i);
		macro->valLength = snprintf(macro->val, sizeof(macro->val), "%u", bindIndex);
		VkDescriptorSetLayoutBinding* info = &bindings[bindIndex];
		info->binding = bindIndex;
		info->descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
		info->descriptorCount = 1;
		info->stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		++bindIndex;
	}
#endif
#if MAX_UNIFORM_BUFFERS
	for (uint32_t i = 0; i < MAX_UNIFORM_BUFFERS; i++)
	{
		struct ShaderMacro* macro = &ShaderMacros[NumShaderMacros++];
		macro->nameLength = snprintf(macro->name, sizeof(macro->name), "uniform_buffer_%u", i);
		macro->valLength = snprintf(macro->val, sizeof(macro->val), "%u", bindIndex);
		VkDescriptorSetLayoutBinding* info = &bindings[bindIndex];
		info->binding = bindIndex;
		info->descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		info->descriptorCount = 1;
		info->stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		++bindIndex;
	}
#endif
#if MAX_SAMPLED_IMAGES
	for (uint32_t i = 0; i < MAX_SAMPLED_IMAGES; i++)
	{
		struct ShaderMacro* macro = &ShaderMacros[NumShaderMacros++];
		macro->nameLength = snprintf(macro->name, sizeof(macro->name), "sampled_image_%u", i);
		macro->valLength = snprintf(macro->val, sizeof(macro->val), "%u", bindIndex);
		VkDescriptorSetLayoutBinding* info = &bindings[bindIndex];
		info->binding = bindIndex;
		info->descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		info->descriptorCount = 1;
		info->stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		++bindIndex;
	}
#endif
	VkDescriptorSetLayoutCreateInfo dslci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = bindIndex,
		.pBindings = bindings
	};
	breakIfFailed(vkCreateDescriptorSetLayout(Device, &dslci, Alloc, &DescriptorSetLayout));
	const uint32_t numPushConstRanges = (MAX_PUSH_CONST_BYTES) ? 1 : 0;
	const VkPushConstantRange pushConst = {
		.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		.size = MAX_PUSH_CONST_BYTES
	};
	VkPipelineLayoutCreateInfo plci = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = 1,
		.pSetLayouts = &DescriptorSetLayout,
		.pushConstantRangeCount = numPushConstRanges,
		.pPushConstantRanges = &pushConst
	};
	breakIfFailed(vkCreatePipelineLayout(Device, &plci, Alloc, &PipelineLayout));

	uint32_t numPoolSizes = 0;
	VkDescriptorPoolSize poolSizes[16];
#if MAX_SAMPLER_STATES
	{
		VkDescriptorPoolSize* ps = &poolSizes[numPoolSizes++];
		ps->type = VK_DESCRIPTOR_TYPE_SAMPLER;
		ps->descriptorCount = MAX_SAMPLER_STATES * MAX_DRAW_CALLS;
	}
#endif
#if MAX_UNIFORM_BUFFERS
	{
		VkDescriptorPoolSize* ps = &poolSizes[numPoolSizes++];
		ps->type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ps->descriptorCount = MAX_UNIFORM_BUFFERS * MAX_DRAW_CALLS;
	}
#endif
#if MAX_SAMPLED_IMAGES
	{
		VkDescriptorPoolSize* ps = &poolSizes[numPoolSizes++];
		ps->type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		ps->descriptorCount = MAX_SAMPLED_IMAGES * MAX_DRAW_CALLS;
	}
#endif
	VkDescriptorPoolCreateInfo dpci = {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = MAX_DRAW_CALLS,
		.poolSizeCount = numPoolSizes,
		.pPoolSizes = poolSizes
	};

	for (int i = 0; i < eDeviceQueue_EnumMax; i++)
	{
		struct DeviceQueueContext* queueContext = &QueueContext[i];
		if (queueContext->numCommandBuffers > 0)
		{
			VkCommandPoolCreateInfo cpci = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
				.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
				.queueFamilyIndex = queueContext->queueFamily
			};
			breakIfFailed(vkCreateCommandPool(Device, &cpci, Alloc, &queueContext->cmdPool));
			queueContext->cbDesc = calloc(queueContext->numCommandBuffers, sizeof(VkDescriptorPool));
			queueContext->cbHandle = calloc(queueContext->numCommandBuffers, sizeof(VkCommandBuffer));
			queueContext->cbSemaphore = calloc(queueContext->numCommandBuffers, sizeof(VkSemaphore));
			queueContext->cbFence = calloc(queueContext->numCommandBuffers, sizeof(VkFence));
			VkCommandBufferAllocateInfo cbai = {
				.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				.commandPool = queueContext->cmdPool,
				.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				.commandBufferCount = queueContext->numCommandBuffers
			};
			breakIfFailed(vkAllocateCommandBuffers(Device, &cbai, queueContext->cbHandle));
			vkGetDeviceQueue(Device, queueContext->queueFamily, 0, &queueContext->queueHandle);
			for (uint32_t j = 0; j < queueContext->numCommandBuffers; j++)
			{
				VkFenceCreateInfo fci = {
					.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
					.flags = VK_FENCE_CREATE_SIGNALED_BIT 
				};
				VkSemaphoreCreateInfo sci = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
				breakIfFailed(vkCreateSemaphore(Device, &sci, Alloc, &queueContext->cbSemaphore[j]));
				breakIfFailed(vkCreateFence(Device, &fci, Alloc, &queueContext->cbFence[j]));
				if ((queueContext->requiredFlags & VK_QUEUE_GRAPHICS_BIT) || (queueContext->requiredFlags & VK_QUEUE_COMPUTE_BIT))
				{
					breakIfFailed(vkCreateDescriptorPool(Device, &dpci, Alloc, &queueContext->cbDesc[j]));
				}
			}
		}
	}
}

void deviceWaitIdle(void)
{
	breakIfFailed(vkDeviceWaitIdle(Device));
}

void destroyDevice(void)
{
	destroySwapchain(false);
	destroyRenderPass(SwapchainRenderPass);
	vkDestroyPipelineLayout(Device, PipelineLayout, Alloc);
	vkDestroyDescriptorSetLayout(Device, DescriptorSetLayout, Alloc);
	for (int i = 0; i < eDeviceQueue_EnumMax; i++)
	{
		struct DeviceQueueContext* queueContext = &QueueContext[i];
		if (queueContext->numCommandBuffers > 0)
		{
			vkDestroyCommandPool(Device, queueContext->cmdPool, Alloc);
			for (uint32_t j = 0; j < queueContext->numCommandBuffers; j++)
			{
				if (queueContext->cbDesc)
				{
					vkDestroyDescriptorPool(Device, queueContext->cbDesc[j], Alloc);
				}
				if (queueContext->cbSemaphore)
				{
					vkDestroySemaphore(Device, queueContext->cbSemaphore[j], Alloc);
				}
				if (queueContext->cbFence)
				{
					vkDestroyFence(Device, queueContext->cbFence[j], Alloc);
				}
			}
			freeMem(queueContext->cbDesc);
			freeMem(queueContext->cbHandle);
			freeMem(queueContext->cbSemaphore);
			freeMem(queueContext->cbFence);
		}
	}
	vkDestroyDevice(Device, Alloc);
	if (Surface)
	{
		vkDestroySurfaceKHR(Instance, Surface, Alloc);
	}
	vkDestroyInstance(Instance, Alloc);
}
