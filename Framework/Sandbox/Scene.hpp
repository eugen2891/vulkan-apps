#pragma once

#include "LuaState.hpp"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

namespace sandbox
{

class Scene
{
public:
	struct MeshInstance
	{
		glm::mat4 transform;
	};
	struct MaterialData
	{
		glm::vec4 albedoColor;
	};
	explicit Scene(const char* fileName = nullptr);
	void initialize();
	void finalize();
	glm::mat4 cameraMatrix(float aspectRatio) const;
	Scene& perspective(const glm::vec3& from, const glm::vec3& to, const glm::vec3& up, float fovY);
	Scene& rotate(float angle, const glm::vec3& axis);
	Scene& translate(const glm::vec3& offs);
	Scene& scale(const glm::vec3& ratios);
	Scene& pushTransform();
	Scene& popTransform();
private:
	struct Stack
	{
		static const size_t BlockSize = 8;
		alignas(16) struct Block
		{
			glm::mat4 items[BlockSize];
			Block* prev = nullptr;
			Block* next = nullptr;			
			size_t count = 0;
		} first, *last = &first;
		explicit Stack();
		glm::mat4& top();
		void push();
		void pop();
		~Stack();
	};
	glm::mat4 m_viewMatrix;
	float m_vertFov = 0.f;
	LuaState m_lua;
	Stack m_stack;
};

}
