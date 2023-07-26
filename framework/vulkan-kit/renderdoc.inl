#pragma once

#if WITH_RENDERDOC

#include <SDL2/SDL_loadso.h>

#include "renderdoc-api/renderdoc_app.h"

static RENDERDOC_API_1_6_0* renderDocGetAPI()
{
	static RENDERDOC_API_1_6_0* RenderDoc;
	if (!RenderDoc)
	{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		void* rdcDllHandle = SDL_LoadObject("renderdoc.dll");
#else
#error Not supported yet
#endif
		if (rdcDllHandle)
		{
			int major, minor, patch;
			void* rdcApiGetter = SDL_LoadFunction(rdcDllHandle, "RENDERDOC_GetAPI");
			pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)rdcApiGetter;
			if (RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_0_0,(void**)&RenderDoc) == 1)
			{
				RenderDoc->GetAPIVersion(&major, &minor, &patch);
				debugPrint("RenderDoc API version: %d.%d.%d\n", major, minor, patch);
			}
			else
			{
				debugPrint("Failed to acquire RenderDoc API\n");
			}
		}
	}
	return RenderDoc;
}

void renderDocStartCapture(void)
{
	RENDERDOC_API_1_6_0* rdc = renderDocGetAPI();
	if (rdc && !rdc->IsFrameCapturing())
	{
		rdc->StartFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(Instance), NULL);
	}
}

void renderDocEndCapture(void)
{
	RENDERDOC_API_1_6_0* rdc = renderDocGetAPI();
	if (rdc && rdc->IsFrameCapturing())
	{
		rdc->EndFrameCapture(RENDERDOC_DEVICEPOINTER_FROM_VKINSTANCE(Instance), NULL);
	}
}

#endif
