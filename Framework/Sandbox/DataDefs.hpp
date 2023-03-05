#ifndef SANDBOX_DATA_DEFS_H
#define SANDBOX_DATA_DEFS_H

#if defined(__cplusplus) || defined(_MSC_VER)
#define GLSL 0
#else
#define GLSL 1
#endif

#if !GLSL

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include "../Utilities/Array.hpp"

namespace sandbox
{

typedef glm::vec3 vec3_t;
typedef glm::vec4 vec4_t;
typedef glm::mat4 mat4_t;

#define STD430 alignas(16)
#define SSBO_REF struct

#else

#define vec3_t vec3
#define vec4_t vec4
#define mat4_t mat4

#define STD430
#define SSBO_REF layout(buffer_reference, std430) buffer

#endif

struct STD430 ViewportData
{
	mat4_t transform;
	mat4_t projection;
	vec4_t dimensions;
};

struct STD430 PointLight
{		
	vec3_t position;
	vec4_t color;
	vec3_t attenuation;
};

struct STD430 LightingData
{
	PointLight pointLight;
};

struct STD430 MeshInstance
{
	mat4_t transform;
};

struct STD430 MaterialData
{
	vec4_t albedoColor;
};

struct STD430 PerObjectData
{
	MeshInstance instance;
	MaterialData material;
};

SSBO_REF STD430 PerFrameData
{
	ViewportData viewport;
	LightingData lighting;
#if GLSL
	PerObjectData objects[];
#else
	ArrayRef<PerObjectData> objects;
#endif
};

#if !GLSL
}//namespace sandbox
#endif

#endif//SANDBOX_DATA_DEFS_H
