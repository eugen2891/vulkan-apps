#include "scene.h"

#define CGLTF_IMPLEMENTATION

#include <cgltf/cgltf.h>

static void initImages(Scene, cgltf_data*);
static void initSamplers(Scene, cgltf_data*);

Scene loadScene(const char* path)
{
	Scene retval = NULL;
	if (path)
	{
		cgltf_data* data = NULL;
		cgltf_options options = { 0 };
		cgltf_result result = cgltf_parse_file(&options, path, &data);
		if (result == cgltf_result_success)
		{
			Scene scene = calloc(1, sizeof(struct SceneT));
			if (scene)
			{
				initImages(scene, data);
				initSamplers(scene, data);
				retval = scene;
			}
			cgltf_free(data);

		}
	}
	return retval;
}

void freeScene(Scene scene)
{
	if (scene)
	{
		for (size_t i = 0; i < scene->numSamplers; i++)
		{
			destroySamplerState(scene->samplers[i]);
		}
		freeMem(scene->samplers);
		freeMem(scene);
	}
}

void initImages(Scene scene, cgltf_data* gltf)
{
	scene->numImages = gltf->images_count;
	scene->samplers = calloc(gltf->samplers_count, sizeof(SamplerState));
	if (!scene->samplers)
	{
		scene->numSamplers = 0;
		return;
	}
}

void initSamplers(Scene scene, cgltf_data* gltf)
{
	scene->numSamplers = gltf->samplers_count;
	scene->samplers = calloc(gltf->samplers_count, sizeof(SamplerState));
	if (!scene->samplers)
	{
		scene->numSamplers = 0;
		return;
	}
	for (cgltf_size i = 0; i < gltf->samplers_count; i++)
	{
		VkFilter minMag = VK_FILTER_NEAREST; 
		VkSamplerMipmapMode mipMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		const cgltf_sampler* samp = &gltf->samplers[i];
		switch (samp->mag_filter)
		{
		case 9728: /* cgltf_sampler_magfilter_nearest */
			minMag = VK_FILTER_NEAREST;
			break;
		case 9729: /* cgltf_sampler_magfilter_linear */
			minMag = VK_FILTER_LINEAR;
			break;
		}
		switch (samp->min_filter)
		{
		case 9728: /* cgltf_sampler_minfilter_nearest */
		case 9984: /* cgltf_sampler_minfilter_nearest_mipmap_nearest */
			/* warning if mag mode is linear */
			minMag = VK_FILTER_NEAREST;
			break;
		case 9986: /* cgltf_sampler_minfilter_nearest_mipmap_linear */
			/* warning if mag mode is linear */
			minMag = VK_FILTER_NEAREST;
			mipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		case 9729: /* cgltf_sampler_minfilter_linear */
		case 9985: /* cgltf_sampler_minfilter_linear_mipmap_nearest */
			/* warning if mag mode is nearest */
			minMag = VK_FILTER_LINEAR;
			break;
		case 9987: /* cgltf_sampler_minfilter_linear_mipmap_linear */
			/* warning if mag mode is nearest */
			minMag = VK_FILTER_LINEAR;
			mipMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		}
		/* warning if wrap is not the same */
		switch (samp->wrap_s)
		{
		case 33071: /* cgltf_sampler_wrap_clamp_to_edge */
			addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case 33648: /* cgltf_sampler_wrap_mirrored_repeat */
			addressMode = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			break;
		case 10497: /* cgltf_sampler_wrap_repeat */
			addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		}
		scene->samplers[i] = createSamplerState(minMag, mipMode, addressMode);
	}
}

