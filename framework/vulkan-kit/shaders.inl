#pragma once

#include <sys/stat.h>
#include <shaderc/shaderc.h>
#include <spirv_cross/spirv_cross_c.h>

static shaderc_compile_options_t createCompilerOptions(VkShaderStageFlags stage)
{
	shaderc_compile_options_t retval = shaderc_compile_options_initialize();
	for (uint32_t i = 0; i < NumShaderMacros; i++)
	{
		const struct ShaderMacro* sm = ShaderMacros + i;
		shaderc_compile_options_add_macro_definition(retval, sm->name, sm->nameLength, sm->val, sm->valLength);
	}
	switch (stage)
	{
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		shaderc_compile_options_add_macro_definition(retval, "FRAGMENT", 8, NULL, 0);
		break;
	case VK_SHADER_STAGE_VERTEX_BIT:
		shaderc_compile_options_add_macro_definition(retval, "VERTEX", 6, NULL, 0);
		break;
	case VK_SHADER_STAGE_COMPUTE_BIT:
		shaderc_compile_options_add_macro_definition(retval, "COMPUTE", 7, NULL, 0);
		break;
	default:
		breakIfNot(0);
	}
	return retval;
}

static char* loadFromFile(const char* fileName, size_t* size)
{
	char* retval = NULL;
	FILE* file = fopen(fileName, "rb");
	breakIfNot(file);
	if (file)
	{
		struct stat fileStat = { 0 };
		if (fstat(_fileno(file), &fileStat) == 0)
		{
			size_t bytes = fileStat.st_size;
			retval = malloc(bytes);
			if (retval)
			{
				size_t bytesRead = fread(retval, sizeof(char), bytes, file);
				if (bytesRead != bytes)
				{
					freeMem(retval);
				}
				*size = bytes;
			}
		}
		fclose(file);
	}
	return retval;
}

static shaderc_compilation_result_t compileShaderSource(shaderc_compiler_t compiler, const char* fileName, VkShaderStageFlags stage)
{
	size_t srcBytes = 0;
	shaderc_shader_kind kind = 0;
	char* src = loadFromFile(fileName, &srcBytes);
	switch (stage)
	{
	case VK_SHADER_STAGE_FRAGMENT_BIT:
		kind = shaderc_fragment_shader;
		break;
	case VK_SHADER_STAGE_VERTEX_BIT:
		kind = shaderc_vertex_shader;
		break;
	case VK_SHADER_STAGE_COMPUTE_BIT:
		kind = shaderc_compute_shader;
		break;
	}
	shaderc_compile_options_t options = createCompilerOptions(stage);
	shaderc_compilation_result_t res = shaderc_compile_into_spv(compiler, src, srcBytes, kind, fileName, kShaderMain, options);
	shaderc_compilation_status sts = shaderc_result_get_compilation_status(res);
	shaderc_compile_options_release(options);
	if (sts != shaderc_compilation_status_success)
	{
		debugPrint("%s\n", shaderc_result_get_error_message(res));
		breakIfNot(0);
	}
	freeMem(src);
	return res;
}

static VkShaderModule makeShaderModule(const uint32_t* code, size_t size)
{
	VkShaderModuleCreateInfo smci = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = size,
		.pCode = code
	};
	VkShaderModule retval = VK_NULL_HANDLE;
	breakIfFailed(vkCreateShaderModule(Device, &smci, Alloc, &retval));
	return retval;
}

static void getVertexAttributes(const uint32_t* code, size_t numWords, VkVertexInputAttributeDescription** attrs, uint32_t* count, uint32_t* stride)
{
	spvc_context context = NULL;
	spvc_compiler compiler = NULL;
	spvc_parsed_ir parsedIr = NULL;
	spvc_resources resources = NULL;

	breakIfNot(spvc_context_create(&context) == SPVC_SUCCESS);
	breakIfNot(spvc_context_parse_spirv(context, code, numWords, &parsedIr) == SPVC_SUCCESS);
	breakIfNot(spvc_context_create_compiler(context, SPVC_BACKEND_NONE, parsedIr, SPVC_CAPTURE_MODE_TAKE_OWNERSHIP, &compiler) == SPVC_SUCCESS);
	breakIfNot(spvc_compiler_create_shader_resources(compiler, &resources) == SPVC_SUCCESS);

	size_t numStageInputs = 0;
	const spvc_reflected_resource* stageInputs = NULL;
	breakIfNot(spvc_resources_get_resource_list_for_type(resources, SPVC_RESOURCE_TYPE_STAGE_INPUT, &stageInputs, &numStageInputs) == SPVC_SUCCESS);

	uint32_t numItems = 0, byteStride = 0;
	VkVertexInputAttributeDescription* items = calloc(numStageInputs, sizeof(VkVertexInputAttributeDescription));
	if (!items)
	{
		spvc_context_destroy(context);
		return;
	}

	for (size_t i = 0; i < numStageInputs; i++)
	{
		const uint32_t location = spvc_compiler_get_decoration(compiler, stageInputs[i].id, SpvDecorationLocation);
		const spvc_type valueType = spvc_compiler_get_type_handle(compiler, stageInputs[i].type_id);
		const spvc_basetype baseType = spvc_type_get_basetype(valueType);
		const uint32_t vectorSize = spvc_type_get_vector_size(valueType);

		VkVertexInputAttributeDescription* desc = items + (numItems++);
		desc->location = location;
		desc->binding = 0;
		desc->offset = byteStride;
		switch (baseType)
		{
		case SPVC_BASETYPE_FP32:
			switch (vectorSize)
			{
			case 1:
				desc->format = VK_FORMAT_R32_SFLOAT;
				byteStride += 4;
				break;
			case 2:
				desc->format = VK_FORMAT_R32G32_SFLOAT;
				byteStride += 8;
				break;
			case 3:
				desc->format = VK_FORMAT_R32G32B32_SFLOAT;
				byteStride += 12;
				break;
			case 4:
				desc->format = VK_FORMAT_R32G32B32A32_SFLOAT;
				byteStride += 16;
				break;
			default:
				break;
			}
			break;
		default:
			breakIfNot(0);
			break;
		}
	}

	spvc_context_destroy(context);
	*stride = byteStride;
	*count = numItems;
	*attrs = items;
}

VkShaderModule compileShader(VkShaderStageFlags stage, const char* fileName, VkVertexInputAttributeDescription** attrs, uint32_t* count, uint32_t* stride)
{
	shaderc_compiler_t compiler = shaderc_compiler_initialize();
	shaderc_compilation_result_t res = compileShaderSource(compiler, fileName, stage);
	const uint32_t* code = (const uint32_t*)shaderc_result_get_bytes(res);
	VkShaderModule retval = makeShaderModule(code, shaderc_result_get_length(res));
	if (stage == VK_SHADER_STAGE_VERTEX_BIT)
	{
		getVertexAttributes(code, shaderc_result_get_length(res) >> 2, attrs, count, stride);
	}
	shaderc_compiler_release(compiler);
	return retval;
}
