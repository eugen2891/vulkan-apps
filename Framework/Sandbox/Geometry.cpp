#include "Geometry.hpp"

namespace sandbox
{

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
};

static const Vertex VertexCache[]
{

/* QUAD */

	{ { -1, -1, 0 }, { 0, 0, 1 } },
	{ {  1, -1, 0 }, { 0, 0, 1 } },
	{ {  1,  1, 0 }, { 0, 0, 1 } },
	{ { -1,  1, 0 }, { 0, 0, 1 } },

/* CUBE */

	/* Front face */
	{ { -1, -1,  1 }, {  0,  0,  1 } },
	{ {  1, -1,  1 }, {  0,  0,  1 } },
	{ {  1,  1,  1 }, {  0,  0,  1 } },
	{ { -1,  1,  1 }, {  0,  0,  1 } },

	/* Back face */
	{ { -1,  1, -1 }, {  0,  0, -1 } },
	{ {  1,  1, -1 }, {  0,  0, -1 } },
	{ {  1, -1, -1 }, {  0,  0, -1 } },
	{ { -1, -1, -1 }, {  0,  0, -1 } },

	/* Left face */
	{ { -1, -1, -1 }, { -1,  0,  0 } },
	{ { -1, -1,  1 }, { -1,  0,  0 } },
	{ { -1,  1,  1 }, { -1,  0,  0 } },
	{ { -1,  1, -1 }, { -1,  0,  0 } },

	/* Right face */
	{ {  1, -1,  1 }, {  1,  0,  0 } },
	{ {  1, -1, -1 }, {  1,  0,  0 } },
	{ {  1,  1, -1 }, {  1,  0,  0 } },
	{ {  1,  1,  1 }, {  1,  0,  0 } },

	/* Bottom face */
	{ {  1, -1,  1 }, {  0, -1,  0 } },
	{ { -1, -1,  1 }, {  0, -1,  0 } },
	{ { -1, -1, -1 }, {  0, -1,  0 } },
	{ {  1, -1, -1 }, {  0, -1,  0 } },

	/* Top face */
	{ { -1,  1,  1 }, {  0,  1,  0 } },
	{ {  1,  1,  1 }, {  0,  1,  0 } },
	{ {  1,  1, -1 }, {  0,  1,  0 } },
	{ { -1,  1, -1 }, {  0,  1,  0 } },

};

static const uint32_t IndexCache[]
{

/* QUAD */

	0, 1, 2, 0, 2, 3,

/* CUBE */

	/* Front face */
	0,  1,  2,  0,  2,  3,

	/* Back face */
	4,  5,  6,  4,  6,  7,

	/* Left face */
	8,  9, 10,  8, 10, 11,

	/* Right face */
	12, 13, 14, 12, 14, 15,

	/* Bottom face */
	16, 17, 18, 16, 18, 19,

	/* Top face */
	20, 21, 22, 20, 22, 23

};

Drawable GetMesh(uint32_t index)
{
	Drawable retval{};
	switch (index % eMesh_EnumMax)
	{
	case eMesh_Quad:
		retval.indexCount = 6;
		break;
	case eMesh_Cube:
		retval.firstIndex = 6;
		retval.indexCount = 36;
		retval.vertexOffset = 4;
		break;
	}
	return retval;
}

Range<const uint8_t> GetVertexData()
{
	const uint8_t* tmp = reinterpret_cast<const uint8_t*>(VertexCache);
	return Range{ tmp, sizeof(VertexCache) };
}

Range<const uint8_t> GetIndexData()
{
	const uint8_t* tmp = reinterpret_cast<const uint8_t*>(IndexCache);
	return Range{ tmp, sizeof(IndexCache) };
}

uint32_t GetVertexStride()
{
	return uint32_t(sizeof(Vertex));
}

}
