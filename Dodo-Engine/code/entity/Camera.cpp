#include "dodopch.h"
#include "Camera.h"


Dodo::Entity::CCamera::CCamera()
{
	//m_transform = AddComponent<Components::CTransform>();
	//m_transform->setPosition(Math::Vector3f(0.0f, 0.0f, -10.0f));
	camSpeed = new float(0.1f);
}


Dodo::Entity::CCamera::~CCamera()
{
}

void Dodo::Entity::CCamera::Update()
{
	m_updated = false;

	//glm::vec3 camFront;
	//camFront.x = -cos(glm::radians(m_transform->getRotation().x)) * sin(glm::radians(m_transform->getRotation().y));
	//camFront.y =  sin(glm::radians(m_transform->getRotation().x));
	//camFront.z =  cos(glm::radians(m_transform->getRotation().x)) * cos(glm::radians(m_transform->getRotation().y));
	//camFront = glm::normalize(camFront);

	UpdateViewMatrix();
}

void Dodo::Entity::CCamera::HandleKeyInput(float cameraSpeed)
{
	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_A))
	{
		//m_transform->setPositionX(m_transform->getPosition().x + 0.01);
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_D))
	{
		//m_transform->setPositionX(m_transform->getPosition().x - 0.01);
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_E))
	{
		//m_transform->setPositionY(m_transform->getPosition().y + 0.01);
		cameraPos.y += cameraSpeed * 1.0f;
	}

	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_Q))
	{
		cameraPos.y -= cameraSpeed * 1.0f;

	}

	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_W))
	{
		cameraSpeed * 2;
		cameraPos += cameraSpeed * cameraFront;
	}

	if (Environment::CInput::IsKeyPressed(Environment::KeyCode::KEY_S))
	{
		cameraPos -= cameraSpeed * cameraFront;
	}
}

void Dodo::Entity::CCamera::UpdateViewMatrix()
{
	float cameraSpeed = *camSpeed;

	if (Environment::CInput::IsModPressed(Environment::ModKeyCode::KEY_MOD_SHIFT))
	{
		cameraSpeed *= 3;
	}
	else
	{
		cameraSpeed = *camSpeed;
	}

	HandleKeyInput(cameraSpeed);


	HandleMouseMove();

	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	m_viewMatrix = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);


	m_updated = true;
}

void Dodo::Entity::CCamera::HandleMouseMove()
{
	Math::Vector2f mousePos = Environment::CInput::GetMousePosition();

	if (firstMouse)
	{
		m_lastMousePos.x = mousePos.x;
		m_lastMousePos.y = mousePos.y;
		firstMouse = false;
	}

	float xoffset = mousePos.x - m_lastMousePos.x;
	float yoffset = m_lastMousePos.y - mousePos.y; // reversed since y-coordinates range from bottom to top
	m_lastMousePos.x = mousePos.x;
	m_lastMousePos.y = mousePos.y;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;


	yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
       pitch = 89.0f;
    if(pitch < -89.0f)
       pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}
