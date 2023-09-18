#include <vkk.h>
#include <SDL2/SDL.h>
#include <cgltf/cgltf.h>

#define CGLTF_IMPLEMENTATION

static SDL_bool processWindowEvents();

int main(int argc, char** argv)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	uint32_t windowFlags = SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN;
	SDL_Window* window = SDL_CreateWindow("Sandbox Renderer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 800, windowFlags);

	requestWindowSurface(window);
	requestDefaultCommandQueue(3, true);
	requestPresentMode(VK_PRESENT_MODE_FIFO_KHR);
	requestSwapchainColorTarget(VK_FORMAT_B8G8R8A8_SRGB);
	requestSwapchainDepthBuffer(VK_FORMAT_D32_SFLOAT);
	requestSwapchainClear(VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_DEPTH_BIT);
	requestSwapchainImageCount(3);
	createDevice();

	SDL_ShowWindow(window);
	//SDL_MaximizeWindow(window);

	while (processWindowEvents())
	{
		beginCommandBuffer(eDeviceQueue_Universal);

		RenderPass finalPass = getSwapchainRenderPass();
		const float clearColor[] = { 0.0f, 0.3f, 0.5f, 1.f };
		setRenderPassClearColor(finalPass, 0, clearColor);
		setRenderPassClearDepth(finalPass, 1.f);

		beginRenderPass(finalPass, getSwapchainFramebuffer());
		endRenderPass();

		submitCommandBuffer(eDeviceQueue_Universal, true);

		presentImageToWindow();
	}

	deviceWaitIdle();
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