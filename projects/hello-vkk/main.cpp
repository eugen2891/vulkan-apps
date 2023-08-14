#define HAVE_M_PI
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <vulkan-kit/vkk.h>
#include <shared/geometry.h>
#include <stb/stb_image.h>

#include "camera.hpp"

static const struct BoxData
{
	float vertices[24];
	uint16_t indices[36];
} kBoxData
{
	{
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f
	},
	{
		/* X+ */ 1, 0, 4, 0, 5, 4,
		/* X- */ 3, 2, 6, 2, 7, 6,
		/* Y+ */ 2, 3, 1, 3, 0, 1,
		/* Y- */ 6, 7, 5, 7, 4, 5,
		/* Z+ */ 6, 5, 3, 5, 0, 3,
		/* Z- */ 4, 7, 1, 7, 2, 1
	}
};

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	uint32_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
	SDL_Window* window = SDL_CreateWindow("hello-vkk",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		1024,
		1024,
		windowFlags);

	requestWindowSurface(window);
	requestDefaultCommandQueue(3, true);
	requestSwapchainColorTarget(VK_FORMAT_B8G8R8A8_SRGB);
	requestSwapchainDepthBuffer(VK_FORMAT_D32_SFLOAT);
	requestPresentMode(VK_PRESENT_MODE_FIFO_KHR);
	requestSwapchainClear(VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);
	requestSwapchainImageCount(3);
	createDevice();

	const float clearColor[]{ 0.0f, 0.5f, 0.5f, 1.f };
	RenderPass swapchainPass = getSwapchainRenderPass();
	setRenderPassClearColor(swapchainPass, 0, clearColor);
	setRenderPassClearDepth(swapchainPass, 1.f);

	Pipeline pipeline = createGraphicsPipeline("hello.glsl", VK_SHADER_STAGE_ALL, swapchainPass);
	setGraphicsPipelineDepthTest(pipeline, true, true, VK_COMPARE_OP_LESS);
	setGraphicsPipelineFaceCulling(pipeline, VK_CULL_MODE_BACK_BIT);

	int imageWidth, imageHeight, imageChannels;
	unsigned char* imageData = stbi_load("../assets/globe-8k.png", &imageWidth, &imageHeight, &imageChannels, 4);
	const size_t imageDataSize = imageHeight * imageWidth * 4;
	Buffer imageBuffer = createUploadBuffer(imageDataSize, eDeviceQueue_Invalid);
	memcpy(getBufferMappedPtr(imageBuffer), imageData, imageDataSize);
	stbi_image_free(imageData);

	SamplerState sampler = createSamplerState(VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);

	Camera camera(5.f, 1.f);
	Mesh sphere = getSphereMesh();
	Buffer cameraData = createUniformBuffer(sizeof(mat4), eDeviceQueue_Universal);
	Image textureImage = nullptr;
	Buffer vertexData = nullptr;

	SDL_MaximizeWindow(window);

	for (bool isRunning = true; isRunning;)
	{
		SDL_Event event;

		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				isRunning = false;
				break;
			case SDL_MOUSEWHEEL:
				camera.applyZoom(event.wheel.preciseY);
				break;
			case SDL_MOUSEMOTION:
				if (event.motion.state & SDL_BUTTON_LMASK)
				{
					camera.applyRotation(float(-event.motion.xrel), float(event.motion.yrel));
				}
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					camera.applyResize(event.window.data1, event.window.data2);
					resetSwapchain();
					break;
				}
				break;
			}
			continue;
		}

		if (!isRunning)
		{
			break;
		}

		camera.writeUniforms(cameraData);

		beginCommandBuffer(eDeviceQueue_Universal);
		
		if (!vertexData)
		{
			vertexData = createVertexArray(sphere->meshDataSize, eDeviceQueue_Invalid);
			bufferMemoryBarrier(vertexData, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
			pipelineBarrier(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			updateBuffer(vertexData, sphere->meshData, 0, sphere->meshDataSize);
			bufferMemoryBarrier(vertexData, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
			pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}

		if (!textureImage)
		{
			const ImageSubset subset = makeImageSubset(0, 1, 0, 1);
			const VkExtent3D imageExt{ uint32_t(imageWidth), uint32_t(imageHeight), 1 };
			textureImage = createSampledImage(VK_FORMAT_R8G8B8A8_UNORM, &imageExt, 12);
			imageMemoryBarrier(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_ACCESS_NONE, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, subset);
			pipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			updateImageMipLevel(imageBuffer, textureImage, 0);

			imageMemoryBarrier(textureImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_ACCESS_NONE, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, makeImageSubset(1, 11, 0, 1));
			for (uint32_t i = 1; i <= 11; i++)
			{
				imageMemoryBarrier(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, makeImageSubset(i - 1, 1, 0, 1));
				pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
				blit(textureImage, textureImage, makeImageSubset(i - 1, 1, 0, 1), makeImageSubset(i, 1, 0, 1));
			}

			imageMemoryBarrier(textureImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, makeImageSubset(0, 11, 0, 1));
			imageMemoryBarrier(textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_SHADER_READ_BIT, makeImageSubset(11, 1, 0, 1));
			pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		}

		beginRenderPass(swapchainPass, getSwapchainFramebuffer());
		bindSamplerState(0, sampler);
		bindUniformBuffer(0, cameraData);
		bindSampledImage(0, textureImage);
		bindVertexBufferRange(0, vertexData, 0);
		bindIndexBufferRange(VK_INDEX_TYPE_UINT32, vertexData, sphere->indexDataOffset);
		bindGraphicsPipeline(pipeline);
		drawIndexed(sphere->indexCount, 1, 0, 0, 0);
		endRenderPass();

		submitCommandBuffer(eDeviceQueue_Universal, true);
		presentImageToWindow();
	}

	deviceWaitIdle();
	destroyImage(textureImage);
	destroyBuffer(imageBuffer);
	destroyBuffer(vertexData);
	destroyBuffer(cameraData);
	destroyPipeline(pipeline);
	destroySamplerState(sampler);
	destroyDevice();

	SDL_DestroyWindow(window);

	return 0;
}