#pragma once
#include <memory>
#include <chrono>

#include "environment/Error.h"
#include "environment/Log.h"
#include "environment/Window.h"
#include "common/VKIntegration.h"
#include "renderer/Renderer.h"
#include "renderer/Material.h"

namespace Dodo
{
	namespace Engine
	{
		using namespace Dodo::Environment;
		
		class CEngine
		{
		public:
			CEngine() = default;

			DodoError Initialize();
			DodoError Run();
			DodoError Update();
			DodoError Finalize();

			// Getter/Setter

		private:
			bool m_bRunning = false;
			std::shared_ptr<CWindow> m_pWindow;
			std::shared_ptr<Rendering::VKIntegration> m_pVulkanIntegration;
			std::shared_ptr<Rendering::CRenderer> m_pRenderer;

			glm::vec2 m_v2WindowDimensions;
			double m_deltaTime = 0.0;
		};
	}
}