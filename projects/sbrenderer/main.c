#include "scene.h"

#include <SDL2/SDL.h>

#ifndef SDL_MAIN_HANDLED
#pragma comment(lib, "SDL2main")
#endif

/*
project:
	resources:
		textures
	pipelines:
		shaders et al
	passes:
		graphics pass:
			pipeline
			geometry
file:

project awesome-project

graphics-pipeline tonemap-demo
graphics-pipeline-vs shaders/ndc-with-uv.vert
graphics-pipeline-fs shaders/tonemapping.frag

texture hdr-texture C:\texture.hdr

graphics-pass tonemap-demo
graphics-pass-bind-texture hdr-texture 0
graphics-pass-draw triangle-ndc

*/

static SDL_bool processWindowEvents();

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	const int windowX = SDL_WINDOWPOS_CENTERED, windowY = SDL_WINDOWPOS_CENTERED;
	uint32_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
	SDL_Window* window = SDL_CreateWindow("Sandbox Renderer", windowX, windowY, 1920, 1080, windowFlags);

	requestWindowSurface(window);
	requestDefaultCommandQueue(3, true);
	requestPresentMode(VK_PRESENT_MODE_FIFO_KHR);
	requestSwapchainColorTarget(VK_FORMAT_B8G8R8A8_SRGB);
	requestSwapchainDepthBuffer(VK_FORMAT_D32_SFLOAT);
	requestSwapchainClear(VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);
	requestSwapchainImageCount(3);
	createDevice();

	Scene scene = (argc > 1) ? loadScene(argv[1]) : NULL;
	//for each file on the list perform upload

	SDL_ShowWindow(window);
	//SDL_MaximizeWindow(window);

	while (processWindowEvents())
	{
		beginCommandBuffer(eDeviceQueue_Universal);

		RenderPass finalPass = getSwapchainRenderPass();
		//screen-space pass tonemap.glsl
		//instead of clear, do tone mapping + write into output render target

		//applyToneMapping(hdrOutputImage, eDeviceQueue_Universal);
		// assume: render pass is on, there is a LDR output target 0
		// bind hdrOutputImage to texture slot 0, nearest neighbor sampler to slot 0
		// perform tone mapping (texelFetch)

		const float clearColor[] = { 0.0f, 0.3f, 0.5f, 1.f };
		setRenderPassClearColor(finalPass, 0, clearColor);
		setRenderPassClearDepth(finalPass, 1.f);

		beginRenderPass(finalPass, getSwapchainFramebuffer());
		endRenderPass();

		submitCommandBuffer(eDeviceQueue_Universal, true);

		presentImageToWindow();
	}

	deviceWaitIdle();
	freeScene(scene);
	destroyDevice();

	SDL_DestroyWindow(window);
	return 0;
}

SDL_bool processWindowEvents()
{
	SDL_Event event;
	
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_MOUSEWHEEL:
			//camera.applyZoom(event.wheel.preciseY);
			break;
		case SDL_MOUSEMOTION:
			if (event.motion.state & SDL_BUTTON_LMASK)
			{
				//camera.applyRotation(float(-event.motion.xrel), float(event.motion.yrel));
				//float h = sunPosition[1] + (float)event.motion.yrel * -0.001f;
				//sunPosition[1] = glm_max(glm_min(h, 1.f), -0.3f);
				//sunPosition[2] = -cosf(asinf(sunPosition[1]));
			}
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				//camera.applyResize(event.window.data1, event.window.data2);
				//aspectRatio = (float)event.window.data1 / (float)event.window.data2;
				resetSwapchain();
				break;
			}
			break;
		case SDL_QUIT:
			return SDL_FALSE;
		default:
			break;
		}
	}

	return SDL_TRUE;
}