#pragma once

typedef enum geo_result_t
{
    GEO_RESULT_OK
} geo_result_t;

typedef enum geo_init_flags_t
{
    GEO_INIT_HEADLESS,
    GEO_INIT_VULKAN,
    GEO_INIT_D3D11,
    GEO_INIT_D3D12,
    GEO_INIT_API_MASK = 0x3
} geo_init_flags_t;

typedef enum geo_mesh_t
{
    GEO_MESH_CUBE,
    GEO_MESH_PYRAMID,
    GEO_MESH_SPHERE,
    GEO_MESH_MAX
} geo_mesh_t;

#define GEO_NONE ((void*)0)

typedef struct geo_scene_t* geo_scene_t;

typedef struct geo_instance_t* geo_instance_t;

geo_result_t geo_initialize(geo_init_flags_t flags);

void geo_begin_frame(void);

void geo_draw_simple_skybox(void);

void geo_end_frame(void);

/*

geo_scene_t - mesh instances, camera, lights

geo_instance_t - instance properties

geo_scene_add_instance()

geo_scene_set_property()

geo_instance_set_property()

geo_begin_frame()

geo_draw_simple_skybox()

geo_end_frame()

by default, empty scene is constructed:
- camera at 0,0,0 pointing towards -Z
- simple sky box render pass

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    //perform in a vertex shader
    vec3 viewDir = vec3(0.0, 0.0, -1.0); // normalized view dir
    float compViewY = 1.0 / sqrt(1.0 - viewDir.y * viewDir.y);
    float g = viewDir.y * compViewY;
    //apply offset g to V

    vec2 uv = ((fragCoord / iResolution.xy) + g) * 2.0 - 1.0;
    float skyWeight = step(0.0, uv.y);

    vec4 skyColor = vec4(0.0, 0.8, 1.0, 1.0);
    vec4 sandColor = vec4(0.93, 0.79, 0.68, 1.0);
    vec4 horizonColor = vec4(1.0, 1.0, 1.0, 1.0);
    float alpha0 = smoothstep(0.0, 1.0, 1.5 * abs(uv.y));
    float alpha1 = 1.0 - smoothstep(0.0, 1.0, 9.0 * abs(uv.y));
    vec4 aboveColor = mix(horizonColor, skyColor, alpha0);
    vec4 belowColor = mix(sandColor, horizonColor, alpha1);

    fragColor = mix(belowColor, aboveColor, skyWeight);
}

*/
