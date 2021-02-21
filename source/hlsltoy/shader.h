#pragma once

#include <vkutil/framework.h>

class Shader
{

public:


    Shader();

    VkShaderModule GetVertexShader() const;

    VkShaderModule GetFragmentShader() const;

    bool HasFragmentShaderChanged();

    bool CompileModules();

    ~Shader();

private:

    static const size_t PAYLOAD_MAX = 65536;

    VkShaderModule m_vertexShader = VK_NULL_HANDLE;
    VkShaderModule m_fragmentShader = VK_NULL_HANDLE;
    char* m_pVsCode = nullptr;
    char* m_pFsCode = nullptr;
    char* m_pFsMain = nullptr;
    size_t m_vsCodeSize = 0;
    size_t m_fsBaseSize = 0;
    bool m_hasChanged = true;

};

inline VkShaderModule Shader::GetVertexShader() const
{
    return m_vertexShader;
}

inline VkShaderModule Shader::GetFragmentShader() const
{
    return m_fragmentShader;
}

inline bool Shader::HasFragmentShaderChanged()
{
    bool retVal = m_hasChanged;
    m_hasChanged = false;
    return retVal;
}
