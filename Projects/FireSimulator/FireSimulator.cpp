#include "FireSimulator.hpp"

#include <VulkanKit/Window.hpp>
#include <VulkanKit/Context.hpp>

#include <chrono>
using namespace std::chrono;

typedef high_resolution_clock::time_point Time;

static Time now() 
{ 
	return high_resolution_clock::now();
}

static float deltaT(Time curr, Time prev)
{
	return duration_cast<duration<float>>(curr - prev).count();
}

struct FireSource
{
	glm::vec2 position;
	glm::vec2 addedVelocity;
	float size ;
	float addedFuel;
};

struct FireParams
{
	glm::vec2 TexelSize{ 1.f / 256.f, 1.f / 256.f };
	VkDeviceAddress sources;
	uint32_t numSources = 1;
	float deltaTime = 0.016f;
	float totalTime = 0.016f;
	float fuelNoise = 0.5f;
	float noiseBlend = 0.1f;
	float burnTemp = 1700.f;
	float cooling = 3000.f;
};

enum FireShader
{
	eFireShader_BeginSimulation,
	eFireShader_VelocityAdvection,
	eFireShader_EndSimulation,
	eFireShader_Display,
	eFireShader_EnumMax,
	eFireShader_NumModules,
};

struct FireShaderPermitaionInfo
{
	const char* name;
	VkShaderStageFlags stages;
	uint32_t count;
};

static const FireShaderPermitaionInfo kFireShaderPermutation[] =
{
	{"BEGIN_SIMULATION", VK_SHADER_STAGE_COMPUTE_BIT, 1},
	{"VELOCITY_ADVECTION", VK_SHADER_STAGE_COMPUTE_BIT, 1},
	{"END_SIMULATION", VK_SHADER_STAGE_COMPUTE_BIT, 1},
	{ nullptr, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 2 }
};

VULKAN_APPLICATION_INSTANCE(FireSimulator);

static const uint32_t kMaxCommandBuffers = 3;

FireSimulator::FireSimulator()
	: m_shaderCompiler{ *this, "./" }
	, m_window{ *this, applicationName(), VK_FORMAT_B8G8R8A8_UNORM, { 256, 256 } }
	, m_bindings{ *this }
	, m_ctx{ *this }
{
	setOutputWindow(&m_window);
}

void FireSimulator::initialize()
{
	m_ctx.initialize(kMaxCommandBuffers);
	m_shaders.resize(eFireShader_NumModules);
	m_bindings.initialize(
		{
			{
				{ 0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT },
				{ 1, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT }
			}
		},
		{
			{ VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(FireParams)},
		}
	);
	vulkan::ShaderCompiler::Source shaderSource("FireSimulator.glsl");
	for (uint32_t i = 0, j = 0; i < eFireShader_EnumMax; i++)
	{
		
		Range<const char*> macros{};
		const char* name = kFireShaderPermutation[i].name;
		if (name)
		{
			macros = { &name, 1 };
		}
		Range<vulkan::ShaderCompiler::Binary> output{ m_shaders.data() + j, kFireShaderPermutation[i].count };
		m_shaderCompiler.compile(shaderSource, kFireShaderPermutation[i].stages, output, macros);
		j += kFireShaderPermutation[i].count;
	}	
}

void FireSimulator::finalize()
{
	m_bindings.finalize();
	m_imageTargets.finalize(*this);
	m_sources->finalize();
	m_ctx.finalize();
}

