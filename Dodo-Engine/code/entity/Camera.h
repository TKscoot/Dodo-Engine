#pragma once
#include "dodopch.h"
#include "Entity.h"
#include "components/ECS.h"
#include "components/Transform.h"
#include "environment/Input.h"


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


		private:

			void UpdateViewMatrix();

			void HandleMouseMove();



			float m_fov;
			float m_znear;
			float m_zfar;

			bool m_updated = false;

			Math::Matrix4x4 m_viewMatrix;
			Math::Matrix4x4 m_projectionMatrix;

			std::shared_ptr<Components::CTransform> m_transform;
			Math::Vector2f m_mousePos;
			//Components::CTransform* m_transform;
		};

	}
}

