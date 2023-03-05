#include "Geometry.hpp"

#include <utility>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace sandbox
{

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
};

void Quad(Mesh& mesh)
{
	static const uint32_t indices[]
	{
		0, 1, 2, 0, 2, 3
	};
	static const Vertex vertices[]
	{
		{ { -1, -1, 0 }, { 0, 0, 1 } },
		{ {  1, -1, 0 }, { 0, 0, 1 } },
		{ {  1,  1, 0 }, { 0, 0, 1 } },
		{ { -1,  1, 0 }, { 0, 0, 1 } }
	};
	mesh.verticesSize = sizeof(vertices);
	mesh.indicesSize = sizeof(indices);
	mesh.vertices = vertices;
	mesh.indices = indices;
}

void Cube(Mesh& mesh)
{
	static const uint32_t indices[]
	{
		 0,  1,  2,  0,  2,  3,
		 4,  5,  6,  4,  6,  7,
		 8,  9, 10,  8, 10, 11,
		12, 13, 14, 12, 14, 15,
		16, 17, 18, 16, 18, 19,
		20, 21, 22, 20, 22, 23
	};
	static const Vertex vertices[]
	{
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
		{ { -1,  1, -1 }, {  0,  1,  0 } }
	};
	mesh.verticesSize = sizeof(vertices);
	mesh.indicesSize = sizeof(indices);
	mesh.vertices = vertices;
	mesh.indices = indices;
}

void Sphere(Mesh& mesh)
{
	const size_t maxSubdiv = 3;
	const size_t maxTris = 20 * (4 << (maxSubdiv + 1));
	const size_t maxVerts = 10 * (4 << (maxSubdiv + 1)) + 2;
	const size_t maxInds = maxTris * 3;

	static uint32_t indices[maxInds];
	static Vertex vertices[maxVerts];

	const glm::vec3 icosVerts[12] =
	{
		{  0.00000000f,      0.000000000f,  1.000000000f },
		{ -0.525731027f,    -0.723606825f,  0.447213590f },
		{  0.525731146f,    -0.723606765f,  0.447213590f },
		{  0.850650787f,     0.276393324f,  0.447213590f },
		{ -1.45720691e-07f,  0.894427180f,  0.447213590f },
		{ -0.850650847f,     0.276393026f,  0.447213590f },
		{ -3.90966548e-08f, -0.894427180f, -0.447213590f },
		{  0.850650847f,    -0.276393205f, -0.447213590f },
		{  0.525731087f,     0.723606825f, -0.447213590f },
		{ -0.525731027f,     0.723606825f, -0.447213590f },
		{ -0.850650847f,    -0.276393175f, -0.447213590f },
		{  0.00000000f,      0.000000000f, -1.000000000f }
	};

	glm::uvec3 icosFaces[20]{};
	for (uint32_t i = 0, v = 1; i < 5; i++, v++)
	{
		uint32_t base = i * 12, c = v + 5;
		uint32_t a = (v == 5) ? 1 : v + 1;
		uint32_t b = ((v == 1) ? 5 : v - 1) + 5;
		icosFaces[base + 0] = { 0, v,  a };
		icosFaces[base + 1] = { a, v,  c };
		icosFaces[base + 2] = { b, c,  v };
		icosFaces[base + 3] = { c, b, 11 };
	}

	mesh.verticesSize = sizeof(vertices);
	mesh.indicesSize = sizeof(indices);
	mesh.vertices = vertices;
	mesh.indices = indices;
}

}


