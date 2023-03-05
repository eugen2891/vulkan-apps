#pragma once

#include "VulkanApi.hpp"

#include "../Utilities/Array.hpp"

struct shaderc_compiler;
struct shaderc_compilation_result;

namespace vulkan
{

class ShaderCompiler
{
public:
	struct Config
	{
		VkDevice device;
		VkAllocationCallbacks* alloc;
	};
	struct Binary
	{
		VkShaderStageFlags stage;
		VkShaderModule spirv;
		const char* entryPoint;
	};
	using Result = ArrayRef<Binary>;
	uint32_t compile(const char* name, const char* src, VkShaderStageFlags stages, Result& result);
	void initialize(const Config& conf);
	void finalize();
private:
	void createBinary(shaderc_compilation_result* cr, VkShaderStageFlags stage, Binary& binary);
	VkAllocationCallbacks* m_alloc = nullptr;
	shaderc_compiler* m_compiler = nullptr;
	VkDevice m_device = VK_NULL_HANDLE;
};

}
