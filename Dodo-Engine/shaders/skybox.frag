#version 450

layout (binding = 1) uniform samplerCube samplerCubeMap;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec3 inVertPos;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inUVW;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = texture(samplerCubeMap, inUVW.xyz);
}