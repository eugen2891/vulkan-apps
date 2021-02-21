#pragma once

#include "framework.h"

namespace vkutil
{

    class GraphicsPipelineState
    {

    public:               

        GraphicsPipelineState(uint32_t stageMask, uint32_t numViews, uint32_t numColorRTs, uint32_t numVAttrs, uint32_t numVBuffers);

        void Apply(VkGraphicsPipelineCreateInfo& info);

        ~GraphicsPipelineState();

        VkViewport* pViewport;

        VkRect2D* pScissorRect;

        VkPipelineShaderStageCreateInfo* ppShaderStage[MAX_SHADER_STAGES];

        VkVertexInputAttributeDescription* pVertexAttr;
        VkVertexInputBindingDescription* pVertexBuffer;
        VkPipelineColorBlendAttachmentState* pColorTarget;

        VkPipelineVertexInputStateCreateInfo vsInputState;
        VkPipelineInputAssemblyStateCreateInfo inputAssembly;
        VkPipelineTessellationStateCreateInfo tesselation;
        VkPipelineViewportStateCreateInfo viewportState;
        VkPipelineRasterizationStateCreateInfo rasterState;
        VkPipelineMultisampleStateCreateInfo multiSample;
        VkPipelineDepthStencilStateCreateInfo depthStencil;
        VkPipelineColorBlendStateCreateInfo colorBlend;
        VkPipelineDynamicStateCreateInfo dynamicState;

    private:

        VkPipelineShaderStageCreateInfo m_shaderStage[MAX_SHADER_STAGES];

        uint32_t m_numActiveShaderStages;

    };

}
