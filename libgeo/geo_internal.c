#include "geo.h"

#define GEO_INTERNAL
#include "geo_internal.h"

geo_result_t geo_initialize(geo_init_flags_t flags)
{
    switch (flags & GEO_INIT_API_MASK)
    {
    case GEO_INIT_VULKAN:
        return GEO_RESULT_FAIL;
        break;
    case GEO_INIT_D3D11:
    case GEO_INIT_D3D12:
        return GEO_RESULT_FAIL;
        break;
    case GEO_INIT_HEADLESS:
    default:
        break;
    }
    return GEO_RESULT_OK;
}

void geo_finalize(void)
{

}
