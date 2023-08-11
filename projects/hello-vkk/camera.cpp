#include "camera.hpp"

#include <vulkan-kit/vkk.h>

#include <cglm/cglm.h>
#include <cstring>

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
	const float limit = GLM_PI_2f - 0.00001f;
	m_elevation = glm_clamp(m_elevation, -limit, limit);
}

void Camera::applyResize(int width, int height)
{
	m_aspectRatio = float(width) / float(height);
}

void Camera::writeUniforms(Buffer buffer)
{
	mat4 view, proj, viewProj;
	const float sinA = sinf(m_azimuth);
	const float cosA = cosf(m_azimuth);
	const float sinE = sinf(m_elevation);
	const float cosE = cosf(m_elevation);
	vec3 eye = { m_radius * sinA * cosE, m_radius * sinE, m_radius * cosA * cosE };
	glm_perspective(glm_rad(60.f), m_aspectRatio, 0.001f, 1000.f, proj);
	glm_lookat(eye, vec3{}, vec3{ 0.f, 1.f, 0.f }, view);
	glm_mul(proj, view, viewProj);
	memcpy(getBufferMappedPtr(buffer), &viewProj, sizeof(viewProj));
}
