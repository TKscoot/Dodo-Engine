#include "dodopch.h"
#include "Camera.h"


Dodo::Entity::CCamera::CCamera()
{
	m_transform = AddComponent<Components::CTransform>();
	m_transform->setPosition(Math::Vector3f(0.0f, 0.0f, -10.0f));
}


Dodo::Entity::CCamera::~CCamera()
{
}

void Dodo::Entity::CCamera::Update()
{
	m_updated = false;
	m_mousePos = Environment::CInput::GetMousePosition();

	glm::vec3 camFront;
	camFront.x = -cos(glm::radians(m_transform->getRotation().x)) * sin(glm::radians(m_transform->getRotation().y));
	camFront.y =  sin(glm::radians(m_transform->getRotation().x));
	camFront.z =  cos(glm::radians(m_transform->getRotation().x)) * cos(glm::radians(m_transform->getRotation().y));
	camFront = glm::normalize(camFront);

	UpdateViewMatrix();
}

void Dodo::Entity::CCamera::UpdateViewMatrix()
{

	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_A))
	{
		m_transform->setPositionX(m_transform->getPosition().x + 0.01);
	}
	
	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_D))
	{
		m_transform->setPositionX(m_transform->getPosition().x - 0.01);
	}
	
	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_E))
	{
		m_transform->setPositionY(m_transform->getPosition().y + 0.01);
	}
	
	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_Q))
	{
		m_transform->setPositionY(m_transform->getPosition().y - 0.01);
	}
	
	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_W))
	{
		m_transform->setPositionZ(m_transform->getPosition().z + 0.01);
	}
	
	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_S))
	{
		m_transform->setPositionZ(m_transform->getPosition().z - 0.01);
	}

	//HandleMouseMove();
	
	glm::mat4 rotM = glm::mat4(1.0f);
	glm::mat4 transM;

	rotM = glm::rotate(rotM, glm::radians(m_transform->getRotation().x), glm::vec3(1.0f, 0.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_transform->getRotation().y), glm::vec3(0.0f, 1.0f, 0.0f));
	rotM = glm::rotate(rotM, glm::radians(m_transform->getRotation().z), glm::vec3(0.0f, 0.0f, 1.0f));

	transM = glm::translate(glm::mat4(1.0f), m_transform->getPosition());

	m_viewMatrix = rotM * transM;

	m_updated = true;
}

void Dodo::Entity::CCamera::HandleMouseMove()
{
	Math::Vector2f mousePos = Environment::CInput::GetMousePosition();

	int32_t dx = (int32_t)m_mousePos.x - mousePos.x;
	int32_t dy = mousePos.y - (int32_t)m_mousePos.y;

	m_mousePos = mousePos;
	Math::Vector3f rotation = m_transform->getRotation();

	rotation.x += dy * 1.25f;
	rotation.y -= dx * 1.25f;

	m_transform->setRotation(rotation);

	m_transform->rotate(Math::Vector3f(dy, -dx, 0.0f));
}
