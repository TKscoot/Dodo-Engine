#include "dodopch.h"
#include "Input.h"

bool Dodo::Environment::CInput::isKeyPressed = false;
bool Dodo::Environment::CInput::isModPressed = false;
Dodo::Environment::KeyCode Dodo::Environment::CInput::lastPressedKey = Dodo::Environment::KeyCode::KEY_UNKNOWN;
Dodo::Environment::ModKeyCode Dodo::Environment::CInput::lastModPressed = Dodo::Environment::ModKeyCode::KEY_MOD_UNKNOWN;
bool Dodo::Environment::CInput::isMouseButtonPressed = false;
Dodo::Environment::MouseKeyCode Dodo::Environment::CInput::lastPressedMouseButton = (MouseKeyCode)Dodo::Environment::KeyCode::KEY_UNKNOWN;
std::shared_ptr<Dodo::Environment::CWindow> Dodo::Environment::CInput::m_pWindow = {};
Dodo::Math::Vector2f Dodo::Environment::CInput::m_lastMousePos = { 0.0f, 0.0f };
bool Dodo::Environment::CInput::toggleMouse = false;

bool Dodo::Environment::CInput::IsKeyPressed(KeyCode key)
{
	if (isKeyPressed)
	{
		if (lastPressedKey == key)
		{
			return true;
		}
	}
	else
	{
		return false;
	}

	// should never get here
	return false;
}

bool Dodo::Environment::CInput::IsModPressed(ModKeyCode key)
{
	if (isModPressed)
	{
		if (lastModPressed == key)
		{
			return true;
		}
	}
	else
	{
		return false;
	}

	// should never get here
	return false;
}

Dodo::Math::Vector2f Dodo::Environment::CInput::GetMousePosition()
{
	double xpos, ypos;
	glfwGetCursorPos(m_pWindow->GetWindow(), &xpos, &ypos);
	return Math::Vector2f(xpos, ypos);
}

bool Dodo::Environment::CInput::IsMouseKeyPressed(MouseKeyCode key)
{
	if (isMouseButtonPressed)
	{
		if (lastPressedMouseButton == key)
		{
			return true;
		}
	}
	else
	{
		return false;
	}

	// should never get here
	return false;
}

void Dodo::Environment::CInput::key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		lastPressedKey = (KeyCode)key;
		isKeyPressed = true;
		if (mods != 0x0000)
		{
			isModPressed = true;
			lastModPressed = (ModKeyCode)mods;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		isKeyPressed = false;
		isModPressed = false;
	}

	if (key == GLFW_KEY_M && action == GLFW_PRESS)
	{
		toggleMouse = !toggleMouse;
	}
	if (toggleMouse)
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	}
	else
	{
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	}
}

void Dodo::Environment::CInput::mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		lastPressedMouseButton = (MouseKeyCode)button;
		isMouseButtonPressed = true;
	}
	else if (action == GLFW_RELEASE)
	{
		isMouseButtonPressed = false;
	}
}

void Dodo::Environment::CInput::mouse_pos_callback(GLFWwindow * window, double xpos, double ypos)
{
	float xoffset = xpos - m_lastMousePos.x;
	float yoffset = m_lastMousePos.y - ypos; // reversed since y-coordinates range from bottom to top
	m_lastMousePos.x = xpos;
	m_lastMousePos.y = ypos;

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;
}
