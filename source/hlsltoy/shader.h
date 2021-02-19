#pragma once

#include <stdint.h>

class Shader
{

public:

    Shader();

    const char* GetVertexShader() const;

    uint32_t GetVeretexShaderSize() const;

private:

    char* m_pVsCode = nullptr;
    size_t m_vsCodeSize = 0;
    size_t m_fsBaseSize = 0;

};

inline const char* Shader::GetVertexShader() const
{
    return m_pVsCode;
}

inline uint32_t Shader::GetVeretexShaderSize() const
{
    return m_vsCodeSize & UINT32_MAX;
}
