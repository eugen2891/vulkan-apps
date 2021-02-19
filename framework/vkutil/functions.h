#pragma once

#include "framework.h"

namespace vkutil
{

    struct Functions
    {
        // 1.0 core global
        static PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
        static PFN_vkCreateInstance vkCreateInstance;

        // 1.1 core global
        static PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion;

        // 1.0 core instance
        static PFN_vkDestroyInstance vkDestroyInstance;
        static PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices;
        static PFN_vkGetPhysicalDeviceFeatures vkGetPhysicalDeviceFeatures;
        static PFN_vkGetPhysicalDeviceFormatProperties vkGetPhysicalDeviceFormatProperties;
        static PFN_vkGetPhysicalDeviceImageFormatProperties vkGetPhysicalDeviceImageFormatProperties;
        static PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties;
        static PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties;
        static PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties;
        static PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr;
        static PFN_vkCreateDevice vkCreateDevice;

        // 1.1 core instance
        static PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups;
        static PFN_vkGetPhysicalDeviceFeatures2 vkGetPhysicalDeviceFeatures2;
        static PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2;
        static PFN_vkGetPhysicalDeviceFormatProperties2 vkGetPhysicalDeviceFormatProperties2;
        static PFN_vkGetPhysicalDeviceImageFormatProperties2 vkGetPhysicalDeviceImageFormatProperties2;
        static PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2;
        static PFN_vkGetPhysicalDeviceMemoryProperties2 vkGetPhysicalDeviceMemoryProperties2;
        static PFN_vkGetPhysicalDeviceSparseImageFormatProperties2 vkGetPhysicalDeviceSparseImageFormatProperties2;
        static PFN_vkGetPhysicalDeviceExternalBufferProperties vkGetPhysicalDeviceExternalBufferProperties;
        static PFN_vkGetPhysicalDeviceExternalFenceProperties vkGetPhysicalDeviceExternalFenceProperties;
        static PFN_vkGetPhysicalDeviceExternalSemaphoreProperties vkGetPhysicalDeviceExternalSemaphoreProperties;

        // 1.0 khr_surface instance
        static PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR;
        static PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR;
        static PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR;
        static PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR;
        static PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR;

