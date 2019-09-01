#include "dodopch.h"
#include "Engine.h"

double Dodo::Engine::CEngine::m_deltaTime = 1.0;

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

		// input init
		m_pInput = std::make_shared<CInput>(m_pWindow);

		// vulkan init
		m_pVulkanIntegration = std::make_shared<Rendering::VKIntegration>();
		VkResult vulkanResult = m_pVulkanIntegration->CreateInstance();
		auto limits = m_pVulkanIntegration->GetPhysDevLimits();

		CError::CheckError<VkResult>(vulkanResult);

		// creating window surface to draw on
		result = m_pWindow->CreateWindowSurface(m_pVulkanIntegration->vulkanInstance());
		CError::CheckError<DodoError>(result);

		m_pVulkanIntegration->CreateLogicalDevice(m_pWindow->GetSurface());

		m_pCamera = std::make_shared<Entity::CCamera>();
		m_pCamera->setName("Camera");
		//m_pCamera->GetComponent<Components::CTransform>()->setPosition(Math::Vector3f(15.0f, -15.0f, 0.0f));
		//m_pCamera->GetComponent<Components::CTransform>()->setPosition(Math::Vector3f(0.0f, 1.0f, 0.0f));
		//m_pCamera->GetComponent<Components::CTransform>()->SetParent(nullptr);
		m_pCamera->setPerspective(60.0f, m_v2WindowDimensions.x / m_v2WindowDimensions.y, 0.1f, 256.0f);	// TODO: use current swap extent for aspect ratio
		m_pCamera->Update();


		Components::CMaterial::ShaderInfo shaderInfo;
		shaderInfo.vertexShaderFileName   = "shaders/default.vert.spv";
		shaderInfo.fragmentShaderFileName = "shaders/default.frag.spv";

		//std::shared_ptr<Entity::CEntity> ent1 = std::make_shared<Entity::CEntity>();
		//std::shared_ptr<Entity::CEntity> ent2 = std::make_shared<Entity::CEntity>();

		Entity::CEntity* ent1 = new Entity::CEntity("Quad");
		Entity::CEntity* ent2 = new Entity::CEntity("Pepe");
		
		std::shared_ptr<Components::CMesh> mesh1 = ent1->AddComponent<Components::CMesh>();
		std::shared_ptr<Components::CMesh> mesh2 = ent2->AddComponent<Components::CMesh>();
		std::shared_ptr<Components::CMaterial> mat1 = ent1->AddComponent<Components::CMaterial>(m_pVulkanIntegration, shaderInfo);
		mat1->SetTexture("resources/textures/Grass.jpg");
		std::shared_ptr<Components::CMaterial> mat2 = ent2->AddComponent<Components::CMaterial>(m_pVulkanIntegration, shaderInfo);
		mat2->SetTexture("resources/textures/pepe_text.png");
		auto transform = ent2->AddComponent<Components::CTransform>();
		transform->setPosition(Math::Vector3f(3.0f, 0.0f, 0.0f));

		std::vector<Vertex> vertices =
		{
			{{0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f},	 {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f},	 {0.0f, 0.0f, 1.0f}}
		};

		mesh1->CreateMeshFromFile("resources/models/WoodBox.obj");
		mesh2->CreateMeshFromFile("resources/models/pepeWithNormals.obj");

		std::vector<std::shared_ptr<Dodo::Entity::CEntity>> entities = {};
		for (auto &ent : Entity::CEntityHandler::GetEntities())
		{
			entities.push_back(std::shared_ptr<Entity::CEntity>(ent));
		}
		std::vector<std::shared_ptr<Components::CMaterial>> materials = {mat1 , mat2 };
		std::vector<std::shared_ptr<Components::CMesh>> meshes = { mesh1, mesh2 };


		// renderer init
		m_pRenderer = std::make_shared<Rendering::CRenderer>(meshes, materials, m_pCamera, entities);
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
			m_pRenderer->DrawFrame(m_deltaTime);

			// delta time stuff
			newtime = std::chrono::high_resolution_clock::now();
			std::chrono::duration<double> delta = newtime - lastTime;
			m_deltaTime = delta.count();

			m_fTimer += m_deltaTime;

			if (m_fTimer >= 1.0f)
			{
				float fps = 1.0 / m_deltaTime;
				char buf[10];
				sprintf_s(buf, "%.1f", fps);
				glfwSetWindowTitle(m_pWindow->GetWindow(), buf);
				m_fTimer = 0.0f;
			}
			newtime = lastTime;
		}
		
		return result;
	}

	DodoError CEngine::Update()
	{
		m_pCamera->Update();
		for (auto ent : Entity::CEntityHandler::GetEntities())
		{
			ent->UpdateComponents();
			ent->Update();
		}

		return DodoError::DODO_OK;
	}

	DodoError CEngine::Finalize()
	{
		m_pRenderer->Finalize();
		m_pWindow->Finalize();
		glfwTerminate();

		return DodoError::DODO_OK;
	}
}