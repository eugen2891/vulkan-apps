#pragma once

#include "vulkanaux.h"

class VulkanImGuiRenderer
{

    VkRenderPass m_renderPass;
    VkFramebuffer m_framebuffer;
    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vbMemory;
    void* m_pVertexData;

    struct FrameData
    {
        VulkanBuffer vb;
        VulkanBuffer ib;
    };

    TRingQueue<FrameData> m_frameData;

    void CreateFrameData(FrameData& frameData);

    void DestroyFrameData(FrameData& frameData);

    void FlipFrameData();

public:

    VulkanImGuiRenderer();
    ~VulkanImGuiRenderer();
    void RenderAll();

};
