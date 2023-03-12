#include "Scene.hpp"
#include "Geometry.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace sandbox
{

static const uint32_t kSceneMaxObjects = 1024;

Scene::Scene(const char* fileName) : m_lua{ *this, fileName }
{
	m_sceneData.viewport.transform = glm::identity<glm::mat4>();
	m_sceneData.viewport.projection = glm::identity<glm::mat4>();
	m_objectData = Array<PerObjectData>::New(kSceneMaxObjects);
	m_drawCalls = Array<Drawable>::New(kSceneMaxObjects);
	m_objectData.num = 0;
	m_drawCalls.num = 0;
}

void Scene::initialize()
{
	m_lua.initialize();
}

void Scene::finalize()
{
	m_lua.finalize();
}

void Scene::updateProjection(float aspectRatio)
{
	m_sceneData.viewport.projection = glm::infinitePerspective(m_vertFov, aspectRatio, 0.01f);	
	m_sceneData.viewport.projection[1][1] *= -1.f;
	m_sceneData.viewport.projection[3][2] *= 0.5f;
	m_sceneData.viewport.projection[2][2] = 0.f;
}

ArrayRef<const Drawable> Scene::drawables() const
{
	const Drawable* tmp = m_drawCalls.items;
	return ArrayRef<const Drawable>{ tmp, m_drawCalls.num };
}

ArrayRef<const uint8_t> Scene::perFrameData() const
{
	const uint8_t* tmp = reinterpret_cast<const uint8_t*>(&m_sceneData);
	return ArrayRef<const uint8_t>{ tmp, sizeof(m_sceneData) };
}

ArrayRef<const uint8_t> Scene::perObjectData() const
{
	const uint8_t* tmp = reinterpret_cast<const uint8_t*>(m_objectData.items);
	return ArrayRef<const uint8_t>{ tmp, sizeof(PerObjectData) * m_objectData.num };
}

Scene& Scene::mesh(const glm::vec4& albedoColor, uint32_t meshId)
{
	if (m_objectData.num < kSceneMaxObjects)
	{
		auto index = m_objectData.num++;
		BreakIfNot(index == m_drawCalls.num);		
		PerObjectData& objectData = m_objectData[index];
		objectData.instance.transform = m_stack.top();
		objectData.material.albedoColor = albedoColor;
		Drawable& cmd = m_drawCalls[m_drawCalls.num++];
		cmd = GetMesh(meshId);
		cmd.dataOffset = sizeof(PerObjectData) * index;
	}
	return *this;
}

Scene& Scene::pointLight(const glm::vec3& position, const glm::vec4& color, const glm::vec3& attenuation)
{
	m_sceneData.lighting.pointLight.position = position;
	m_sceneData.lighting.pointLight.color = color;
	m_sceneData.lighting.pointLight.attenuation = attenuation;
	return *this;
}

Scene& Scene::perspective(const glm::vec3& from, const glm::vec3& to, const glm::vec3& up, float fovY)
{	
	m_vertFov = glm::radians(fovY);
	m_sceneData.viewport.transform = glm::lookAt(from, to, up);
	return *this;
}

Scene& Scene::rotate(float angle, const glm::vec3& axis)
{
	glm::mat4& matrix = m_stack.top();
	matrix = glm::rotate(matrix, glm::radians(angle), axis);
	return *this;
}

Scene& Scene::translate(const glm::vec3& offs)
{
	glm::mat4& matrix = m_stack.top();
	matrix = glm::translate(matrix, offs);
	return *this;
}

Scene& Scene::scale(const glm::vec3& ratios)
{
	glm::mat4& matrix = m_stack.top();
	matrix = glm::scale(matrix, ratios);
	return *this;
}

sandbox::Scene& Scene::pushTransform()
{
	m_stack.push();
	return *this;
}

sandbox::Scene& Scene::popTransform()
{
	m_stack.pop();
	return *this;
}

Scene::Stack::Stack()
{
	first.count = 1;
	first.items[0] = glm::identity<glm::mat4>();
}

glm::mat4& Scene::Stack::top()
{
	return last->items[last->count - 1];
}

void Scene::Stack::push()
{
	if (last->count == BlockSize)
	{
		if (!last->next)
		{
			Block* newBlock = new Block;
			newBlock->prev = last;
			last->next = newBlock;			
		}
		last = last->next;
	}
	Block& block = *last;
	block.items[block.count] = block.items[block.count - 1];
	++block.count;
}

void Scene::Stack::pop()
{
	Block& block = *last;
	if (--block.count == 0)
	{
		Block* prevBlock = block.prev;
		if (prevBlock) last = prevBlock;		
	}
}

Scene::Stack::~Stack()
{
	while (last->prev)
	{
		Block* next = last->prev;
		delete last;
		last = next;
	}
}

}