        // 1.0 core device
        static PFN_vkDestroyDevice vkDestroyDevice;
        static PFN_vkGetDeviceQueue vkGetDeviceQueue;
        static PFN_vkQueueSubmit vkQueueSubmit;
        static PFN_vkQueueWaitIdle vkQueueWaitIdle;
        static PFN_vkDeviceWaitIdle vkDeviceWaitIdle;
        static PFN_vkAllocateMemory vkAllocateMemory;
        static PFN_vkFreeMemory vkFreeMemory;
        static PFN_vkMapMemory vkMapMemory;
        static PFN_vkUnmapMemory vkUnmapMemory;
        static PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges;
        static PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges;
        static PFN_vkGetDeviceMemoryCommitment vkGetDeviceMemoryCommitment;
        static PFN_vkBindBufferMemory vkBindBufferMemory;
        static PFN_vkBindImageMemory vkBindImageMemory;
        static PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements;
        static PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements;
        static PFN_vkGetImageSparseMemoryRequirements vkGetImageSparseMemoryRequirements;
        static PFN_vkQueueBindSparse vkQueueBindSparse;
        static PFN_vkCreateFence vkCreateFence;
        static PFN_vkDestroyFence vkDestroyFence;
        static PFN_vkResetFences vkResetFences;
        static PFN_vkGetFenceStatus vkGetFenceStatus;
        static PFN_vkWaitForFences vkWaitForFences;
        static PFN_vkCreateSemaphore vkCreateSemaphore;
        static PFN_vkDestroySemaphore vkDestroySemaphore;
        static PFN_vkCreateEvent vkCreateEvent;
        static PFN_vkDestroyEvent vkDestroyEvent;
        static PFN_vkGetEventStatus vkGetEventStatus;
        static PFN_vkSetEvent vkSetEvent;
        static PFN_vkResetEvent vkResetEvent;
        static PFN_vkCreateQueryPool vkCreateQueryPool;
        static PFN_vkDestroyQueryPool vkDestroyQueryPool;
        static PFN_vkGetQueryPoolResults vkGetQueryPoolResults;
        static PFN_vkCreateBuffer vkCreateBuffer;
        static PFN_vkDestroyBuffer vkDestroyBuffer;
        static PFN_vkCreateBufferView vkCreateBufferView;
        static PFN_vkDestroyBufferView vkDestroyBufferView;
        static PFN_vkCreateImage vkCreateImage;
        static PFN_vkDestroyImage vkDestroyImage;
        static PFN_vkGetImageSubresourceLayout vkGetImageSubresourceLayout;
        static PFN_vkCreateImageView vkCreateImageView;
        static PFN_vkDestroyImageView vkDestroyImageView;
        static PFN_vkCreateShaderModule vkCreateShaderModule;
        static PFN_vkDestroyShaderModule vkDestroyShaderModule;
        static PFN_vkCreatePipelineCache vkCreatePipelineCache;
        static PFN_vkDestroyPipelineCache vkDestroyPipelineCache;
        static PFN_vkGetPipelineCacheData vkGetPipelineCacheData;
        static PFN_vkMergePipelineCaches vkMergePipelineCaches;
        static PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines;
        static PFN_vkCreateComputePipelines vkCreateComputePipelines;
        static PFN_vkDestroyPipeline vkDestroyPipeline;
        static PFN_vkCreatePipelineLayout vkCreatePipelineLayout;
        static PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout;
        static PFN_vkCreateSampler vkCreateSampler;
        static PFN_vkDestroySampler vkDestroySampler;
        static PFN_vkCreateDescriptorSetLayout vkCreateDescriptorSetLayout;
        static PFN_vkDestroyDescriptorSetLayout vkDestroyDescriptorSetLayout;
        static PFN_vkCreateDescriptorPool vkCreateDescriptorPool;
        static PFN_vkDestroyDescriptorPool vkDestroyDescriptorPool;
        static PFN_vkResetDescriptorPool vkResetDescriptorPool;
        static PFN_vkAllocateDescriptorSets vkAllocateDescriptorSets;
        static PFN_vkFreeDescriptorSets vkFreeDescriptorSets;
        static PFN_vkUpdateDescriptorSets vkUpdateDescriptorSets;
        static PFN_vkCreateFramebuffer vkCreateFramebuffer;
        static PFN_vkDestroyFramebuffer vkDestroyFramebuffer;
        static PFN_vkCreateRenderPass vkCreateRenderPass;
        static PFN_vkDestroyRenderPass vkDestroyRenderPass;
        static PFN_vkGetRenderAreaGranularity vkGetRenderAreaGranularity;
        static PFN_vkCreateCommandPool vkCreateCommandPool;
        static PFN_vkDestroyCommandPool vkDestroyCommandPool;
        static PFN_vkResetCommandPool vkResetCommandPool;
        static PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers;
        static PFN_vkFreeCommandBuffers vkFreeCommandBuffers;
        static PFN_vkBeginCommandBuffer vkBeginCommandBuffer;
        static PFN_vkEndCommandBuffer vkEndCommandBuffer;
        static PFN_vkResetCommandBuffer vkResetCommandBuffer;
        static PFN_vkCmdBindPipeline vkCmdBindPipeline;
        static PFN_vkCmdSetViewport vkCmdSetViewport;
        static PFN_vkCmdSetScissor vkCmdSetScissor;
        static PFN_vkCmdSetLineWidth vkCmdSetLineWidth;
        static PFN_vkCmdSetDepthBias vkCmdSetDepthBias;
        static PFN_vkCmdSetBlendConstants vkCmdSetBlendConstants;
        static PFN_vkCmdSetDepthBounds vkCmdSetDepthBounds;
        static PFN_vkCmdSetStencilCompareMask vkCmdSetStencilCompareMask;
        static PFN_vkCmdSetStencilWriteMask vkCmdSetStencilWriteMask;
        static PFN_vkCmdSetStencilReference vkCmdSetStencilReference;
        static PFN_vkCmdBindDescriptorSets vkCmdBindDescriptorSets;
        static PFN_vkCmdBindIndexBuffer vkCmdBindIndexBuffer;
        static PFN_vkCmdBindVertexBuffers vkCmdBindVertexBuffers;
        static PFN_vkCmdDraw vkCmdDraw;
        static PFN_vkCmdDrawIndexed vkCmdDrawIndexed;
        static PFN_vkCmdDrawIndirect vkCmdDrawIndirect;
        static PFN_vkCmdDrawIndexedIndirect vkCmdDrawIndexedIndirect;
        static PFN_vkCmdDispatch vkCmdDispatch;
        static PFN_vkCmdDispatchIndirect vkCmdDispatchIndirect;
        static PFN_vkCmdCopyBuffer vkCmdCopyBuffer;
        static PFN_vkCmdCopyImage vkCmdCopyImage;
        static PFN_vkCmdBlitImage vkCmdBlitImage;
        static PFN_vkCmdCopyBufferToImage vkCmdCopyBufferToImage;
        static PFN_vkCmdCopyImageToBuffer vkCmdCopyImageToBuffer;
        static PFN_vkCmdUpdateBuffer vkCmdUpdateBuffer;
        static PFN_vkCmdFillBuffer vkCmdFillBuffer;
        static PFN_vkCmdClearColorImage vkCmdClearColorImage;
        static PFN_vkCmdClearDepthStencilImage vkCmdClearDepthStencilImage;
        static PFN_vkCmdClearAttachments vkCmdClearAttachments;
        static PFN_vkCmdResolveImage vkCmdResolveImage;
        static PFN_vkCmdSetEvent vkCmdSetEvent;
        static PFN_vkCmdResetEvent vkCmdResetEvent;
        static PFN_vkCmdWaitEvents vkCmdWaitEvents;
        static PFN_vkCmdPipelineBarrier vkCmdPipelineBarrier;
        static PFN_vkCmdBeginQuery vkCmdBeginQuery;
        static PFN_vkCmdEndQuery vkCmdEndQuery;
        static PFN_vkCmdResetQueryPool vkCmdResetQueryPool;
        static PFN_vkCmdWriteTimestamp vkCmdWriteTimestamp;
        static PFN_vkCmdCopyQueryPoolResults vkCmdCopyQueryPoolResults;
        static PFN_vkCmdPushConstants vkCmdPushConstants;
        static PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass;
        static PFN_vkCmdNextSubpass vkCmdNextSubpass;
        static PFN_vkCmdEndRenderPass vkCmdEndRenderPass;
        static PFN_vkCmdExecuteCommands vkCmdExecuteCommands;

