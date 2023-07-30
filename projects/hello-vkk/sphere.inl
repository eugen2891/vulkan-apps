#pragma once

#include <cglm/cglm.h>

#ifndef SPHERE_NUM_PARALLELS
#define SPHERE_NUM_PARALLELS 32
#endif

#ifndef SPHERE_NUM_MERIDIANS
#define SPHERE_NUM_MERIDIANS 32
#endif

struct SphereData
{
	vec3 vertices[(SPHERE_NUM_PARALLELS - 1) * SPHERE_NUM_MERIDIANS + 2];
	uint32_t indices[3 * (2 * SPHERE_NUM_PARALLELS - 2) * SPHERE_NUM_MERIDIANS];
};

static const struct SphereData* getSphereMesh(void)
{
	static uint32_t numVertices;
	static struct SphereData mesh;
	if (numVertices == 0)
	{
		numVertices = (SPHERE_NUM_PARALLELS - 1) * SPHERE_NUM_MERIDIANS + 2;
		const float numParRcp = 1.f / (float)SPHERE_NUM_PARALLELS;
		const float numMerRcp = 1.f / (float)SPHERE_NUM_MERIDIANS;

		mesh.vertices[0][0] = 0.f;
		mesh.vertices[0][1] = 1.f;
		mesh.vertices[0][2] = 0.f;
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
				mesh.vertices[index][0] = x;
				mesh.vertices[index][1] = y;
				mesh.vertices[index][2] = z;
			}
		}
		mesh.vertices[numVertices - 1][0] = 0.f;
		mesh.vertices[numVertices - 1][1] = -1.f;
		mesh.vertices[numVertices - 1][2] = 0.f;

		uint32_t indexPos = 0;

		for (uint32_t i = 0; i < SPHERE_NUM_MERIDIANS; ++i)
		{
			const uint32_t a = i + 1;
			const uint32_t b = (i + 1) % SPHERE_NUM_MERIDIANS + 1;
			mesh.indices[indexPos++] = 0;
			mesh.indices[indexPos++] = b;
			mesh.indices[indexPos++] = a;
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
				mesh.indices[indexPos++] = a;
				mesh.indices[indexPos++] = a1;
				mesh.indices[indexPos++] = b1;
				mesh.indices[indexPos++] = a;
				mesh.indices[indexPos++] = b1;
				mesh.indices[indexPos++] = b;
			}
		}

		for (uint32_t i = 0; i < SPHERE_NUM_MERIDIANS; ++i)
		{
			const uint32_t a = i + SPHERE_NUM_MERIDIANS * (SPHERE_NUM_PARALLELS - 2) + 1;
			const uint32_t b = (i + 1) % SPHERE_NUM_MERIDIANS + SPHERE_NUM_MERIDIANS * (SPHERE_NUM_PARALLELS - 2) + 1;
			mesh.indices[indexPos++] = numVertices - 1;
			mesh.indices[indexPos++] = a;
			mesh.indices[indexPos++] = b;
		}
	}
	return &mesh;
}


