#include "geometry.h"

Mesh getSphereMesh(void)
{
	static struct MeshT mesh;
	static struct SphereData
	{
		vec3 vertices[(SPHERE_NUM_PARALLELS - 1) * SPHERE_NUM_MERIDIANS + 2];
		uint32_t indices[3 * (2 * SPHERE_NUM_PARALLELS - 2) * SPHERE_NUM_MERIDIANS];
	} sphere;
	if (mesh.indexCount == 0)
	{
		const uint32_t numVertices = (SPHERE_NUM_PARALLELS - 1) * SPHERE_NUM_MERIDIANS + 2;
		const float numParRcp = 1.f / (float)SPHERE_NUM_PARALLELS;
		const float numMerRcp = 1.f / (float)SPHERE_NUM_MERIDIANS;

		sphere.vertices[0][0] = 0.f;
		sphere.vertices[0][1] = 1.f;
		sphere.vertices[0][2] = 0.f;
		for (uint32_t i = 0; i < (SPHERE_NUM_PARALLELS - 1); i++)
		{
			const float polar = (float)M_PI * (float)(i + 1) * numParRcp;
			const float sp = sinf(polar), cp = cosf(polar);
			for (uint32_t j = 0; j < SPHERE_NUM_MERIDIANS; j++)
			{
				const uint32_t index = i * SPHERE_NUM_MERIDIANS + j + 1;
				const float azimuth = 2.f * (float)M_PI * (float)j * numMerRcp;
				const float sa = sinf(azimuth), ca = cosf(azimuth);
				const float x = sp * ca, y = cp, z = sp * sa;
				sphere.vertices[index][0] = x;
				sphere.vertices[index][1] = y;
				sphere.vertices[index][2] = z;
			}
		}
		sphere.vertices[numVertices - 1][0] = 0.f;
		sphere.vertices[numVertices - 1][1] = -1.f;
		sphere.vertices[numVertices - 1][2] = 0.f;

		uint32_t indexPos = 0;

		for (uint32_t i = 0; i < SPHERE_NUM_MERIDIANS; ++i)
		{
			const uint32_t a = i + 1;
			const uint32_t b = (i + 1) % SPHERE_NUM_MERIDIANS + 1;
			sphere.indices[indexPos++] = 0;
			sphere.indices[indexPos++] = b;
			sphere.indices[indexPos++] = a;
		}

		for (uint32_t i = 0; i < SPHERE_NUM_PARALLELS - 2; ++i)
		{
			const uint32_t aStart = i * SPHERE_NUM_MERIDIANS + 1;
			const uint32_t bStart = (i + 1) * SPHERE_NUM_MERIDIANS + 1;
			for (uint32_t j = 0; j < SPHERE_NUM_MERIDIANS; ++j)
			{
				const uint32_t a = aStart + j;
				const uint32_t a1 = aStart + (j + 1) % SPHERE_NUM_MERIDIANS;
				const uint32_t b = bStart + j;
				const uint32_t b1 = bStart + (j + 1) % SPHERE_NUM_MERIDIANS;
				sphere.indices[indexPos++] = a;
				sphere.indices[indexPos++] = a1;
				sphere.indices[indexPos++] = b1;
				sphere.indices[indexPos++] = a;
				sphere.indices[indexPos++] = b1;
				sphere.indices[indexPos++] = b;
			}
		}

		for (uint32_t i = 0; i < SPHERE_NUM_MERIDIANS; ++i)
		{
			const uint32_t a = i + SPHERE_NUM_MERIDIANS * (SPHERE_NUM_PARALLELS - 2) + 1;
			const uint32_t b = (i + 1) % SPHERE_NUM_MERIDIANS + SPHERE_NUM_MERIDIANS * (SPHERE_NUM_PARALLELS - 2) + 1;
			sphere.indices[indexPos++] = numVertices - 1;
			sphere.indices[indexPos++] = a;
			sphere.indices[indexPos++] = b;
		}
		mesh.indexCount = 3 * (2 * SPHERE_NUM_PARALLELS - 2) * SPHERE_NUM_MERIDIANS;
		mesh.indexType = VK_INDEX_TYPE_UINT32;
		mesh.meshData = &sphere;
		mesh.meshDataSize = sizeof(sphere);
		mesh.indexDataOffset = sizeof(sphere.vertices);
	}
	return &mesh;
}
