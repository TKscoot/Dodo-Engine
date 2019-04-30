#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

#pragma comment(lib, "glfw3.lib")

#include <memory>
#include "environment/Error.h"

namespace Dodo
{
	namespace Environment
	{
		class CWindow
		{
		public:

			DodoError Initialize();

			DodoError CreateGLFWWindow(const glm::vec2 _dimensions, const char* _title);
			DodoError CreateWindowSurface(VkInstance _instance);

			// getter/setter
			GLFWwindow *GetWindow(){ return m_pWindow; }
			VkSurfaceKHR *GetSurface(){return &m_vkSurface;}

			static void framebufferResizeCallback(GLFWwindow* _window, int _width, int _height);

		private:
			const GLFWvidmode *m_pVideoMode = nullptr;
			GLFWmonitor       *m_pMonitor   = nullptr;
			GLFWwindow        *m_pWindow	= nullptr;
			VkSurfaceKHR	   m_vkSurface  = VK_NULL_HANDLE;

		};

	}
}

