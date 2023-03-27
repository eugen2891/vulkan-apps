#include <GlobalPCH.hpp>

#include "ShaderCompiler.hpp"

#include <sys/stat.h>

#include <shaderc/shaderc.h>

#if defined(_MSC_VER)
#pragma comment(lib, "shaderc_shared")
#endif

namespace vulkan
{

void ShaderCompiler::releaseHeader(shaderc_include_result* header) const
{
	Source* includeSource = static_cast<Source*>(header->user_data);
	delete includeSource;
	delete header;
}

shaderc_include_result* ShaderCompiler::acquireHeader(const char* fileName) const
{
	strcpy_s(relStart, relMax, fileName);
	Source* includeSource = new Source(includePath);	
	shaderc_include_result* result = new shaderc_include_result();
	result->source_name = includeSource->name;
	result->source_name_length = strlen(includeSource->name);
	result->content = includeSource->data;
	result->content_length = includeSource->size;
	result->user_data = includeSource;
	return result;
}

ShaderCompiler::ShaderCompiler(const APIState& vk, const char* incDir) : APIClient(vk)
{
	strcpy_s(includePath, incDir);
	char* sep = strrchr(includePath, '/');
	if (!sep) sep = strrchr(includePath, '\\');
	BreakIfNot(sep && (*(sep + 1) == 0));
	relMax = sizeof(includePath) - size_t(sep - includePath) - 1;	
	relStart = ++sep;
}

uint32_t ShaderCompiler::compile(const Source& src, VkShaderStageFlags stages, Result& result)
{
	const size_t length = src.size;
	uint32_t modules = 0, index = 0;
	shaderc_compile_options_t sharedOptions = shaderc_compile_options_initialize();
	shaderc_compile_options_set_include_callbacks(sharedOptions,
		[](void* ctx, const char* header, int, const char*, size_t) { return static_cast<ShaderCompiler*>(ctx)->acquireHeader(header); },
		[](void* ctx, shaderc_include_result* result) { return static_cast<ShaderCompiler*>(ctx)->releaseHeader(result); }, this
	);
	if (stages & VK_SHADER_STAGE_VERTEX_BIT)
	{
		BreakIfNot(result.num >= (++modules));
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_clone(sharedOptions);
		shaderc_compile_options_add_macro_definition(options, "VERTEX_SHADER", 13, "1", 1);		
		shaderc_compilation_result_t res = shaderc_compile_into_spv(compiler, src.data, length, shaderc_vertex_shader, src.name, "main", options);
		createBinary(res, VK_SHADER_STAGE_VERTEX_BIT, result[index++]);
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);
	}
	if (stages & VK_SHADER_STAGE_FRAGMENT_BIT)
	{
		BreakIfNot(result.num >= (++modules));
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_clone(sharedOptions);
		shaderc_compile_options_add_macro_definition(options, "FRAGMENT_SHADER", 15, "1", 1);
		shaderc_compilation_result_t res = shaderc_compile_into_spv(compiler, src.data, length, shaderc_fragment_shader, src.name, "main", options);
		createBinary(res, VK_SHADER_STAGE_FRAGMENT_BIT, result[index++]);
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);
	}
	if (stages & VK_SHADER_STAGE_COMPUTE_BIT)
	{
		BreakIfNot(result.num >= (++modules));
		shaderc_compiler_t compiler = shaderc_compiler_initialize();
		shaderc_compile_options_t options = shaderc_compile_options_clone(sharedOptions);
		shaderc_compile_options_add_macro_definition(options, "COMPUTE_SHADER", 14, "1", 1);
		shaderc_compilation_result_t res = shaderc_compile_into_spv(compiler, src.data, length, shaderc_compute_shader, src.name, "main", options);
		createBinary(res, VK_SHADER_STAGE_COMPUTE_BIT, result[index++]);
		shaderc_compile_options_release(options);
		shaderc_compiler_release(compiler);
	}
	return modules;
}

ShaderCompiler::~ShaderCompiler()
{
}

void ShaderCompiler::createBinary(shaderc_compilation_result* cr, VkShaderStageFlags stage, Binary& binary)
{
	shaderc_compilation_status sts = shaderc_result_get_compilation_status(cr);
	if (sts != shaderc_compilation_status_success)
	{
		printf("%s\n", shaderc_result_get_error_message(cr));
		shaderc_result_release(cr);
		ReturnIfNot(0);
	}
	const uint32_t* words = reinterpret_cast<const uint32_t*>(shaderc_result_get_bytes(cr));
	VkShaderModuleCreateInfo smci
	{
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, nullptr, 0, shaderc_result_get_length(cr), words
	};
	BreakIfFailed(vkCreateShaderModule(m_vk.device(), &smci, m_vk.alloc(), &binary.spirv));
	shaderc_result_release(cr);
	binary.entryPoint = "main";
	binary.stage = stage;
}

ShaderCompiler::Source::Source(const char* fileName)
{
	struct stat fstats{};
	FILE* file = nullptr;
	name = _strdup(fileName);
	BreakIfNot(fopen_s(&file, fileName, "rb") == 0);
	if (file)
	{
		BreakIfNot(fstat(_fileno(file), &fstats) == 0);
		size = size_t(fstats.st_size);
		data = new char[size + 1];
		fread_s(data, size, 1, size, file);
		data[size] = 0;
		fclose(file);
	}
	else
	{
		data = nullptr;
		size = 0;
	}
}

ShaderCompiler::Source::~Source()
{
	delete[] data;
	free(name);
}

}
