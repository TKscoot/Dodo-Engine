#include "dodopch.h"
#include "Input.h"

bool Dodo::Environment::CInput::isKeyPressed = false;
Dodo::Environment::KeyCode Dodo::Environment::CInput::lastPressedKey = Dodo::Environment::KeyCode::KEY_UNKNOWN;
bool Dodo::Environment::CInput::isMouseButtonPressed = false;
Dodo::Environment::MouseKeyCode Dodo::Environment::CInput::lastPressedMouseButton = (MouseKeyCode)Dodo::Environment::KeyCode::KEY_UNKNOWN;
std::shared_ptr<Dodo::Environment::CWindow> Dodo::Environment::CInput::m_pWindow = {};

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
	}
	else if (action == GLFW_RELEASE)
	{
		isKeyPressed = false;
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
