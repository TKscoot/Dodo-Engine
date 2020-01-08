#version 450
//#extension GL_ARB_separate_shader_objects : enable
//#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	vec3 camPos;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexcoords;
layout(location = 4) in vec3 inColor;

layout(location = 0) out vec3 fragNormal;
layout(location = 1) out vec3 fragVertPos;
layout(location = 2) out vec2 fragTexCoord;
layout(location = 3) out vec3 fragUVW;
layout(location = 4) out vec3 fragCamPos;

layout(push_constant) uniform PushConsts {
	vec3 objPos;
} pushConsts;

void main()
{
	fragUVW = inPosition;
	fragCamPos = ubo.camPos;
	//fragUVW.y *= -1.0;
	fragVertPos=inPosition;
	gl_Position = ubo.proj * ubo.model * vec4(inPosition.xyz, 1.0);
}