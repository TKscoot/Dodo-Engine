#pragma once
#include "dodopch.h"

#include "environment/Error.h"
#include "environment/Log.h"
#include "environment/Window.h"
#include "environment/Input.h"
#include "common/VKIntegration.h"
#include "renderer/Renderer.h"
#include "components/Material.h"
#include "entity/EntityHandler.h"
#include "components/ECS.h"
#include "entity/Entity.h"
#include "entity/Camera.h"
#include "renderer/GUI.h"


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
			static double const deltaTime() { return m_deltaTime; }


		private:
			bool  m_bRunning = false;
			float m_fTimer = 0.0f;
			std::shared_ptr<CWindow> m_pWindow;
			std::shared_ptr<CInput> m_pInput;
			std::shared_ptr<Rendering::VKIntegration> m_pVulkanIntegration;
			std::shared_ptr<Rendering::CRenderer>     m_pRenderer;
			std::shared_ptr<Entity::CCamera>		  m_pCamera;
			std::shared_ptr<Rendering::GUI>			  m_pGui;

			glm::vec2 m_v2WindowDimensions;
			static double m_deltaTime;
		};
	}
}