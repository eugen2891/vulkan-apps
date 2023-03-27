#pragma once

#include "LuaState.hpp"
#include "DataDefs.hpp"
#include "Geometry.hpp"

#include "../Utilities/Array.hpp"

namespace sandbox
{

class Scene
{
public:
	void initialize();
	void finalize();
	void updateProjection(float aspectRatio);
	ArrayRef<const Drawable> drawables() const;
	ArrayRef<const uint8_t> perFrameData() const;
	ArrayRef<const uint8_t> perObjectData() const;
	explicit Scene(const char* fileName = nullptr);
	Scene& mesh(const glm::vec4& albedoColor, uint32_t meshId);
	Scene& directionalLight(const glm::vec3& direction, const glm::vec4& color);
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
	Array<PerObjectData> m_objectData;
	Array<Drawable> m_drawCalls;
	float m_vertFov = 0.f;
	LuaState m_lua;
	Stack m_stack;
};

}
