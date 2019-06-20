#include "dodopch.h"
#include "Engine.h"

namespace Dodo::Engine
{
	DodoError CEngine::Initialize()
	{
		CLog::CreateLogger("logger_console");
		CLog::SetFormat("[%H:%M:%S] [%^%l%$]: %v");
		DodoError result;
		m_pWindow = std::make_shared<CWindow>();
		result = m_pWindow->Initialize();
		CError::CheckError<DodoError>(result);
		m_v2WindowDimensions.x = 800;
		m_v2WindowDimensions.y = 600;
		result = m_pWindow->CreateGLFWWindow(m_v2WindowDimensions, "Test");
		CError::CheckError<DodoError>(result);
		
		// vulkan init
		m_pVulkanIntegration = std::make_shared<Rendering::VKIntegration>();
		VkResult vulkanResult = m_pVulkanIntegration->CreateInstance();
		CError::CheckError<VkResult>(vulkanResult);

		// creating window surface to draw on
		result = m_pWindow->CreateWindowSurface(m_pVulkanIntegration->vulkanInstance());
		CError::CheckError<DodoError>(result);

		m_pVulkanIntegration->CreateLogicalDevice(m_pWindow->GetSurface());

		Components::CMaterial::ShaderInfo shaderInfo;
		shaderInfo.vertexShaderFileName   = "shaders/default.vert.spv";
		shaderInfo.fragmentShaderFileName = "shaders/default.frag.spv";

		std::shared_ptr<Entity::CEntity> ent1 = std::make_shared<Entity::CEntity>();
		std::shared_ptr<Entity::CEntity> ent2 = std::make_shared<Entity::CEntity>();
		//std::shared_ptr<Entity::TestEnt> testEnt = std::make_shared<Entity::TestEnt>();
		
		std::shared_ptr<Components::CMaterial> mat1 = ent1->AddComponent<Components::CMaterial>(m_pVulkanIntegration, shaderInfo);
		std::shared_ptr<Components::CMaterial> mat2 = ent2->AddComponent<Components::CMaterial>(m_pVulkanIntegration, shaderInfo);

		std::vector<Vertex> vertices =
		{
			{{0.3f, -0.85f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f},	 {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.9f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f},	 {0.0f, 0.0f, 1.0f}}
		};

		mat2->m_vertices = vertices;

		std::vector<std::shared_ptr<Dodo::Entity::CEntity>> entities = Entity::CEntityHandler::GetEntities();
		std::vector<std::shared_ptr<Components::CMaterial>> materials = { mat1, mat2 };


		// renderer init
		m_pRenderer = std::make_shared<Rendering::CRenderer>(materials);
		m_pRenderer->Initialize(m_pVulkanIntegration, m_pWindow);

		CLog::Message("======== Engine Initialized! ========");

		return result;
	}

	DodoError CEngine::Run()
	{
		m_bRunning = true;
		DodoError result;
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (m_bRunning && !glfwWindowShouldClose(m_pWindow->GetWindow()))
		{
			// delta time stuff
			auto newtime = std::chrono::high_resolution_clock::now();
			lastTime     = std::chrono::high_resolution_clock::now();

			// Poll window and keyboard events
			glfwPollEvents();

			// Update all entities
			result = Update();

			// Draw the frame
			m_pRenderer->DrawFrame();

			// delta time stuff
			newtime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> delta = newtime - lastTime;

			m_deltaTime = delta.count();

			float fps = 1.0 / m_deltaTime;
			char buf[10];
			sprintf_s(buf, "%.1f", fps);
			glfwSetWindowTitle(m_pWindow->GetWindow(), buf);
			newtime = lastTime;
		}

		return result;
	}

	DodoError CEngine::Update()
	{
		Entity::CEntityHandler::Update();

		return DodoError::DODO_OK;
	}

	DodoError CEngine::Finalize()
	{
		return DodoError();
	}
}