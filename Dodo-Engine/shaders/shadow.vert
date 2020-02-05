#version 450

layout( location = 0 ) in vec4 app_position;

//layout( set = 0, binding = 0 ) uniform UniformBuffer {
//  mat4 ShadowModelViewMatrix;
//  mat4 SceneModelViewMatrix;
//  mat4 ProjectionMatrix;
//};

layout( set = 0, binding = 0 ) uniform UniformBuffer {
mat4 depthMVP;
};

void main() {
	//mat4 modelview = ShadowModelViewMatrix * SceneModelViewMatrix * ProjectionMatrix;
	//gl_Position = ProjectionMatrix * ShadowModelViewMatrix * vec4(app_position.xyz, 1.0f);
	gl_Position = depthMVP * vec4(app_position.xyz, 1.0f);
}
