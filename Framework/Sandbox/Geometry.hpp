#pragma once

#include <cstdint>

#include "../Utilities/Array.hpp"

namespace sandbox
{

struct Drawable
{
	size_t dataOffset;
	uint32_t indexCount;
	uint32_t firstIndex;
	int32_t vertexOffset;
	uint32_t vertexCount;
};

enum eMesh : uint32_t
{
	eMesh_Quad,
	eMesh_Cube,
	eMesh_EnumMax
};

Drawable GetMesh(uint32_t index);

Range<const uint8_t> GetVertexData();

Range<const uint8_t> GetIndexData();

uint32_t GetVertexStride();

}