        // 1.1 core device
        static PFN_vkBindBufferMemory2 vkBindBufferMemory2;
        static PFN_vkBindImageMemory2 vkBindImageMemory2;
        static PFN_vkGetDeviceGroupPeerMemoryFeatures vkGetDeviceGroupPeerMemoryFeatures;
        static PFN_vkCmdSetDeviceMask vkCmdSetDeviceMask;
        static PFN_vkCmdDispatchBase vkCmdDispatchBase;
        static PFN_vkGetImageMemoryRequirements2 vkGetImageMemoryRequirements2;
        static PFN_vkGetBufferMemoryRequirements2 vkGetBufferMemoryRequirements2;
        static PFN_vkGetImageSparseMemoryRequirements2 vkGetImageSparseMemoryRequirements2;
        static PFN_vkTrimCommandPool vkTrimCommandPool;
        static PFN_vkGetDeviceQueue2 vkGetDeviceQueue2;
        static PFN_vkCreateSamplerYcbcrConversion vkCreateSamplerYcbcrConversion;
        static PFN_vkDestroySamplerYcbcrConversion vkDestroySamplerYcbcrConversion;
        static PFN_vkCreateDescriptorUpdateTemplate vkCreateDescriptorUpdateTemplate;
        static PFN_vkDestroyDescriptorUpdateTemplate vkDestroyDescriptorUpdateTemplate;
        static PFN_vkUpdateDescriptorSetWithTemplate vkUpdateDescriptorSetWithTemplate;
        static PFN_vkGetDescriptorSetLayoutSupport vkGetDescriptorSetLayoutSupport;

        // 1.2 core device
        static PFN_vkCmdDrawIndirectCount vkCmdDrawIndirectCount;
        static PFN_vkCmdDrawIndexedIndirectCount vkCmdDrawIndexedIndirectCount;
        static PFN_vkCreateRenderPass2 vkCreateRenderPass2;
        static PFN_vkCmdBeginRenderPass2 vkCmdBeginRenderPass2;
        static PFN_vkCmdNextSubpass2 vkCmdNextSubpass2;
        static PFN_vkCmdEndRenderPass2 vkCmdEndRenderPass2;
        static PFN_vkResetQueryPool vkResetQueryPool;
        static PFN_vkGetSemaphoreCounterValue vkGetSemaphoreCounterValue;
        static PFN_vkWaitSemaphores vkWaitSemaphores;
        static PFN_vkSignalSemaphore vkSignalSemaphore;
        static PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddress;
        static PFN_vkGetBufferOpaqueCaptureAddress vkGetBufferOpaqueCaptureAddress;
        static PFN_vkGetDeviceMemoryOpaqueCaptureAddress vkGetDeviceMemoryOpaqueCaptureAddress;

        // 1.0 khr_swapchain device
        static PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR;
        static PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR;
        static PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR;
        static PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR;
        static PFN_vkQueuePresentKHR vkQueuePresentKHR;

        // 1.1 khr_swapchain device
        static PFN_vkGetDeviceGroupPresentCapabilitiesKHR vkGetDeviceGroupPresentCapabilitiesKHR;
        static PFN_vkGetDeviceGroupSurfacePresentModesKHR vkGetDeviceGroupSurfacePresentModesKHR;
        static PFN_vkGetPhysicalDevicePresentRectanglesKHR vkGetPhysicalDevicePresentRectanglesKHR;
        static PFN_vkAcquireNextImage2KHR vkAcquireNextImage2KHR;

        static bool UpdateFunctionPointers(VkInstance instance, VkDevice device, uint32_t version = VK_API_VERSION_1_0);

    };

}