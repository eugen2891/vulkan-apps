#include "shader.h"

#include <new>

#include <stdio.h>
#include <string.h>

#include <vkutil/shaderutil.h>

static const char* INTERP_BLOCK = 
"struct InterpBlock\n"
"{\n"
"    float4 position : SV_POSITION;\n"
"    float2 uv : TEXCOORD;\n"
"};\n\n";

static const char* BUILTIN_CONST =
"[[vk::push_constant]] struct\n"
"{\n"
"    float elapsedTime;\n"
"    float elapsedTimeRcp;\n"
"    float deltaTime;\n"
"    float deltaTimeRcp;\n"
"    float2 viewSize;\n"
"    float2 viewSizeRcp;\n"
"} G;\n\n"
;

static const char* VERTEX_SHADER =
"void FullscreenTirangle(in uint index, out float4 pos, out float2 uv)\n"
"{\n"
//CCW: (-1,-1)->(-1,3)->(3,-1) INVERT-Y REQUIRED
"    pos = float4(0.f, 0.f, 0.f, 1.f);\n"
"    pos.x = -1.f + ((index == 2) ? 4.f : 0.f);\n"
"    pos.y = 1.f - ((index == 1) ? 4.f : 0.f);\n"
//CCW: (0,0)->(0,2)->(2,0)
"    uv.x = 0.f + ((index == 2) ? 2.f : 0.f);\n"
"    uv.y = 0.f + ((index == 1) ? 2.f : 0.f);\n"
"}\n\n"
"InterpBlock VSMain(uint index : SV_VERTEXID)\n"
"{\n"
"    InterpBlock output;\n"
"    FullscreenTirangle(index, output.position, output.uv);\n"
"    return output;\n"
"}\n\n";

static const char* DEFAULT_PAYLOAD =
"float4 MainImage(in float2 position, in float2 uv)\n"
"{\n"
#if 0
"    return float4(1.f, 1.f, 1.f, 1.f);"
#else
"    float3 color = 0.5f + 0.5f * cos(G.elapsedTime + uv.xyx + float3(0.f, 2.f, 4.f));\n"
"    return float4(color, 1.f);\n"
#endif
"}";


static const char* FRAGMENT_SHADER =
"\n\nfloat4 FSMain(InterpBlock input) : SV_TARGET\n"
"{\n"
"    return MainImage(input.position.xy, input.uv);\n"
"}\n\n";

Shader::Shader()
{
    m_vsCodeSize = strlen(INTERP_BLOCK) + strlen(VERTEX_SHADER);
    m_fsBaseSize = strlen(INTERP_BLOCK) + strlen(BUILTIN_CONST) + strlen(FRAGMENT_SHADER);
    m_pFsCode = new (std::nothrow) char[m_fsBaseSize + PAYLOAD_MAX + 1];
    m_pVsCode = new (std::nothrow) char[m_vsCodeSize + 1];
    sprintf(m_pVsCode, "%s%s", INTERP_BLOCK, VERTEX_SHADER);
}

bool Shader::CompileModules()
{
    if (m_vertexShader == VK_NULL_HANDLE)
    {
        vkutil::HLSLCode code;
        code.pCode = m_pVsCode;
        code.codeSize = m_vsCodeSize & UINT32_MAX;
        code.stage = vkutil::VERTEX_SHADER;
        if (!vkutil::IHLSLCompiler::Get()->Compile(code, &m_vertexShader))
            return false;
    }
    {
        if (!m_pFsMain)
        {
            m_pFsMain = new (std::nothrow) char[PAYLOAD_MAX + 1];
            strcpy(m_pFsMain, DEFAULT_PAYLOAD);
        }        
        sprintf(m_pFsCode, "%s%s%s%s", INTERP_BLOCK, BUILTIN_CONST, m_pFsMain, FRAGMENT_SHADER);
        vkutil::HLSLCode code;
        code.pCode = m_pFsCode;
        code.codeSize = (m_fsBaseSize + strlen(m_pFsMain)) & UINT32_MAX;
        code.stage = vkutil::FRAGMENT_SHADER;
        if (!vkutil::IHLSLCompiler::Get()->Compile(code, &m_fragmentShader))
            return false;
    }
    m_hasChanged = true;
    return true;
}

Shader::~Shader()
{
    delete[] m_pFsMain;
    delete[] m_pFsCode;
    delete[] m_pVsCode;
}
