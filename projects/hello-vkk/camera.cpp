#include "camera.hpp"

#include <vulkan-kit/vkk.h>

#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(float radius, float aspectRatio)
	: m_radius(radius)
	, m_elevation(0.f)
	, m_azimuth(0.f)
	, m_angleStep(0.005f)
	, m_aspectRatio(aspectRatio)
{
}

void Camera::applyZoom(float amount)
{
	m_radius += amount * 0.1f;
}

void Camera::applyRotation(float horz, float vert)
{
	m_azimuth += horz * m_angleStep;
	m_elevation += vert * m_angleStep;
	constexpr float eLimit = glm::half_pi<float>();
	m_elevation = glm::clamp(m_elevation, -eLimit, eLimit);
}

void Camera::applyResize(int width, int height)
{
	m_aspectRatio = float(width) / float(height);
}

void Camera::writeUniforms(Buffer buffer)
{
	const float sinA = glm::sin(m_azimuth);
	const float cosA = glm::cos(m_azimuth);
	const float sinE = glm::sin(m_elevation);
	const float cosE = glm::cos(m_elevation);
	const glm::vec3 eye{ m_radius * sinA * cosE, m_radius * sinE, m_radius * cosA * cosE };
	glm::mat4 proj = glm::perspective(glm::radians(60.f), m_aspectRatio, 0.001f, 1000.f);
	glm::mat4 viewProj = proj * glm::lookAt(eye, {}, { 0.f, 1.f, 0.f });
	memcpy(getBufferMappedPtr(buffer), &viewProj, sizeof(viewProj));

}
