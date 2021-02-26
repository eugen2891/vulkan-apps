#include <vkutil/application.h>
#include <vkutil/shaderutil.h>
#include <vkutil/pipeline.h>

#include "shader.h"

class HLSLToy : public vkutil::Application
{

    Shader m_shader;
    VkFramebuffer* m_pFramebuffer = nullptr;
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    vkutil::GraphicsPipelineState* m_pGfxState = nullptr;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;

    struct
    {
        float elapsedTime;
        float elapsedTimeRcp;
        float deltaTime;
        float deltaTimeRcp;
        float viewWidth;
        float viewHeight;
        float viewWidthRcp;
        float viewHeightRcp;
    } m_builtinConst;

    void ProcessArguments(int argc, const char** argv)
    {
        for (int i = 0; i < argc; i++)
        {
            if (*argv[i] == '-')
            {
                char option = argv[i][1];
                switch (option)
                {
                case 'I':
                    m_shader.SetPayloadFile(argv[++i]);
                    break;
                }
            }
        }
    }

    bool OnInitialized()
    {
        {
            VkAttachmentDescription attachmentDescr{};
            attachmentDescr.format = m_swapchainFormat;
            attachmentDescr.samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescr.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescr.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescr.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            VkAttachmentReference attachmentRef;
            attachmentRef.attachment = 0;
            attachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            VkSubpassDescription subpassDescr{};
            subpassDescr.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
            subpassDescr.colorAttachmentCount = 1;
            subpassDescr.pColorAttachments = &attachmentRef;
            VkRenderPassCreateInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
            info.attachmentCount = 1;
            info.pAttachments = &attachmentDescr;
            info.subpassCount = 1;
            info.pSubpasses = &subpassDescr;
            VKUTIL_CHECK_RETURN(vkCreateRenderPass(m_vkDevice, &info, m_pVkAlloc, &m_renderPass), false);
        }
        {
            m_pFramebuffer = new(std::nothrow)VkFramebuffer[m_numSwapchainFrames];
            VkFramebufferCreateInfo info{ VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
            info.renderPass = m_renderPass;
            info.attachmentCount = 1;
            info.width = m_swapchainExtent.width;
            info.height = m_swapchainExtent.height;
            info.layers = 1;
            for (uint32_t i = 0; i < m_numSwapchainFrames; i++)
            {
                info.pAttachments = &m_pSwapchainImageView[i];
                VKUTIL_CHECK_RETURN(vkCreateFramebuffer(m_vkDevice, &info, m_pVkAlloc, &m_pFramebuffer[i]), false);
            }
        }
        {
            VkPushConstantRange builtinRange
            {
                VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(m_builtinConst)
            };
            VkPipelineLayoutCreateInfo info{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
            info.pPushConstantRanges = &builtinRange;
            info.pushConstantRangeCount = 1;
            VKUTIL_CHECK_RETURN(vkCreatePipelineLayout(m_vkDevice, &info, m_pVkAlloc, &m_pipelineLayout), false);

        }
        m_pGfxState = new(std::nothrow)vkutil::GraphicsPipelineState(VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1, 0, 0);
        if (!m_shader.CompileModules())
            return false;
        m_pGfxState->pColorTarget[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        m_pGfxState->ppShaderStage[vkutil::VERTEX_SHADER]->pName = "VSMain";
        m_pGfxState->ppShaderStage[vkutil::VERTEX_SHADER]->module = m_shader.GetVertexShader();
        m_pGfxState->ppShaderStage[vkutil::FRAGMENT_SHADER]->pName = "FSMain";
        m_pGfxState->inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        m_pGfxState->pViewport[0].width = static_cast<float>(m_swapchainExtent.width);
        m_pGfxState->pViewport[0].height = static_cast<float>(m_swapchainExtent.height);
        m_pGfxState->pScissorRect[0].extent.width = m_swapchainExtent.width;
        m_pGfxState->pScissorRect[0].extent.height = m_swapchainExtent.height;
        return m_shader.CompileModules();
    }

    void UpdateState(float elapsedT, float deltaT)
    {
        m_builtinConst.elapsedTime = elapsedT;
        m_builtinConst.elapsedTimeRcp = 1.f / elapsedT;
        m_builtinConst.deltaTime = deltaT;
        m_builtinConst.deltaTimeRcp = 1.f / deltaT;
        m_builtinConst.viewWidth = static_cast<float>(m_swapchainExtent.width);
        m_builtinConst.viewHeight = static_cast<float>(m_swapchainExtent.height);
        m_builtinConst.viewWidthRcp = 1.f / m_builtinConst.viewWidth;
        m_builtinConst.viewHeightRcp = 1.f / m_builtinConst.viewHeight;
    }

    bool RenderFrame(uint32_t contextIdx)
    {
        VkClearValue clearValue{};
        VkRenderPassBeginInfo passBeginInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
        passBeginInfo.framebuffer = m_pFramebuffer[GetSwapchainImageIndex()];
        passBeginInfo.renderPass = m_renderPass;
        passBeginInfo.renderArea.extent.width = m_swapchainExtent.width;
        passBeginInfo.renderArea.extent.height = m_swapchainExtent.height;
        passBeginInfo.pClearValues = &clearValue;
        passBeginInfo.clearValueCount = 1;
        vkCmdBeginRenderPass(m_vkCommandBuffer, &passBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdPushConstants(m_vkCommandBuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(m_builtinConst), &m_builtinConst);
        if (m_shader.HasFragmentShaderChanged() || m_pipeline == VK_NULL_HANDLE)
        {
            if (m_pipeline != VK_NULL_HANDLE)
                vkDestroyPipeline(m_vkDevice, m_pipeline, m_pVkAlloc);
            VkGraphicsPipelineCreateInfo info{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
            m_pGfxState->ppShaderStage[vkutil::FRAGMENT_SHADER]->module = m_shader.GetFragmentShader();
            m_pGfxState->Apply(info);
            info.layout = m_pipelineLayout;
            info.renderPass = m_renderPass;
            VKUTIL_CHECK_RETURN(vkCreateGraphicsPipelines(m_vkDevice, VK_NULL_HANDLE, 1, &info, m_pVkAlloc, &m_pipeline), false);
        }
        vkCmdBindPipeline(m_vkCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
        vkCmdDraw(m_vkCommandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(m_vkCommandBuffer);
        return true;
    }

    void OnFinalized()
    {
        vkDestroyPipeline(m_vkDevice, m_pipeline, m_pVkAlloc);
        for (uint32_t i = 0; i < m_numSwapchainFrames; i++)
            vkDestroyFramebuffer(m_vkDevice, m_pFramebuffer[i], m_pVkAlloc);
        vkDestroyShaderModule(m_vkDevice, m_shader.GetVertexShader(), m_pVkAlloc);
        vkDestroyShaderModule(m_vkDevice, m_shader.GetFragmentShader(), m_pVkAlloc);
        vkDestroyPipelineLayout(m_vkDevice, m_pipelineLayout, m_pVkAlloc);
        vkDestroyRenderPass(m_vkDevice, m_renderPass, m_pVkAlloc);
        delete[] m_pFramebuffer;
        delete m_pGfxState;
    }

public:

    HLSLToy(int argc, const char** argv)
    {
        ProcessArguments(argc, argv);
        m_pAppName = "HLSLToy";
        m_windowW = 1280;
        m_windowH = 800;
    }

};

vkutil::Application* CreateApplication(int argc, const char** argv)
{
    return new(std::nothrow)HLSLToy(argc, argv);
}
