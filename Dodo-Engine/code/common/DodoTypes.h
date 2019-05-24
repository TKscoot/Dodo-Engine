#pragma once
#include "VKIntegration.h"

typedef glm::vec3 Vector3f;
typedef glm::vec2 Vector2f;
typedef glm::mat4 Matrix4x4;


struct Vertex
{
	Vector3f position;
	Vector3f normal;
	Vector3f tangent;
	Vector2f texcoords;

	Vector3f color;
};
