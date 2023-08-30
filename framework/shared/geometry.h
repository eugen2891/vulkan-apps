#pragma once

#include <cglm/cglm.h>
#include <vulkan/vulkan.h>

#ifndef SPHERE_NUM_PARALLELS
#define SPHERE_NUM_PARALLELS 64
#endif

#ifndef SPHERE_NUM_MERIDIANS
#define SPHERE_NUM_MERIDIANS 64
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct MeshT
{
	VkIndexType indexType;
	uint32_t indexCount;
	const void* meshData;
	size_t indexDataOffset;
	size_t meshDataSize;
};

typedef const struct MeshT* Mesh;

Mesh getSphereMesh(void);

#ifdef __cplusplus
}
#endif
