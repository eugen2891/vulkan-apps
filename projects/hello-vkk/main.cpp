#include "camera.hpp"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <vulkan-kit/vkk.h>

#include <glm/gtc/matrix_transform.hpp>

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

	Camera camera(5.f, 1.f);
	Buffer cameraData = createUniformBuffer(sizeof(glm::mat4), eDeviceQueue_Universal);
	Buffer vertexData = NULL;

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
			vertexData = createVertexArray(sizeof(kBoxData), eDeviceQueue_Invalid);
			bufferMemoryBarrier(vertexData, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
			pipelineBarrier(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			updateBuffer(vertexData, &kBoxData, 0, sizeof(kBoxData));
			bufferMemoryBarrier(vertexData, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
			pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}

		beginRenderPass(swapchainPass, getSwapchainFramebuffer());
		bindUniformBuffer(0, cameraData);
		bindVertexBufferRange(0, vertexData, 0);
		bindIndexBufferRange(VK_INDEX_TYPE_UINT16, vertexData, sizeof(kBoxData.vertices));
		bindGraphicsPipeline(pipeline);
		drawIndexed(36, 1, 0, 0, 0);
		endRenderPass();

		submitCommandBuffer(eDeviceQueue_Universal, true);
		presentImageToWindow();
	}

	deviceWaitIdle();
	destroyBuffer(vertexData);
	destroyBuffer(cameraData);
	destroyPipeline(pipeline);
	destroyDevice();

	SDL_DestroyWindow(window);

	return 0;
}