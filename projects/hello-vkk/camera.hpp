#pragma once

typedef struct BufferT* Buffer;

class Camera
{

public:
	Camera(float radius, float aspectRatio);
	void applyZoom(float amount);
	void applyRotation(float horz, float vert);
	void applyResize(int width, int height);
	void writeUniforms(Buffer buffer);

private:
	float m_radius;
	float m_elevation;
	float m_azimuth;
	float m_angleStep;
	float m_aspectRatio;

};
