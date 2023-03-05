#include "Scene.hpp"

#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace sandbox
{

static const uint32_t kSceneMaxObjects = 1024;

static void setMeshHACK(PerObjectData& obj, uint32_t mesh)
{
	reinterpret_cast<uint32_t&>(obj.instance.transform[3][3]) = mesh;
}

static uint32_t getMeshHACK(const PerObjectData& obj)
{
	return reinterpret_cast<const uint32_t&>(obj.instance.transform[3][3]);
}

static void clearMeshHACK(PerObjectData& obj)
{
	obj.instance.transform[3][3] = 1.f;
}

Scene::Scene(const char* fileName) : m_lua{ *this, fileName }
{
	m_sceneData.viewport.transform = glm::identity<glm::mat4>();
	m_sceneData.viewport.projection = glm::identity<glm::mat4>();
	m_sceneData.objects = Array<PerObjectData>::New(kSceneMaxObjects);
	m_sceneData.objects.num = 0;
}

void Scene::initialize()
{
	m_lua.initialize();
	std::sort(m_sceneData.objects.begin(), m_sceneData.objects.end(), [this](const PerObjectData& a, const PerObjectData& b) { return getMeshHACK(a) < getMeshHACK(b); });
	for (auto& object : m_sceneData.objects) clearMeshHACK(object);
}

void Scene::finalize()
{
	m_lua.finalize();
}

void Scene::updateProjection(float aspectRatio)
{
	m_sceneData.viewport.projection = glm::perspective(m_vertFov, aspectRatio, 0.01f, 100.f);
}

Scene& Scene::quadMesh(const glm::vec4& albedoColor)
{
	if (m_sceneData.objects.num < kSceneMaxObjects)
	{
		auto index = m_sceneData.objects.num++;
		PerObjectData& objectData = m_sceneData.objects[index];
		objectData.instance.transform = m_stack.top();
		objectData.material.albedoColor = albedoColor;
		++m_drawCalls[eMesh_Quad].numInstances;
		setMeshHACK(objectData, eMesh_Quad);
	}
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
