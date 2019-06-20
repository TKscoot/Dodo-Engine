#pragma once
#include "dodopch.h"
#include "VKIntegration.h"

namespace Dodo
{
	namespace Math
	{
		typedef glm::vec3 Vector3f;
		typedef glm::vec2 Vector2f;
		typedef glm::mat4 Matrix4x4;

	}

	struct Vertex
	{
		Math::Vector3f position;
		Math::Vector3f normal;
		Math::Vector3f tangent;
		Math::Vector2f texcoords;

		Math::Vector3f color;
	};
}




