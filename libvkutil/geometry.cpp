#include "vecmath.h"

struct Vertex
{
    Vec3 position, normal;
};

struct Triangle
{
    Vertex vertex[3];
};

#define ICOSPHERE_LOD 6
#define ICOSPHERE_A 0.525731112f
#define ICOSPHERE_B 0.850650808f
#define ICOSPHERE_MEM (20*(4<<(ICOSPHERE_LOD-1)))
Triangle g_pIcoSphereVertexData[ICOSPHERE_MEM];

static void GenerateIcoSphere()
{
    static const Vec3 init[]
    {
        {-ICOSPHERE_A,  ICOSPHERE_B, 0.f},
        {ICOSPHERE_A,  ICOSPHERE_B,  0.f},
        {-ICOSPHERE_A, -ICOSPHERE_B, 0.f},
        {ICOSPHERE_A, -ICOSPHERE_B,  0.f},
        {0.f, -ICOSPHERE_A,  ICOSPHERE_B},
        {0.f,  ICOSPHERE_A,  ICOSPHERE_B},
        {0.f, -ICOSPHERE_A, -ICOSPHERE_B},
        {0.f,  ICOSPHERE_A, -ICOSPHERE_B},
        {ICOSPHERE_B, 0.f,  -ICOSPHERE_A},
        {ICOSPHERE_B, 0.f,   ICOSPHERE_A},
        {-ICOSPHERE_B, 0.f, -ICOSPHERE_A},
        {-ICOSPHERE_B, 0.f,  ICOSPHERE_A},
    };

    int stride = ICOSPHERE_MEM / 20;

    g_pIcoSphereVertexData[0  * stride] = { init[0],  init[11], init[5]  };
    g_pIcoSphereVertexData[1  * stride] = { init[0],  init[5],  init[1]  };
    g_pIcoSphereVertexData[2  * stride] = { init[0],  init[1],  init[7]  };
    g_pIcoSphereVertexData[3  * stride] = { init[0],  init[7],  init[10] };
    g_pIcoSphereVertexData[4  * stride] = { init[0],  init[10], init[11] };
    g_pIcoSphereVertexData[5  * stride] = { init[0],  init[5],  init[9]  };
    g_pIcoSphereVertexData[6  * stride] = { init[5],  init[11], init[4]  };
    g_pIcoSphereVertexData[7  * stride] = { init[11], init[10], init[2]  };
    g_pIcoSphereVertexData[8  * stride] = { init[10], init[7],  init[6]  };
    g_pIcoSphereVertexData[9  * stride] = { init[7],  init[1],  init[8]  };
    g_pIcoSphereVertexData[10 * stride] = { init[3],  init[9],  init[4]  };
    g_pIcoSphereVertexData[11 * stride] = { init[3],  init[4],  init[2]  };
    g_pIcoSphereVertexData[12 * stride] = { init[3],  init[2],  init[6]  };
    g_pIcoSphereVertexData[13 * stride] = { init[3],  init[6],  init[8]  };
    g_pIcoSphereVertexData[14 * stride] = { init[3],  init[8],  init[9]  };
    g_pIcoSphereVertexData[15 * stride] = { init[4],  init[9],  init[5]  };
    g_pIcoSphereVertexData[16 * stride] = { init[2],  init[4],  init[11] };
    g_pIcoSphereVertexData[17 * stride] = { init[6],  init[2],  init[10] };
    g_pIcoSphereVertexData[18 * stride] = { init[8],  init[6],  init[7]  };
    g_pIcoSphereVertexData[15 * stride] = { init[9],  init[8],  init[1]  };

    while (stride > 1)
    {
        int next = stride / 4;
        for (int i = 0; i < ICOSPHERE_MEM; i += stride)
        {
            Vec3 point[6]
            {
                g_pIcoSphereVertexData[i].vertex[0].position,
                g_pIcoSphereVertexData[i + 1].vertex[0].position,
                g_pIcoSphereVertexData[i + 2].vertex[0].position,
                {}, {}, {}
            };
            
        }
        stride = next;
    }
}
