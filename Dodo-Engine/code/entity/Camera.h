#pragma once
#include "dodopch.h"
#include "Entity.h"
#include "components/ECS.h"
#include "components/Transform.h"
#include "environment/Input.h"
#include "environment/Log.h"
#include "renderer/GUI.h"


namespace Dodo
{
	namespace Entity
	{
		class CCamera : public CEntity
		{
		public:
			CCamera();
			~CCamera();

			void Update();
			void HandleKeyInput(float cameraSpeed);

			void setPerspective(float fov, float aspect, float znear, float zfar)
			{
				this->m_fov   = fov;
				this->m_znear = znear;
				this->m_zfar  = zfar;
				m_projectionMatrix = glm::perspective(glm::radians(fov), aspect, znear, zfar);
			};

			const Math::Matrix4x4& getViewMatrix() const
			{
				return m_viewMatrix;
			}

			const Math::Matrix4x4& getProjectionMatrix() const
			{
				return m_projectionMatrix;
			}


			Math::Vector3f cameraPos = Math::Vector3f(0.0f, 0.0f, 3.0f);
			Math::Vector3f cameraFront = Math::Vector3f(0.0f, 0.0f, -1.0f);
			Math::Vector3f cameraUp = Math::Vector3f(0.0f, 1.0f, 0.0f);

			float* camSpeed;

		private:

			void UpdateViewMatrix();

			void HandleMouseMove();



			float m_fov;
			float m_znear;
			float m_zfar;

			bool m_updated = false;

			Math::Matrix4x4 m_viewMatrix;
			Math::Matrix4x4 m_projectionMatrix;

			//std::shared_ptr<Components::CTransform> m_transform;
			Math::Vector2f m_lastMousePos;
			bool firstMouse = true;
			float pitch = 0.0f;
			float yaw = 0.0f;
		};

	}
}

