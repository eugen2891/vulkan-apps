#pragma once

#include "vulkanapi.h"

enum ResourceFlags
{
    VERTEX_BUFFER  = 1,
    INDEX_BUFFER   = 2,
    UNIFORM_BUFFER = 4,
    STORAGE_BUFFER = 8,
    SAMPLED_IMAGE  = VERTEX_BUFFER,
    COLOR_TARGET   = INDEX_BUFFER,
    DEPTH_STENCIL  = UNIFORM_BUFFER,
    STORAGE_IMAGE  = STORAGE_BUFFER,
    CPU_WRITE      = 16,
    CPU_READ       = 32,
    TRANSFER_SRC   = 64,
    TRANSFER_DST   = 128
};

class VulkanBuffer
{

    VkBuffer m_handle = VK_NULL_HANDLE;
    VkDeviceSize m_size = 0;
    VkDeviceMemory m_memory = VK_NULL_HANDLE;
    VkDeviceSize m_offset = 0;
    void* m_pMappedMem = nullptr;
    VkDeviceSize m_appendPos = 0;

public:

    ~VulkanBuffer();

    void Init(size_t bytes, ResourceFlags flags);

    void Write(const void* pSrc, size_t offset, size_t size);    

    template <typename T> void Write(const T& src, size_t offset)
    {
        Write(&src, offset, sizeof(decltype(src)));
    }

    void Append(const void* pSrc, size_t size);

    template <typename T> void Append(const T& src)
    {
        Append(&src, sizeof(decltype(src)));
    }

    void BeginAppend()
    {
        m_appendPos = 0;
    }

};
