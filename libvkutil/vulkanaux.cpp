#include "vulkanaux.h"

#include <cstring>

VulkanBuffer::~VulkanBuffer()
{
    const VulkanAPI& vk = *VulkanAPI::GetInstance();
    if (m_pMappedMem)
        vkUnmapMemory(vk.GetDevice(), m_memory);
    if (m_handle != VK_NULL_HANDLE)
        vkDestroyBuffer(vk.GetDevice(), m_handle, vk.GetMAlloc());
    if (m_memory != VK_NULL_HANDLE)
        vkFreeMemory(vk.GetDevice(), m_memory, vk.GetMAlloc());
}

void VulkanBuffer::Init(size_t bytes, ResourceFlags flags)
{
    const VulkanAPI& vk = *VulkanAPI::GetInstance();
}

void VulkanBuffer::Write(const void* pSrc, size_t offset, size_t size)
{
    if (m_pMappedMem)
    {
        if ((offset + size) > m_size)
            FAIL_RETURN()
        void* pDst = static_cast<uint8_t*>(m_pMappedMem) + offset;
        memcpy(pDst, pSrc, size);
    }
}

void VulkanBuffer::Append(const void* pSrc, size_t size)
{
    if (m_pMappedMem)
    {
        if ((m_appendPos + size) > m_size)
            FAIL_RETURN()
        void* pDst = static_cast<uint8_t*>(m_pMappedMem) + m_appendPos;
        memcpy(pDst, pSrc, size);
        m_appendPos += size;
    }
}
