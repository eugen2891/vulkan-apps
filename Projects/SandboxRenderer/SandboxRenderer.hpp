#pragma once

#include <Sandbox/Scene.hpp>
#include <VulkanKit/Window.hpp>
#include <VulkanKit/Context.hpp>
#include <VulkanKit/Resources.hpp>
#include <VulkanKit/Application.hpp>
#include <VulkanKit/ImGuiRenderer.hpp>
#include <VulkanKit/ShaderCompiler.hpp>

class SandboxRenderer : public vulkan::Application
{
public:
	explicit SandboxRenderer();
protected:
	void initialize() override;
	void finalize() override;
	void runApplication() override;
	const char* applicationName() const override;
	bool detectQueues(VkPhysicalDevice physicalDevice) override;
	vulkan::DeviceQueueCreateList queueInfos() const override;
	VkQueue presentQueue() override;
private:
	VkPipeline pipeline();
	void updateSceneAndUploadData();
	vulkan::ImGuiRenderer m_imGuiRenderer;
	vulkan::ShaderCompiler m_shaderCompiler;
	vulkan::ShaderCompiler::Binary m_shaders[2]{};
	VkPipelineLayout m_layout = VK_NULL_HANDLE;
	VkPipeline m_pipeline = VK_NULL_HANDLE;
	uint32_t m_queueFamily = UINT32_MAX;
	VkQueue m_queue = VK_NULL_HANDLE;
	vulkan::Buffer m_vertexData;
	vulkan::Buffer m_indexData;
	vulkan::Buffer m_frameData;
	vulkan::Buffer m_objectData;
	vulkan::Window m_window;
	vulkan::Context m_ctx;
	sandbox::Scene m_scene;
};
