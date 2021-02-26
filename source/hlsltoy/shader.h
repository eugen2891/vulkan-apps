#pragma once

#include <vkutil/framework.h>

class Shader
{

public:


    Shader();

    void SetPayloadFile(const char* pFile);

    VkShaderModule GetVertexShader() const;

    VkShaderModule GetFragmentShader() const;

    bool HasFragmentShaderChanged();

    bool CompileModules();

    ~Shader();

private:

    static const size_t PAYLOAD_MAX = 65536;

    VkShaderModule m_vertexShader;
    VkShaderModule m_fragmentShader;
    const char* m_pPayloadFile;
    char* m_pVsCode;
    char* m_pFsCode;
    char* m_pPayload;
    size_t m_vsCodeSize;
    size_t m_fsBaseSize;
    bool m_hasChanged;

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

inline void Shader::SetPayloadFile(const char* pFile)
{
    m_pPayloadFile = pFile;
}
