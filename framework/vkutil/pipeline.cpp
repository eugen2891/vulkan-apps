#include "pipeline.h"

#include <string.h>

vkutil::GraphicsPipelineState::GraphicsPipelineState(uint32_t stageMask, uint32_t numViews, uint32_t numColorRTs, uint32_t numVAttrs, uint32_t numVBuffers)
    : pViewport(nullptr)
    , pScissorRect(nullptr)
    , pVertexAttr(nullptr)
    , pVertexBuffer(nullptr)
    , pColorTarget(nullptr)
{
    m_numActiveShaderStages = 0;
    memset(ppShaderStage, 0, sizeof(ppShaderStage));
    memset(m_shaderStage, 0, sizeof(m_shaderStage));
    if (stageMask & VK_SHADER_STAGE_VERTEX_BIT)
    {
        VkPipelineShaderStageCreateInfo& info = m_shaderStage[m_numActiveShaderStages++];
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        ppShaderStage[VERTEX_SHADER] = &info;
    }
    if (stageMask & VK_SHADER_STAGE_FRAGMENT_BIT)
    {
        VkPipelineShaderStageCreateInfo& info = m_shaderStage[m_numActiveShaderStages++];
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        ppShaderStage[FRAGMENT_SHADER] = &info;
    }

    if (numVAttrs)
    {
        pVertexAttr = new(std::nothrow) VkVertexInputAttributeDescription[numVAttrs];
        memset(pVertexAttr, 0, sizeof(VkVertexInputAttributeDescription) * numVAttrs);
    }
    if (numVBuffers)
    {
        pVertexBuffer = new(std::nothrow) VkVertexInputBindingDescription[numVBuffers];
        memset(pVertexBuffer, 0, sizeof(VkVertexInputBindingDescription) * numVBuffers);
    }
    vsInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vsInputState.pNext = nullptr;
    vsInputState.flags = 0;
    vsInputState.vertexBindingDescriptionCount = numVBuffers;
    vsInputState.pVertexBindingDescriptions = pVertexBuffer;
    vsInputState.vertexAttributeDescriptionCount = numVAttrs;
    vsInputState.pVertexAttributeDescriptions = pVertexAttr;

    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.pNext = nullptr;
    inputAssembly.flags = 0;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    tesselation.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tesselation.pNext = nullptr;
    tesselation.flags = 0;
    tesselation.patchControlPoints = 0;

    if (numViews)
    {
        pViewport = new(std::nothrow) VkViewport[numViews];
        pScissorRect = new(std::nothrow) VkRect2D[numViews];
        memset(pViewport, 0, sizeof(VkViewport) * numViews);
        memset(pScissorRect, 0, sizeof(VkRect2D) * numViews);
    }
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.flags = 0;
    viewportState.viewportCount = numViews;
    viewportState.pViewports = pViewport;
    viewportState.scissorCount = numViews;
    viewportState.pScissors = pScissorRect;

    memset(&rasterState, 0, sizeof(rasterState));
    rasterState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterState.lineWidth = 1.f;

    memset(&multiSample, 0, sizeof(multiSample));
    multiSample.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    memset(&depthStencil, 0, sizeof(depthStencil));
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    
    memset(&dynamicState, 0, sizeof(dynamicState));
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;

    if (numColorRTs)
    {
        pColorTarget = new(std::nothrow) VkPipelineColorBlendAttachmentState[numColorRTs];
        memset(pColorTarget, 0, sizeof(VkPipelineColorBlendAttachmentState) * numColorRTs);
    }
    colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlend.pNext = nullptr;
    colorBlend.flags = 0;
    colorBlend.logicOpEnable = VK_FALSE;
    colorBlend.logicOp = VK_LOGIC_OP_CLEAR;
    colorBlend.attachmentCount = numColorRTs;
    colorBlend.pAttachments = pColorTarget;
    colorBlend.blendConstants[0] = 1.f;
    colorBlend.blendConstants[1] = 1.f;
    colorBlend.blendConstants[2] = 1.f;
    colorBlend.blendConstants[3] = 1.f;
}

void vkutil::GraphicsPipelineState::Apply(VkGraphicsPipelineCreateInfo& info)
{
    info.stageCount = m_numActiveShaderStages;
    info.pStages = m_shaderStage;
    info.pVertexInputState = &vsInputState;
    info.pInputAssemblyState = &inputAssembly;
    info.pTessellationState = &tesselation;
    info.pViewportState = &viewportState;
    info.pRasterizationState = &rasterState;
    info.pMultisampleState = &multiSample;
    info.pDepthStencilState = &depthStencil;
    info.pColorBlendState = &colorBlend;
    info.pDynamicState = &dynamicState;
}

vkutil::GraphicsPipelineState::~GraphicsPipelineState()
{
    delete[] pViewport;
    delete[] pScissorRect;
    delete[] pColorTarget;
    delete[] pVertexBuffer;
    delete[] pVertexAttr;
}
