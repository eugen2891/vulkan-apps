#pragma once

SamplerState createSamplerState(VkFilter minMag, VkSamplerMipmapMode mipMode, VkSamplerAddressMode addressMode)
{
	VkSamplerCreateInfo sci = {
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = minMag,
		.minFilter = minMag,
		.mipmapMode = mipMode,
		.addressModeU = addressMode,
		.addressModeV = addressMode,
		.addressModeW = addressMode
	};
	VkSampler retval = VK_NULL_HANDLE;
	breakIfFailed(vkCreateSampler(Device, &sci, Alloc, &retval));
	return retval;
}

void destroySamplerState(SamplerState sampler)
{
	vkDestroySampler(Device, sampler, Alloc);
}

