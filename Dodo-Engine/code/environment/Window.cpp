#include "dodopch.h"
#include "Window.h"
#include "renderer/Renderer.h"

namespace Dodo
{
	namespace Environment
	{
		DodoError Dodo::Environment::CWindow::Initialize()
		{
			int i = glfwInit();
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

			m_pMonitor = glfwGetPrimaryMonitor();

			if (!m_pMonitor)
			{
				return DodoError::DODO_NULL;
			}
			m_pVideoMode = glfwGetVideoMode(m_pMonitor);
			if (!m_pVideoMode)
			{
				return DodoError::DODO_NULL;
			}

			glfwWindowHint(GLFW_RED_BITS, m_pVideoMode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, m_pVideoMode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, m_pVideoMode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, m_pVideoMode->refreshRate);

			return DodoError::DODO_OK;
		}

		DodoError Dodo::Environment::CWindow::CreateGLFWWindow(const glm::vec2 _dimensions, const char * _title)
		{
			m_pWindow = glfwCreateWindow(_dimensions.x, _dimensions.y, _title, nullptr, nullptr);
			int count;
			GLFWmonitor** monitors = glfwGetMonitors(&count);
			

			HWND consoleWindow = GetConsoleWindow();
			SetWindowPos(consoleWindow, 0, -1920, 500, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

			if (count > 1)
			{
				int x, y;
				glfwGetMonitorPos(monitors[1], &x, &y);
				glfwSetWindowPos(m_pWindow, x + 625, y + 50);
				glfwFocusWindow(m_pWindow);
			}

			if (!m_pWindow)
			{
				return DodoError::DODO_NULL;
			}


			glfwSetFramebufferSizeCallback(m_pWindow, Dodo::Rendering::CRenderer::framebufferResizeCallback);

			return DodoError::DODO_OK;
		}

		DodoError CWindow::CreateWindowSurface(VkInstance _instance)
		{
			VkResult result = glfwCreateWindowSurface(_instance, m_pWindow, nullptr, &m_vkSurface);
			CError::CheckError<VkResult>(result);
			if (result != VK_SUCCESS)
			{
				CLog::Error("Failed to create window surface!", DODO_FAILED);
				return DODO_OK;
			}

			return DODO_OK;
		}

		void CWindow::Finalize()
		{
			glfwDestroyWindow(m_pWindow);
		}
	}
}