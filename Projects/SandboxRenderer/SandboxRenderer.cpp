#include "SandboxRenderer.hpp"

#include <VulkanKit/Window.hpp>
#include <VulkanKit/Context.hpp>
#include <ImGui/imgui.h>

VULKAN_APPLICATION_INSTANCE(SandboxRenderer);

SandboxRenderer::SandboxRenderer()
	: m_shaderCompiler{ *this, "../../Framework/" }
	, m_window{ *this, applicationName(), VK_FORMAT_B8G8R8A8_UNORM, { 1024, 1024 } }, m_imGuiRenderer{ *this }
	, m_scene{ "CornellBox2.lua" }
{
	m_window.setEventHandler(&m_imGuiRenderer);
	setOutputWindow(&m_window);
}

void SandboxRenderer::initialize()
{
	m_ctx.initialize({ m_device, m_alloc, m_queue, m_queueFamily, 3 });
	m_imGuiRenderer.initialize({ m_device, m_physicalDevice, m_alloc, m_window.sdlWindow() });

	vulkan::ShaderCompiler::Source shaderSource("SandboxObject.glsl");
	ArrayRef<vulkan::ShaderCompiler::Binary> outputs{ m_shaders, CountOf(m_shaders) };
	m_shaderCompiler.compile(shaderSource, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, outputs);

	VkPushConstantRange pcr{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, 2 * sizeof(VkDeviceAddress)};
	VkPipelineLayoutCreateInfo plci{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO, nullptr, 0, 0, nullptr, 1, &pcr };
	BreakIfFailed(vkCreatePipelineLayout(m_device, &plci, m_alloc, &m_layout));

	vulkan::Image2DCreateInfo ici{ VK_FORMAT_D32_SFLOAT, m_window.rect().extent, 1, 1, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT };
	m_depthBuffer.initialize(*this, ici, vulkan::MemoryFlags::kDeviceOnly);

	m_scene.initialize();
}

void SandboxRenderer::finalize()
{
	vkDestroyPipeline(m_device, m_pipeline, m_alloc);
	vkDestroyPipelineLayout(m_device, m_layout, m_alloc);
	for (auto const& shader : m_shaders) vkDestroyShaderModule(m_device, shader.spirv, m_alloc);
	m_frameData.finalize(*this);
	m_objectData.finalize(*this);
	m_vertexData.finalize(*this);
	m_indexData.finalize(*this);
	m_depthBuffer.finalize(*this);
	m_imGuiRenderer.finalize();
	m_scene.finalize();
	m_ctx.finalize();
}

