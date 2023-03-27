#version 450 core

#extension GL_EXT_buffer_reference : require

#if VERTEX_SHADER
#define intermediate out
#elif FRAGMENT_SHADER
#define intermediate in
#endif

#include "Sandbox/DataDefs.hpp"

layout(push_constant) uniform Params
{
	PerFrameData frm;
	PerObjectData obj;
};

layout(location=0) intermediate vec3 Normal;
layout(location=1) intermediate vec3 LightVec;

#if VERTEX_SHADER

layout(location=0) in vec3 InPos;
layout(location=1) in vec3 InNorm;

void main()
{
	vec4 worldPos = obj.instance.transform * vec4(InPos, 1.0);
	gl_Position = frm.viewport.projection * frm.viewport.transform * worldPos;
	Normal = normalize((obj.instance.normalMatrix * vec4(InNorm, 0.0)).xyz);
	LightVec = frm.lighting.pointLight.position - worldPos.xyz;
}

#endif

#if FRAGMENT_SHADER

layout(location = 0) out vec4 OutColor;

vec4 GetDirectionalLight(DirectionalLight light)
{
	float diffuseRatio = max(0.0, dot(Normal, light.direction));
	return vec4(light.color.rgb * diffuseRatio, 1.0);
}

vec4 GetPointLight(PointLight light)
{
	float distance = length(LightVec);
	float diffuseRatio = max(0.0, dot(Normal, normalize(LightVec)));
	vec3 attenuation = light.attenuation * vec3(1.0, distance, distance * distance);
	return vec4((light.color / (attenuation.x + attenuation.y + attenuation.z)).rgb * diffuseRatio, 1.0);
}

void main()
{
	vec4 albedoColor = vec4(0.318309886 * obj.material.albedoColor.rgb, 1.0);
	vec4 lightColor = GetPointLight(frm.lighting.pointLight) + GetDirectionalLight(frm.lighting.directionalLight);
	OutColor = lightColor * albedoColor;
}

#endif
