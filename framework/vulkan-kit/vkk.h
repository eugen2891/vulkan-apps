#pragma once

#include "macros.inl"

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <Volk/volk.h>

enum DeviceQueueT
{
	eDeviceQueue_Invalid = -1,
	eDeviceQueue_Universal,
	eDeviceQueue_Transfer,
	eDeviceQueue_Compute,
	eDeviceQueue_EnumMax
};

struct SDL_Window;

typedef struct ImageT* Image;
typedef struct BufferT* Buffer;
typedef struct PipelineT* Pipeline;
typedef struct RenderPassT* RenderPass;
typedef struct FramebufferT* Framebuffer;
typedef enum DeviceQueueT DeviceQueue;

void requestWindowSurface(struct SDL_Window* window);
void requestDefaultCommandQueue(uint32_t numCommandBuffers, bool present);
void requestSwapchainColorTarget(VkFormat format);
void requestSwapchainDepthBuffer(VkFormat format);
void requestPresentMode(VkPresentModeKHR presentMode);
void requestSwapchainImageCount(uint32_t numImages);
void requestSwapchainPreserve(VkImageAspectFlags flags);
void requestSwapchainClear(VkImageAspectFlags flags);

void createDevice(void);
void resetSwapchain(void);
void deviceWaitIdle(void);
void destroyDevice(void);

RenderPass createRenderPass(uint32_t numColor, uint32_t numDepth);
void setRenderPassClearColor(RenderPass renderPass, uint32_t colorTarget, const float value[4]);
void setRenderPassClearDepth(RenderPass renderPass, float value);
VkAttachmentDescription* getRenderPassColorTarget(RenderPass renderPass, uint32_t colorTarget);
VkAttachmentDescription* getRenderPassDepthStencilTarget(RenderPass renderPass);
void destroyRenderPass(RenderPass renderPass);
RenderPass getSwapchainRenderPass(void);

Framebuffer createFramebuffer(RenderPass renderPass, Image* images);
void destroyFramebuffer(Framebuffer framebuffer);
Framebuffer getSwapchainFramebuffer(void);

Buffer createVertexArray(size_t bytes, DeviceQueue queue);
Buffer createUniformBuffer(size_t bytes, DeviceQueue queue);
Buffer createUploadBuffer(size_t bytes, DeviceQueue queue);
void* getBufferMappedPtr(Buffer buffer);
void destroyBuffer(Buffer buffer);

Image createRenderTargetImage(VkFormat format, const VkExtent3D* size);
void destroyImage(Image image);

Pipeline createGraphicsPipeline(const char* shaderFile, VkShaderStageFlags stageFlags, RenderPass renderPass);
void setGraphicsPipelineDepthTest(Pipeline pipeline, bool write, bool test, VkCompareOp compareOp);
void setGraphicsPipelineFaceCulling(Pipeline pipeline, VkCullModeFlags mode);
void destroyPipeline(Pipeline pipeline);

void beginCommandBuffer(DeviceQueue queue);
void submitCommandBuffer(DeviceQueue queue, bool useSwapchainImage);
void bufferMemoryBarrier(Buffer buffer, VkAccessFlags from, VkAccessFlags to);
void imageMemoryBarrier(Image image, VkImageLayout fromLayout, VkAccessFlags fromAccess, VkImageLayout toLayout, VkAccessFlags toAccess, ImageSubset subset);
void pipelineBarrier(VkPipelineStageFlags from, VkPipelineStageFlags to);
void updateBuffer(Buffer buffer, const void* data, size_t dstOffset, size_t bytes);
//updateImageMipLevel(Buffer src, Image dst, uint32_t level);
void beginRenderPass(RenderPass renderPass, Framebuffer framebuffer);
void bindUniformBuffer(uint32_t binding, Buffer buffer);
void bindVertexBufferRange(uint32_t binding, Buffer buffer, size_t offset);
void bindIndexBufferRange(VkIndexType indexType, Buffer buffer, size_t offset);
void bindGraphicsPipeline(Pipeline pipeline);
void drawIndexed(uint32_t numIndices, uint32_t numInstances, uint32_t firstIndex, uint32_t firstVertex, uint32_t firstInstance);
void endRenderPass(void);

void presentImageToWindow(void);

#if WITH_RENDERDOC
void renderDocStartCapture(void);
void renderDocEndCapture(void);
#endif

#ifdef __cplusplus
}
#endif
