#pragma once

#include <Sandbox/Scene.hpp>
#include <VulkanKit/Window.hpp>
#include <VulkanKit/Context.hpp>
#include <VulkanKit/Application.hpp>
#include <VulkanKit/ImGuiRenderer.hpp>
#include <VulkanKit/ShaderCompiler.hpp>

namespace vulkan
{
class Resource;
}

class SandboxRenderer : public vulkan::Application
{
public:
	explicit SandboxRenderer();
protected:
	void initialize() override;
	void finalize() override;
	void runApplication() override;
	const char* applicationName() const override;
private:
	VkPipeline pipeline();
	void updateSceneAndUploadData();
	vulkan::ImGuiRenderer m_imGuiRenderer;
	vulkan::ShaderCompiler m_shaderCompiler;
	vulkan::ShaderCompiler::Binary m_shaders[2]{};
	VkPipelineLayout m_layout = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;
	vulkan::Resource* m_vertexData = nullptr;
	vulkan::Resource* m_indexData = nullptr;
	vulkan::Resource* m_frameData = nullptr;
	vulkan::Resource* m_objectData = nullptr;
	vulkan::Resource* m_depthBuffer;
	vulkan::Window m_window;
	vulkan::Context m_ctx;
	sandbox::Scene m_scene;
};
