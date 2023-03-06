#pragma once

#include "LuaState.hpp"
#include "DataDefs.hpp"
#include "Geometry.hpp"

#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>

#include "../Utilities/Array.hpp"

namespace sandbox
{

class Scene
{
public:
	struct DrawCommand
	{
		uint32_t fromVertex;
		uint32_t fromIndex;
		uint32_t numIndices;
		uint32_t numInstances;
	};
	explicit Scene(const char* fileName = nullptr);
	void initialize();
	void finalize();
	size_t sceneDataSize() const;
	void writeSceneData(void* buffer) const;
	void updateProjection(float aspectRatio);
	Scene& mesh(const glm::vec4& albedoColor, uint32_t meshId);
	Scene& pointLight(const glm::vec3& position, const glm::vec4& color, const glm::vec3& attenuation);
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
	PerFrameData m_sceneData;
	DrawCommand m_drawCalls[eMesh_EnumMax]{};
	float m_vertFov = 0.f;
	LuaState m_lua;
	Stack m_stack;
};

}
