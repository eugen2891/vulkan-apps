#include "ShaderCompiler.hpp"

#include <cstring>

#include <shaderc/shaderc.h>

#if defined(_MSC_VER)
#pragma comment(lib, "shaderc_shared")
#endif

uint32_t vulkan::ShaderCompiler::compile(const char* name, const char* src, VkShaderStageFlags stages, Result& result)
{	
	size_t length = strlen(src);
	uint32_t modules = 0, index = 0;	
	if (stages & VK_SHADER_STAGE_VERTEX_BIT)
	{
		BreakIfNot(result.num >= (++modules));
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_add_macro_definition(options, "VERTEX_SHADER", 13, "1", 1);
		shaderc_compilation_result_t res = shaderc_compile_into_spv(m_compiler, src, length, shaderc_vertex_shader, name, "main", options);
		createBinary(res, VK_SHADER_STAGE_VERTEX_BIT, result[index++]);
		shaderc_compile_options_release(options);
	}
	if (stages & VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		BreakIfNot(result.num >= (++modules));
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_add_macro_definition(options, "FRAGMENT_SHADER", 15, "1", 1);
		shaderc_compilation_result_t res = shaderc_compile_into_spv(m_compiler, src, length, shaderc_fragment_shader, name, "main", options);
		createBinary(res, VK_SHADER_STAGE_FRAGMENT_BIT, result[index++]);
		shaderc_compile_options_release(options);
	}
	if (stages & VK_SHADER_STAGE_COMPUTE_BIT)
	{
		BreakIfNot(result.num >= (++modules));
		shaderc_compile_options_t options = shaderc_compile_options_initialize();
		shaderc_compile_options_add_macro_definition(options, "COMPUTE_SHADER", 14, "1", 1);
		shaderc_compilation_result_t res = shaderc_compile_into_spv(m_compiler, src, length, shaderc_compute_shader, name, "main", options);
		createBinary(res, VK_SHADER_STAGE_COMPUTE_BIT, result[index++]);
		shaderc_compile_options_release(options);
	}
	return modules;
}

void vulkan::ShaderCompiler::initialize(const Config& conf)
{
	m_compiler = shaderc_compiler_initialize();
	BreakIfNot(m_compiler);
	m_device = conf.device;
	m_alloc = conf.alloc;
}

void vulkan::ShaderCompiler::finalize()
{
	shaderc_compiler_release(m_compiler);
}

void vulkan::ShaderCompiler::createBinary(shaderc_compilation_result* cr, VkShaderStageFlags stage, Binary& binary)
{
	const uint32_t* words = (const uint32_t*)shaderc_result_get_bytes(cr);
	VkShaderModuleCreateInfo smci
	{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, shaderc_result_get_length(cr), words
	};
	BreakIfFailed(vkCreateShaderModule(m_device, &smci, m_alloc, &binary.spirv));
	shaderc_result_release(cr);
	binary.entryPoint = "main";
	binary.stage = stage;
}
