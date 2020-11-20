#include "global.h"

#include "renderer.h"

#include <vkutil.h>

#define VIEW_X 64
#define VIEW_Y 64

static VkRenderPass g_uiOutputPass = VK_NULL_HANDLE;
static VkRenderPass g_sceneOutputPass = VK_NULL_HANDLE;

void sb::Initialize()
{
    VkSubpassDependency spd[]
    {
        {
            VK_SUBPASS_EXTERNAL,
            0,
            VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            0,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        },
        {
            0,
            VK_SUBPASS_EXTERNAL,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
            VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            0
        }
    };
    VkAttachmentDescription attd =
    {
        0,
        vkUtilGetColorBufferFormat(),
        VK_SAMPLE_COUNT_1_BIT,
        VK_ATTACHMENT_LOAD_OP_CLEAR,
        VK_ATTACHMENT_STORE_OP_STORE,
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        VK_ATTACHMENT_STORE_OP_DONT_CARE,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };
    VkAttachmentReference color{ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    VkSubpassDescription sp{};
    sp.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sp.colorAttachmentCount = 1;
    sp.pColorAttachments = &color;
    VkRenderPassCreateInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
    info.attachmentCount = 1;
    info.pAttachments = &attd;
    info.subpassCount = 1;
    info.pSubpasses = &sp;
    info.dependencyCount = 2;
    info.pDependencies = spd;
    VkDevice device = vkUtilGetDevice();
    vkCreateRenderPass(device, &info, nullptr, &g_uiOutputPass);
    attd.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attd.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    vkCreateRenderPass(device, &info, nullptr, &g_sceneOutputPass);    
}

void sb::RenderMainUI()
{
    VkClearValue clearVal;
    clearVal.color = { 0.f, 0.f, 1.f, 1.f };
    VkCommandBuffer cb = vkUtilGetCommandBuffer();
    VkRenderPassBeginInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    info.renderPass = g_uiOutputPass;
    info.framebuffer = vkUtilGetFramebuffer();
    info.renderArea.offset = { 0, 0 };
    info.renderArea.extent = { vkUtilGetViewWidth(), vkUtilGetViewHeight() };
    info.clearValueCount = 1;
    info.pClearValues = &clearVal;
    vkCmdBeginRenderPass(cb, &info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cb);
}

void sb::RenderScene()
{
    VkClearValue clearVal;
    clearVal.color = { 0.f, 1.f, 0.f, 1.f };
    uint32_t viewW = (vkUtilGetViewWidth() >> 1) - VIEW_X - (VIEW_X >> 1);
    uint32_t viewH = (viewW >> 2) * 3;
    VkCommandBuffer cb = vkUtilGetCommandBuffer();
    VkRenderPassBeginInfo info{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
    info.renderPass = g_sceneOutputPass;
    info.framebuffer = vkUtilGetFramebuffer();
    info.renderArea.offset = { VIEW_X, VIEW_Y };
    info.renderArea.extent = { viewW, viewH };
    info.clearValueCount = 1;
    info.pClearValues = &clearVal;
    vkCmdBeginRenderPass(cb, &info, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdEndRenderPass(cb);
}

void sb::RenderOverlay()
{
}

void sb::Finalize()
{
}
