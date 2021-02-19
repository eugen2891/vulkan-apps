#include <new>

#include <vkutil/application.h>

#include <vkutil/shaderutil.h>

#include "shader.h"

class HLSLToy : public vkutil::Application
{    

    Shader m_shader;
    VkRenderPass m_renderPass;
    VkFramebuffer* m_pFramebuffer;
    VkShaderModule m_vertexShader;

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
            m_pFramebuffer = new (std::nothrow) VkFramebuffer[m_numSwapchainFrames];
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
            vkutil::HLSLCode code;
            code.pCode = m_shader.GetVertexShader();
            code.codeSize = m_shader.GetVeretexShaderSize();
            code.stage = vkutil::VERTEX_SHADER;
            if (!m_pHLSLCompiler->Compile(code, &m_vertexShader))
                return false;
        }
        return true;
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
        vkCmdEndRenderPass(m_vkCommandBuffer);
        return true;
    }

    void OnFinalized()
    {
        for (uint32_t i = 0; i < m_numSwapchainFrames; i++)
            vkDestroyFramebuffer(m_vkDevice, m_pFramebuffer[i], m_pVkAlloc);
        vkDestroyShaderModule(m_vkDevice, m_vertexShader, m_pVkAlloc);
        vkDestroyRenderPass(m_vkDevice, m_renderPass, m_pVkAlloc);
        delete[] m_pFramebuffer;
    }

public:

    HLSLToy()
    {
        m_pAppName = "HLSLToy";
        m_windowW = 1280;
        m_windowH = 800;
    }

};

vkutil::Application* CreateApplication()
{
    return new (std::nothrow) HLSLToy();
}
