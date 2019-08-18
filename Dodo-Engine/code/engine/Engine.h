#pragma once
#include "dodopch.h"

#include "environment/Error.h"
#include "environment/Log.h"
#include "environment/Window.h"
#include "common/VKIntegration.h"
#include "renderer/Renderer.h"
#include "components/Material.h"
#include "entity/EntityHandler.h"
#include "components/ECS.h"
#include "entity/Entity.h"
#include "entity/Camera.h"

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
			std::shared_ptr<Rendering::CRenderer>     m_pRenderer;
			std::shared_ptr<Entity::CCamera>		  m_pCamera;

			glm::vec2 m_v2WindowDimensions;
			double m_deltaTime = 0.0;
		};
	}
}