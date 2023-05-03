#include "ImGuiWidgetsDemo.hpp"

#include <VulkanKit/Window.hpp>
#include <VulkanKit/Context.hpp>
#include <ImGui/imgui.h>

VULKAN_APPLICATION_INSTANCE(ImGuiWidgetsDemo);

static const uint32_t kMaxCommandBuffers = 3;

ImGuiWidgetsDemo::ImGuiWidgetsDemo()
	: m_window{ *this, applicationName(), VK_FORMAT_B8G8R8A8_UNORM, { 1440, 900 } }
	, m_imGuiRenderer{ *this }
	, m_ctx{ *this }
{
	m_window.setEventHandler(&m_imGuiRenderer);
	setOutputWindow(&m_window);
}

void ImGuiWidgetsDemo::initialize()
{
	m_ctx.initialize(kMaxCommandBuffers);
	m_imGuiRenderer.initialize({ m_device, m_physicalDevice, m_alloc, m_window.sdlWindow() });
}

void ImGuiWidgetsDemo::finalize()
{
	m_imGuiRenderer.finalize();
	m_ctx.finalize();
}

void ImGuiWidgetsDemo::runApplication()
{
	while (m_window.pollEvents())
	{
		m_ctx.beginCommandBuffer();

		VkRect2D windowRect = m_window.rect();
		m_imGuiRenderer.startNewFrame(m_ctx.commandBuffer(), windowRect);

		ImGui::ShowDemoWindow();

		vulkan::PipelineBarrier& barrier = m_ctx.pipelineBarrier();
		barrier.image(m_window.currentImage(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 })
			.layout(VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

		m_imGuiRenderer.render(m_ctx.commandBuffer(), m_window.currentImageView(), m_window.pixelFormat(), &m_clearColor);

		barrier.image(0).layout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0);
		barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		m_ctx.endCommandBuffer();

		if (m_window.swapchainValid())
		{
			VkSemaphore imgOk = m_window.currentSemaphore();
			VkPipelineStageFlags imgStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			VkSemaphore cmdOk = m_ctx.submit({ &imgOk, 1 }, &imgStage);
			m_window.present({ &cmdOk, 1 });
		}
	}
}

const char* ImGuiWidgetsDemo::applicationName() const
{
	return "ImGui Widgets Demo";
}
