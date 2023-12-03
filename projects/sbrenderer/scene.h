#pragma once

#include <vkk.h>

struct SceneT
{
	Image* images;
	size_t numImages;
	SamplerState* samplers;
	size_t numSamplers;
};

typedef struct SceneT* Scene;

Scene loadScene(const char* path);

void freeScene(Scene scene);
