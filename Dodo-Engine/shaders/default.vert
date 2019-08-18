#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexcoords;
layout(location = 4) in vec3 inColor;

layout(location = 0) out vec3 fragNormalInterp;
layout(location = 1) out vec3 fragVertPos;
layout(location = 2) out vec2 fragTexCoord;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    //fragColor = inColor;
	//fragTexCoord = inTexcoords;
	mat4 modelView = ubo.view * ubo.model;

	vec4 vertPos4 = ubo.view * ubo.model * vec4(inPosition, 1.0);
    fragVertPos = vec3(vertPos4) / vertPos4.w;
	mat4 normalMat = transpose(inverse(modelView));
    fragNormalInterp = vec3(normalMat * vec4(inNormal, 0.0));
	fragTexCoord = inTexcoords;
}