#pragma once

namespace vkutil
{

    enum ShaderStage
    {
        VERTEX_SHADER,
        FRAGMENT_SHADER
    };

    struct HLSLCode
    {
        const char* pCode;
        uint32_t codeSize;
        ShaderStage stage;
    };

    struct IHLSLCompiler
    {        
        static IHLSLCompiler* Get();
        virtual bool Initialize(VkDevice device, VkAllocationCallbacks* pVkAlloc) = 0;
        virtual bool Compile(const HLSLCode& code, VkShaderModule* pShaderModule) = 0;
        virtual void Finalize() = 0;
    };

}
