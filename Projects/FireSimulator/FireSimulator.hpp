#pragma once

#include "ImageTargets.hpp"

#include <VulkanKit/Window.hpp>
#include <VulkanKit/Context.hpp>
#include <VulkanKit/Application.hpp>
#include <VulkanKit/BindingTable.hpp>
#include <VulkanKit/ShaderCompiler.hpp>

struct FireSource;
struct FireParams;

class FireSimulator : public vulkan::Application
{
public:
	FireSimulator();
protected:
	void initialize() override;
	void finalize() override;
	void runApplication() override;
	const char* applicationName() const override;
private:
	void updateParameters(const FireParams& params);
	void updateSources(const Range<FireSource>& sources);
	void startSimulation(const FireParams& params);
	void velocityAdvection(const FireParams& params);
	void endSimulation(const FireParams& params);
	void renderDisplay(uint32_t source);
	void syncComputeToGraphics();
	void syncComputeToCompute();
	ImageTargets m_imageTargets;
	vulkan::BindingTableLayout m_bindings;
	vulkan::ShaderCompiler m_shaderCompiler;
	VkPipeline m_simulationStart = VK_NULL_HANDLE;
	VkPipeline m_velocityAdvection = VK_NULL_HANDLE;
	VkPipeline m_simulationEnd = VK_NULL_HANDLE;
	VkPipeline m_displayPipeline = VK_NULL_HANDLE;
	std::vector<vulkan::ShaderCompiler::Binary> m_shaders;
	uint32_t m_queueFamily = UINT32_MAX;
	VkQueue m_queue = VK_NULL_HANDLE;
	VkClearColorValue m_clearColor{};
	vulkan::Resource* m_sources;
	vulkan::Window m_window;
	vulkan::Context m_ctx;
};