void FireSimulator::runApplication()
{
	static bool firstFrame = true;
	VkRect2D rect = m_window.rect();	
	m_imageTargets.initialize(*this, rect, m_bindings);

	FireSource sources[]
	{ 
		{ { 0.30f, 0.99f }, { 0.f, -32.f }, 0.02f, 1.f },
		{ { 0.50f, 0.99f }, { 0.f, -32.f }, 0.02f, 1.f },
		{ { 0.70f, 0.99f }, { 0.f, -32.f }, 0.02f, 1.f }
	};
	m_sources = createBuffer(sizeof(sources), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vulkan::MemoryFlags::kDeviceOnly);

	Time curr = now(), start = curr;
	while (m_window.pollEvents())
	{
		FireParams params;
		Time next = now();
		params.totalTime = deltaT(next, start);
		params.deltaTime = deltaT(next, curr);
		params.sources = m_sources->bufferAddress();
		params.numSources = CountOf(sources);
		curr = next;
		
		m_ctx.beginCommandBuffer();

		if (firstFrame)
		{
			params.noiseBlend = 1.f;
			m_imageTargets.resetImages(m_ctx);
			updateSources({ sources, params.numSources });
			firstFrame = false;
		}

		m_imageTargets.bindToContext(m_ctx, m_bindings);
		startSimulation(params);
		syncComputeToCompute();
		velocityAdvection(params);
		syncComputeToCompute();
		endSimulation(params);
		syncComputeToGraphics();
		renderDisplay(0);

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

const char* FireSimulator::applicationName() const
{
	return "FireSimulator";
}

void FireSimulator::updateParameters(const FireParams& params)
{
	vkCmdPushConstants(m_ctx.commandBuffer(), m_bindings.layout(), VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(params), &params);
}

void FireSimulator::updateSources(const Range<FireSource>& sources)
{
	vulkan::PipelineBarrier barrier;
	barrier.buffer(*m_sources).access(VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	vkCmdUpdateBuffer(m_ctx.commandBuffer(), *m_sources, 0, sizeof(FireSource) * sources.num(), sources.get());
	barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}

void FireSimulator::renderDisplay(uint32_t source)
{
	if (m_displayPipeline == VK_NULL_HANDLE)
	{
		VkPipelineShaderStageCreateInfo pssci[]
		{
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_VERTEX_BIT, m_shaders[eFireShader_Display + 0].spirv, "main"},
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_FRAGMENT_BIT, m_shaders[eFireShader_Display + 1].spirv, "main" },
		};
		VkPipelineVertexInputStateCreateInfo pvisci{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
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
		VkFormat pixelFormat = m_window.pixelFormat();
		VkPipelineRenderingCreateInfo prci{ VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO, nullptr, 0, 1, &pixelFormat, VK_FORMAT_D32_SFLOAT };
		VkGraphicsPipelineCreateInfo gpci
		{
			VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO, &prci, 0, CountOf(pssci), pssci, &pvisci, &piasci, &ptsci, &pvsci, &prsci, &pmsci, &pdssci, &pcbsci, &pdsci,
			m_bindings.layout()
		};
		BreakIfFailed(vkCreateGraphicsPipelines(device(), VK_NULL_HANDLE, 1, &gpci, m_alloc, &m_displayPipeline));
	}

	VkRect2D rect = m_window.rect();
	vulkan::PipelineBarrier barrier;
	vulkan::CommandContext ctx(m_ctx.commandBuffer());

	barrier.image(m_window.currentImage(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 })
		.layout(VK_IMAGE_LAYOUT_UNDEFINED, 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
	barrier.memory(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

	vulkan::ClearValue clearColor{ 1.f, 1.f, 0.f, 1.f };
	const VkRenderingAttachmentInfo raiColor
	{
		VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO, nullptr, m_window.currentImageView(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		VK_RESOLVE_MODE_NONE, VK_NULL_HANDLE, VK_IMAGE_LAYOUT_UNDEFINED, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, clearColor
	};
	const VkRenderingInfo ri{ VK_STRUCTURE_TYPE_RENDERING_INFO, nullptr, 0,  rect, 1, 0, 1, &raiColor };
	vkCmdBeginRendering(m_ctx.commandBuffer(), &ri);

	ctx.bindGraphicsPipeline(m_displayPipeline);

	const VkViewport viewport{ 0.f, 0.f, float(rect.extent.width), float(rect.extent.height), 0.f, 1.f };
	vkCmdSetViewport(m_ctx.commandBuffer(), 0, 1, &viewport);
	vkCmdSetScissor(m_ctx.commandBuffer(), 0, 1, &rect);

	ctx.draw(3);

	vkCmdEndRendering(m_ctx.commandBuffer());

	barrier.reset();
	barrier.image(m_window.currentImage(), { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 })
		.layout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, 0);
	barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
}

void FireSimulator::startSimulation(const FireParams& params)
{
	if (m_simulationStart == VK_NULL_HANDLE)
	{
		VkComputePipelineCreateInfo cpci
		{
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0,
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_COMPUTE_BIT, m_shaders[eFireShader_BeginSimulation].spirv, "main"},
			m_bindings.layout()
		};
		m_simulationStart = createComputePipeline(cpci);
	}
	vulkan::CommandContext ctx(m_ctx.commandBuffer());
	ctx.bindComputePipeline(m_simulationStart);
	ctx.pushConstants(m_bindings.layout(), VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(params), &params);
	ctx.dispatch({ 32, 32, 1 });
}

void FireSimulator::velocityAdvection(const FireParams& params)
{
	if (m_velocityAdvection == VK_NULL_HANDLE)
	{
		VkComputePipelineCreateInfo cpci
		{
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0,
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_COMPUTE_BIT, m_shaders[eFireShader_VelocityAdvection].spirv, "main"},
			m_bindings.layout()
		};
		m_velocityAdvection = createComputePipeline(cpci);
	}
	vulkan::CommandContext ctx(m_ctx.commandBuffer());
	ctx.bindComputePipeline(m_velocityAdvection);
	ctx.pushConstants(m_bindings.layout(), VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(params), &params);
	ctx.dispatch({ 32, 32, 1 });
}

void FireSimulator::endSimulation(const FireParams& params)
{
	if (m_simulationEnd == VK_NULL_HANDLE)
	{
		VkComputePipelineCreateInfo cpci
		{
			VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO, nullptr, 0,
			{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO, nullptr, 0, VK_SHADER_STAGE_COMPUTE_BIT, m_shaders[eFireShader_EndSimulation].spirv, "main"},
			m_bindings.layout()
		};
		m_simulationEnd = createComputePipeline(cpci);
	}
	vulkan::CommandContext ctx(m_ctx.commandBuffer());
	ctx.bindComputePipeline(m_simulationEnd);
	ctx.pushConstants(m_bindings.layout(), VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(params), &params);
	ctx.dispatch({ 32, 32, 1 });
}

void FireSimulator::syncComputeToGraphics()
{
	vulkan::PipelineBarrier barrier;
	barrier.memory(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
	barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}

void FireSimulator::syncComputeToCompute()
{
	vulkan::PipelineBarrier barrier;
	barrier.memory(VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT);
	barrier.submit(m_ctx.commandBuffer(), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
}