void SandboxRenderer::runApplication()
{
	while (m_window.pollEvents())
	{
		m_ctx.beginCommandBuffer();
		updateSceneAndUploadData();		

		VkRect2D windowRect = m_window.rect();
		m_imGuiRenderer.startNewFrame(m_ctx.commandBuffer(), windowRect);

		vulkan::PipelineBarrier& barrier = m_ctx.pipelineBarrier();
		barrier.image(m_window.currentImage(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 })
			.layout(VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		barrier.image(m_depthBuffer.handle(), { VK_IMAGE_ASPECT_DEPTH_BIT, 0, 1, 0, 1 })
			.layout(VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
		barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);

		vulkan::ClearValue clearColor{};
		vulkan::ClearValue clearDepth{ 0.f };
		const VkRenderingAttachmentInfo raiColor
		{
			VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, nullptr, m_window.currentImageView(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			VK_RESOLVE_MODE_NONE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, clearColor
		};
		const VkRenderingAttachmentInfo raiDepth
		{
			VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, nullptr, m_depthBuffer.view(), VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL,
			VK_RESOLVE_MODE_NONE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, clearDepth
		};
		const VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO, nullptr, 0, windowRect, 1, 0, 1, &raiColor, &raiDepth };
		vkCmdBeginRendering(m_ctx.commandBuffer(), &ri);
		
		VkDeviceSize bufferDataOffset = 0;
		VkBuffer vertexBuffer = m_vertexData.handle(), indexBuffer = m_indexData.handle();
		vkCmdBindVertexBuffers(m_ctx.commandBuffer(), 0, 1, &vertexBuffer, &bufferDataOffset);
		vkCmdBindIndexBuffer(m_ctx.commandBuffer(), indexBuffer, bufferDataOffset, VK_INDEX_TYPE_UINT32);
		vkCmdBindPipeline(m_ctx.commandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline());

		const VkViewport viewport{ 0.f, 0.f, float(windowRect.extent.width), float(windowRect.extent.height), 0.f, 1.f };
		vkCmdSetViewport(m_ctx.commandBuffer(), 0, 1, &viewport);
		vkCmdSetScissor(m_ctx.commandBuffer(), 0, 1, &windowRect);

		const VkDeviceAddress framePtr = m_frameData.gpuAddress();
		vkCmdPushConstants(m_ctx.commandBuffer(), m_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(VkDeviceAddress), &framePtr);
		for (const sandbox::Drawable& cmd : m_scene.drawables())
		{
			const VkDeviceAddress objectPtr = m_objectData.gpuAddress() + cmd.dataOffset;
			vkCmdPushConstants(m_ctx.commandBuffer(), m_layout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(VkDeviceAddress), sizeof(VkDeviceAddress), &objectPtr);
			vkCmdDrawIndexed(m_ctx.commandBuffer(), cmd.indexCount, 1, cmd.firstIndex, cmd.vertexOffset, 0);
		}

		m_imGuiRenderer.render(m_ctx.commandBuffer(), VK_NULL_HANDLE, m_window.pixelFormat(), nullptr);

		vkCmdEndRendering(m_ctx.commandBuffer());

		barrier.reset();
		barrier.image(m_window.currentImage(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 })
			.layout(VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0);
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

const char* SandboxRenderer::applicationName() const
{
	return "Sandbox Renderer";
}

bool SandboxRenderer::detectQueues(VkPhysicalDevice physicalDevice)
{
	vulkan::QueueFamiliesList queueFamilies(physicalDevice);
	m_queueFamily = queueFamilies.findByFlags(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);
	if (m_queueFamily < queueFamilies.num) return canPresent(physicalDevice, m_queueFamily);
	return false;
}

vulkan::DeviceQueueCreateList SandboxRenderer::queueInfos() const
{
	static float priority = 1.f;
	vulkan::DeviceQueueCreateList queues(1);
	queues.items[0].queueFamilyIndex = m_queueFamily;
	queues.items[0].queueCount = 1;
	return queues;
}

VkQueue SandboxRenderer::presentQueue()
{
	if (m_queue == VK_NULL_HANDLE) vkGetDeviceQueue(m_device, m_queueFamily, 0, &m_queue);
	return m_queue;
}

VkPipeline SandboxRenderer::pipeline()
{
	if (m_pipeline == VK_NULL_HANDLE)
	{
		VkPipelineShaderStageCreateInfo pssci[]
		{
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, m_shaders[0].spirv, "main"},
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, m_shaders[1].spirv, "main" },
		};
		VkVertexInputBindingDescription vibd{ 0, sandbox::GetVertexStride(), VK_VERTEX_INPUT_RATE_VERTEX };
		VkVertexInputAttributeDescription viad[]
		{
			{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 }, { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) },
		};
		VkPipelineVertexInputStateCreateInfo pvisci{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, nullptr, 0, 1, &vibd, CountOf(viad), viad };
		VkPipelineInputAssemblyStateCreateInfo piasci{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO, nullptr, 0, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST };
		VkPipelineTessellationStateCreateInfo ptsci{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
		VkPipelineViewportStateCreateInfo pvsci{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO, nullptr, 0, 1, nullptr, 1, nullptr };
		VkPipelineRasterizationStateCreateInfo prsci
		{
			VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_FALSE, VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE, VK_FALSE, 0.f, 0.f, 0.f, 1.f
		};
		VkPipelineMultisampleStateCreateInfo pmsci{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO, nullptr, 0, VK_SAMPLE_COUNT_1_BIT };
		VkPipelineDepthStencilStateCreateInfo pdssci
		{ 
			VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO, nullptr, 0, VK_TRUE, VK_TRUE, VK_COMPARE_OP_GREATER
		};
		VkPipelineColorBlendAttachmentState pcbas
		{
			VK_FALSE, VK_BLEND_FACTOR_SRC_ALPHA, VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
			VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA, VK_BLEND_FACTOR_ZERO, VK_BLEND_OP_ADD,
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};
		VkPipelineColorBlendStateCreateInfo pcbsci{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO, nullptr, 0, VK_FALSE, VK_LOGIC_OP_NO_OP, 1, &pcbas };
		VkDynamicState ds[]{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		VkPipelineDynamicStateCreateInfo pdsci{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO, nullptr, 0, CountOf(ds), ds };
		VkFormat pixelFormat = m_window.pixelFormat(), depthFormat = VK_FORMAT_D32_SFLOAT;
		VkPipelineRenderingCreateInfo prci{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, nullptr, 0, 1, &pixelFormat, VK_FORMAT_D32_SFLOAT };
		VkGraphicsPipelineCreateInfo gpci
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, &prci, 0, CountOf(pssci), pssci, &pvisci, &piasci, &ptsci, &pvsci, &prsci, &pmsci, &pdssci, &pcbsci, &pdsci,
			m_layout
		};
		BreakIfFailed(vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &gpci, m_alloc, &m_pipeline));
	}
	return m_pipeline;
}

