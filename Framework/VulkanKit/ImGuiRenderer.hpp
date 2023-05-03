#pragma once

#include "VulkanApi.hpp"
#include "EventHandler.hpp"
#include "BindingTable.hpp"

#include "../Utilities/Array.hpp"

struct SDL_Window;

namespace vulkan
{

class ImGuiRenderer : public APIClient, public EventHandler
{
public:
	struct Config
	{
		VkDevice device;
		VkPhysicalDevice physicalDevice;
		VkAllocationCallbacks* alloc;
		SDL_Window* window;
		uint32_t numBuffers;
		size_t vertexMem;
	};
	explicit ImGuiRenderer(APIState& vk);
	void initialize(const Config& conf);
	void startNewFrame(VkCommandBuffer commandBuffer, const VkRect2D& rect);
	void render(VkCommandBuffer commandBuffer, VkImageView renderTarget, VkFormat outputFormat, const VkClearColorValue* clearColor);
	void finalize();
protected:
	bool onWindowEvent(Window& window, const SDL_Event& event) override;
	VkPipeline pipeline(VkFormat outputFormat);
private:
	struct ImGuiInternalState* m_int = nullptr;
	BindingTableLayout m_bindingTableLayout;
	VkFormat m_outputFormat = VK_FORMAT_UNDEFINED;
	VkImageView m_fontAtlasImageView = VK_NULL_HANDLE;
	VkSampler m_fontAtlasSampler = VK_NULL_HANDLE;
	VkDeviceMemory m_imageMem = VK_NULL_HANDLE;
	VkDeviceMemory m_bufferMem = VK_NULL_HANDLE;
	VkShaderModule m_shaderVert = VK_NULL_HANDLE;
	VkShaderModule m_shaderFrag = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;
	VkSampler m_sampler = VK_NULL_HANDLE;
	VkImage m_texture = VK_NULL_HANDLE;
	VkImageView m_srv = VK_NULL_HANDLE;
	std::vector<VkBuffer> m_vertexBuffers;
	BindingTable m_bindingTable;
	uint32_t m_currentBuffer = 0;
	size_t m_bufferBytes = 0;
};

}
