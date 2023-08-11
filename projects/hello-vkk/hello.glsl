#version 460

#ifdef VERTEX

layout(location=0) in vec3 aPosition;

layout(location=0) out vec3 vTexCoord;

layout(binding=uniform_buffer_0) uniform Camera
{
	mat4 uViewProj;
};

void main()
{
	float u = atan(aPosition.x, aPosition.z) / (2 * 3.141592653589793) + 0.5;
	vTexCoord = vec3(u, fract(u + 0.5) - 0.5, -aPosition.y * 0.5 + 0.5);
	gl_Position = uViewProj * vec4(aPosition, 1.);
	gl_Position.y = -gl_Position.y;
}

#endif

#ifdef FRAGMENT

layout(location=0) in vec3 vTexCoord;

layout(location=0) out vec4 oColor;

layout(binding=sampler_state_0) uniform sampler uSampler;
layout(binding=sampled_image_0) uniform texture2D uTexture;

vec2 correctUV()
{
	float dU0 = fwidth(vTexCoord.x);
	float dU1 = fwidth(vTexCoord.y);
	float u = (dU0 > dU1) ? vTexCoord.y : vTexCoord.x;
	return vec2(u, vTexCoord.z);
}

void main()
{
	oColor = texture(sampler2D(uTexture, uSampler), correctUV());
}

#endif