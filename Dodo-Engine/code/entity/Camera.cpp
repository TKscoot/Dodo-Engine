#include "dodopch.h"
#include "Camera.h"


Dodo::Entity::CCamera::CCamera()
{
	m_transform = AddComponent<Components::CTransform>();
	m_transform->setPosition(Math::Vector3f(0.0f, 0.0f, 0.0f));
}


Dodo::Entity::CCamera::~CCamera()
{
}

void Dodo::Entity::CCamera::Update()
{
	m_updated = false;

	glm::vec3 camFront;
	camFront.x = -cos(glm::radians(m_transform->getRotation().x)) * sin(glm::radians(m_transform->getRotation().y));
	camFront.y =  sin(glm::radians(m_transform->getRotation().x));
	camFront.z =  cos(glm::radians(m_transform->getRotation().x)) * cos(glm::radians(m_transform->getRotation().y));
	camFront = glm::normalize(camFront);
	


	//m_transform->SetLocalPosition(camFront);

	UpdateViewMatrix();
};