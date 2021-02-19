#include "shader.h"

#include <new>

#include <stdio.h>
#include <string.h>

static const char* INTERP_BLOCK = 
"struct InterpBlock\n"
"{\n"
"    float4 position : SV_POSITION;\n"
"    float2 uv : TEXCOORD;\n"
"};\n\n";

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

static const char* DEFAULT_MAIN_IMAGE =
"float4 MainImage(in float2 position, in float2 uv)\n"
"{\n"
"    return float4(1.f, 1.f, 1.f, 1.f);"
"}";

static const char* FRAGMENT_SHADER =
"\n\nfloat4 FSMain(InterpBlock input) : SV_TARGET\n"
"{\n"
"    return MainImage(input.position.xy, input.uv);\n"
"}\n\n";

//float3 color = 0.5f + 0.5f * cos(gTime + uv.xyx + float3(0.f, 2.f, 4.f));
//return float4(color, 1.f);

Shader::Shader()
{
    m_vsCodeSize = strlen(INTERP_BLOCK) + strlen(VERTEX_SHADER);
    m_fsBaseSize = strlen(INTERP_BLOCK) + strlen(FRAGMENT_SHADER) + 1;
    m_pVsCode = new (std::nothrow) char[m_vsCodeSize + 1];
    sprintf(m_pVsCode, "%s%s", INTERP_BLOCK, VERTEX_SHADER);
}
