#define HAVE_M_PI
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include <vulkan-kit/vkk.h>
#include <shared/geometry.h>

#include <string.h>

static struct SkyboxParams
{
	vec3 sunDirection;
	float sunIntencity;
	vec3 rayleighScatter;
	float rayleighScaleH;
	float viewElevation;
	float atmosphereMaxH;
	float planetRadius;
	float mieScatter;
	float mieScaleH;
	float miePrefD;
} Skybox = {
	.sunIntencity = 32.f,
	.rayleighScatter = {
		5.5e-6f,
		13.0e-6f,
		22.4e-6f
	},
	.rayleighScaleH = 8000.f,
	.viewElevation = 1000.f,
	.atmosphereMaxH = 100000.f,
	.planetRadius = 6371000.f,
	.mieScatter = 21e-6f,
	.mieScaleH = 1200.f,
	.miePrefD = 0.758f
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

	RenderPass swapchainPass = getSwapchainRenderPass();
	const float clearColor[] = { 0.0f, 0.5f, 0.5f, 1.f };
	setRenderPassClearColor(swapchainPass, 0, clearColor);
	setRenderPassClearDepth(swapchainPass, 1.f);

	Pipeline pipeline = createGraphicsPipeline("skybox.glsl", VK_SHADER_STAGE_ALL, swapchainPass);
	setGraphicsPipelineDepthTest(pipeline, true, true, VK_COMPARE_OP_LESS);
	setGraphicsPipelineFaceCulling(pipeline, VK_CULL_MODE_BACK_BIT);

	mat4 cameraMatrix;
	float aspectRatio = 1.f;
	Mesh sphere = getSphereMesh();
	vec3 sunPosition = { 0.f, 1.f, 0.f };
	Buffer vertexData = NULL, cameraData = NULL, skyboxData = NULL;

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
				//camera.applyZoom(event.wheel.preciseY);
				break;
			case SDL_MOUSEMOTION:
				if (event.motion.state & SDL_BUTTON_LMASK)
				{
					//camera.applyRotation(float(-event.motion.xrel), float(event.motion.yrel));
					float h = sunPosition[1] + (float)event.motion.yrel * -0.001f;
					sunPosition[1] = glm_max(glm_min(h, 1.f), -0.3f);
					sunPosition[2] = -cosf(asinf(sunPosition[1]));
				}
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event)
				{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
					//camera.applyResize(event.window.data1, event.window.data2);
					aspectRatio = (float)event.window.data1 / (float)event.window.data2;
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

		beginCommandBuffer(eDeviceQueue_Universal);

		if (!cameraData)
		{
			mat4 view, proj;
			glm_perspective(glm_rad(60.f), aspectRatio, 0.001f, 1000.f, proj);
			glm_lookat((vec3) { 0.f, 0.f, 5.f }, (vec3){0.f, 0.f, 0.f}, (vec3){ 0.f, 1.f, 0.f }, view);
			cameraData = createUniformBuffer(sizeof(cameraMatrix), eDeviceQueue_Universal);
			glm_mul(proj, view, cameraMatrix);
		}

		if (!skyboxData)
		{
			skyboxData = createUniformBuffer(sizeof(Skybox), eDeviceQueue_Universal);
		}

		glm_normalize_to(sunPosition, Skybox.sunDirection);
		memcpy(getBufferMappedPtr(cameraData), cameraMatrix, sizeof(cameraMatrix));
		memcpy(getBufferMappedPtr(skyboxData), &Skybox, sizeof(Skybox));

		if (!vertexData)
		{
			vertexData = createVertexArray(sphere->meshDataSize, eDeviceQueue_Invalid);
			bufferMemoryBarrier(vertexData, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
			pipelineBarrier(VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);
			updateBuffer(vertexData, sphere->meshData, 0, sphere->meshDataSize);
			bufferMemoryBarrier(vertexData, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
			pipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		}

		beginRenderPass(swapchainPass, getSwapchainFramebuffer());
		bindUniformBuffer(0, cameraData);
		bindUniformBuffer(1, skyboxData);
		bindVertexBufferRange(0, vertexData, 0);
		bindIndexBufferRange(VK_INDEX_TYPE_UINT32, vertexData, sphere->indexDataOffset);
		bindGraphicsPipeline(pipeline);
		drawIndexed(sphere->indexCount, 1, 0, 0, 0);
		endRenderPass();

		submitCommandBuffer(eDeviceQueue_Universal, true);
		presentImageToWindow();
	}

	deviceWaitIdle();
	destroyBuffer(vertexData);
	destroyBuffer(cameraData);
	destroyBuffer(skyboxData);
	destroyPipeline(pipeline);
	destroyDevice();

	SDL_DestroyWindow(window);

	return 0;
}
