#include "Scene.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace sandbox
{

Scene::Scene(const char* fileName) : m_lua{ *this, fileName }
{
	m_perFrame.viewport.transform = glm::identity<glm::mat4>();
	m_perFrame.viewport.projection = glm::identity<glm::mat4>();
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
	m_perFrame.viewport.projection = glm::perspective(m_vertFov, aspectRatio, 0.01f, 100.f);
}

Scene& Scene::perspective(const glm::vec3& from, const glm::vec3& to, const glm::vec3& up, float fovY)
{	
	m_vertFov = glm::radians(fovY);
	m_perFrame.viewport.transform = glm::lookAt(from, to, up);
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
