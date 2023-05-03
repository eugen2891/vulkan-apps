#pragma once

#include <VulkanKit/Window.hpp>
#include <VulkanKit/Context.hpp>
#include <VulkanKit/Application.hpp>
#include <VulkanKit/ImGuiRenderer.hpp>

class ImGuiWidgetsDemo : public vulkan::Application
{
public:
	explicit ImGuiWidgetsDemo();
protected:
	void initialize() override;
	void finalize() override;
	void runApplication() override;
	const char* applicationName() const override;
private:
	vulkan::ImGuiRenderer m_imGuiRenderer;
	uint32_t m_queueFamily = UINT32_MAX;
	VkQueue m_queue = VK_NULL_HANDLE;
	VkClearColorValue m_clearColor{};
	vulkan::Window m_window;
	vulkan::Context m_ctx;
};
