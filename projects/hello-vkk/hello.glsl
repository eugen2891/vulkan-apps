#version 460

#ifdef VERTEX

layout(location=0) in vec3 aPosition;

layout(location=0) out vec3 vNormal;

layout(binding=uniform_buffer_0) uniform Camera
{
	mat4 uViewProj;
};

void main()
{
	vNormal = aPosition;
	gl_Position = uViewProj * vec4(aPosition, 1.);
	gl_Position.y = -gl_Position.y;
}

#endif

#ifdef FRAGMENT

layout(location=0) in vec3 vNormal;

layout(location=0) out vec4 oColor;

void main()
{
	oColor = vec4(vNormal, 1.);
}

#endif