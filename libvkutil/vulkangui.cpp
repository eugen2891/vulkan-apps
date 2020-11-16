#include "vulkangui.h"

#include <imgui.h>
#include <imgui.cpp>
#include <imgui_draw.cpp>
#include <imgui_widgets.cpp>

VulkanImGuiRenderer::VulkanImGuiRenderer()
{   
    const VulkanAPI& vk = *VulkanAPI::GetInstance();    
}

VulkanImGuiRenderer::~VulkanImGuiRenderer()
{
}

void VulkanImGuiRenderer::RenderAll()
{
    FrameData& frameData = *m_frameData.Get();
    const VulkanAPI& vk = *VulkanAPI::GetInstance();
    /*ImDrawData* pDrawData = ImGui::GetDrawData();
    if (!pDrawData)
        return;

    ImVec2& displayPos = pDrawData->DisplayPos;*/
    frameData.vb.BeginAppend();
    frameData.ib.BeginAppend();


    VkCommandBuffer cb = vk.GetCommandBuffer();
    VkRect2D scissor{ {0, 0}, vk.GetScreenSize() };
    VkViewport viewport{0.f, 0.f, 0.f, 0.f, 0.f, 1.f};
    viewport.width = static_cast<float>(scissor.extent.width);
    viewport.height = static_cast<float>(scissor.extent.height);
    VkRenderPassBeginInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    info.renderPass = m_renderPass;
    info.framebuffer = m_framebuffer;
    info.renderArea.extent = scissor.extent;
    vkCmdBeginRenderPass(cb, &info, VK_SUBPASS_CONTENTS_INLINE);
    //vkCmdBindPipeline(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_finalColorPipeline);
    //vkCmdBindDescriptorSets(cb, VK_PIPELINE_BIND_POINT_GRAPHICS, m_finalColorPipelineLayout, 0, 1, &m_finalColorDescriptorSet, 0, nullptr);
    //projection matrix in push constants
    vkCmdSetViewport(cb, 0, 1, &viewport);
    //for every imgui batch - BEGIN
    vkCmdSetScissor(cb, 0, 1, &scissor);
    vkCmdDraw(cb, 3, 1, 0, 0);
    //for every imgui batch - END
    vkCmdEndRenderPass(cb);

    m_frameData.Next();
}
