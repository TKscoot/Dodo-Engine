#pragma once
#include "dodopch.h"
#include "Entity.h"
#include "components/ECS.h"
#include "components/Transform.h"


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

			void UpdateViewMatrix()
			{
				m_transform->setPosition(Math::Vector3f(20.0f, 0.0f, 4.0f));
				m_transform->setRotationY(5.0f);
				glm::mat4 rotM = glm::mat4(1.0f);
				glm::mat4 transM;

				rotM = glm::rotate(rotM, glm::radians(m_transform->getRotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
				rotM = glm::rotate(rotM, glm::radians(m_transform->getRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
				rotM = glm::rotate(rotM, glm::radians(m_transform->getRotation().z), glm::vec3(0.0f, 0.0f, 1.0f));

				transM = glm::translate(glm::mat4(1.0f), m_transform->getPosition());

				
				//m_viewMatrix = /* rotM * */ transM;
				//m_viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
				m_viewMatrix = glm::lookAt(m_transform->getPosition(), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)) * rotM;

				m_updated = true;
			};


			float m_fov;
			float m_znear;
			float m_zfar;

			bool m_updated = false;

			Math::Matrix4x4 m_viewMatrix;
			Math::Matrix4x4 m_projectionMatrix;

			std::shared_ptr<Components::CTransform> m_transform;
			//Components::CTransform* m_transform;
		};

	}
}

