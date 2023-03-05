#pragma once

namespace sandbox
{

enum eMesh : unsigned int
{
	eMesh_Quad,
	eMesh_Cube,
	eMesh_EnumMax
};

struct Mesh
{
	const void* vertices;
	const void* indices;
	size_t verticesSize;
	size_t indicesSize;
};

void Quad(Mesh& mesh);
void Cube(Mesh& mesh);
void Sphere(Mesh& mesh);

}