void SandboxRenderer::updateSceneAndUploadData()
{
	m_scene.updateProjection(m_window.aspectRatio());
	ArrayRef<const uint8_t> perFrameData = m_scene.perFrameData();
	ArrayRef<const uint8_t> perObjectData = m_scene.perObjectData();
	ArrayRef<const uint8_t> vertexData, indexData;
	BreakIfNot(perFrameData.num && perObjectData.num);
	{
		if (!m_frameData.valid())
		{
			vulkan::BufferCreateInfo bci{ perFrameData.num, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT };
			m_frameData.initialize(*this, bci, vulkan::MemoryFlags::kDeviceOnly);
		}
		if (!m_objectData.valid())
		{
			vulkan::BufferCreateInfo bci{ perObjectData.num, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT };
			m_objectData.initialize(*this, bci, vulkan::MemoryFlags::kDeviceOnly);
		}
		if (!m_vertexData.valid())
		{
			vertexData = sandbox::GetVertexData();
			vulkan::BufferCreateInfo bci{ vertexData.num, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT };
			m_vertexData.initialize(*this, bci, vulkan::MemoryFlags::kDeviceOnly);
		}
		if (!m_indexData.valid())
		{
			indexData = sandbox::GetIndexData();
			vulkan::BufferCreateInfo bci{ indexData.num, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT };
			m_indexData.initialize(*this, bci, vulkan::MemoryFlags::kDeviceOnly);
		}
		uint32_t indexBarrier = 1, vertexBarrier = 1;
		vulkan::PipelineBarrier& barrier = m_ctx.pipelineBarrier();
		VkPipelineStageFlags stageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.buffer(m_frameData.handle()).access(VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
		barrier.buffer(m_objectData.handle()).access(VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
		if (vertexData.num)
		{
			barrier.buffer(m_vertexData.handle()).access(VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
			stageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			vertexBarrier = 2;
		}
		if (indexData.num)
		{
			barrier.buffer(m_indexData.handle()).access(VK_ACCESS_INDEX_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
			stageMask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			indexBarrier = vertexBarrier + 1;
		}
		barrier.submit(m_ctx.commandBuffer(), stageMask, VK_PIPELINE_STAGE_TRANSFER_BIT);
		vkCmdUpdateBuffer(m_ctx.commandBuffer(), m_frameData.handle(), 0, perFrameData.num, perFrameData.items);
		vkCmdUpdateBuffer(m_ctx.commandBuffer(), m_objectData.handle(), 0, perObjectData.num, perObjectData.items);
		if (vertexData.num) vkCmdUpdateBuffer(m_ctx.commandBuffer(), m_vertexData.handle(), 0, vertexData.num, vertexData.items);
		if (indexData.num) vkCmdUpdateBuffer(m_ctx.commandBuffer(), m_indexData.handle(), 0, indexData.num, indexData.items);
		barrier.buffer(0u).access(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		barrier.buffer(1u).access(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
		if (vertexData.num) barrier.buffer(vertexBarrier).access(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
		if (indexData.num) barrier.buffer(indexBarrier).access(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_INDEX_READ_BIT);
		barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, stageMask);
	}
}
