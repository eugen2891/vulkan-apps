#pragma once

#include "VulkanApi.hpp"

#include "../Utilities/Array.hpp"

struct shaderc_compiler;
struct shaderc_include_result;
struct shaderc_compilation_result;

namespace vulkan
{

class ShaderCompiler : public APIClient
{
public:
	struct Binary
	{
		VkShaderStageFlags stage;
		VkShaderModule spirv;
		const char* entryPoint;
	};
	struct Source
	{
		Source(const char* fileName);
		~Source();
		char* name;
		char* data;
		size_t size;
	};
	using Result = Range<Binary>;
	void releaseHeader(shaderc_include_result* header) const;
	explicit ShaderCompiler(const APIState& vk, const char* incDir);
	shaderc_include_result* acquireHeader(const char* fileName) const;
	uint32_t compile(const Source& src, VkShaderStageFlags stages, Result& result);
	~ShaderCompiler();
private:
	void createBinary(shaderc_compilation_result* cr, VkShaderStageFlags stage, Binary& binary);
	char includePath[256];
	char* relStart;
	size_t relMax;
};

}
