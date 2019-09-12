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
		m_v2WindowDimensions.x = 1280;
		m_v2WindowDimensions.y = 720;
		result = m_pWindow->CreateGLFWWindow(m_v2WindowDimensions, "Dodo Engine v0.1 alpha");
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
		m_pCamera->setPerspective(60.0f, m_v2WindowDimensions.x / m_v2WindowDimensions.y, 0.001f, 2048.0f);	// TODO: use current swap extent for aspect ratio
		m_pCamera->Update();


		Components::CMaterial::ShaderInfo shaderInfo;
		shaderInfo.vertexShaderFileName   = "shaders/default.vert.spv";
		shaderInfo.fragmentShaderFileName = "shaders/pbr.frag.spv";

		//std::shared_ptr<Entity::CEntity> boxEntity = std::make_shared<Entity::CEntity>();
		//std::shared_ptr<Entity::CEntity> pepeEntity = std::make_shared<Entity::CEntity>();

		Entity::CEntity* boxEntity   = new Entity::CEntity("Quad");
		Entity::CEntity* pepeEntity  = new Entity::CEntity("Pepe");
		Entity::CEntity* floorEntity = new Entity::CEntity("Floor");
		
		std::shared_ptr<Components::CMesh> mesh1 = boxEntity->AddComponent<Components::CMesh>();
		std::shared_ptr<Components::CMesh> mesh2 = pepeEntity->AddComponent<Components::CMesh>();
		std::shared_ptr<Components::CMesh> mesh3 = floorEntity->AddComponent<Components::CMesh>();
		std::shared_ptr<Components::CMaterial> mat1 = boxEntity->AddComponent<Components::CMaterial>(m_pVulkanIntegration, shaderInfo);
		//mat1->SetTexture("resources/textures/WoodBox/default.jpg");
		mat1->SetTextures(
			//"resources/textures/RustedIron/rustediron2_basecolor.png",
			"resources/textures/Grass.jpg",

			"resources/textures/RustedIron/rustediron2_normal.png",
			"resources/textures/RustedIron/rustediron2_metallic.png",
			"resources/textures/RustedIron/rustediron2_metallic.png");
		std::shared_ptr<Components::CMaterial> mat2 = pepeEntity->AddComponent<Components::CMaterial>(m_pVulkanIntegration, shaderInfo);
		//mat2->SetTexture("resources/textures/pepe_text.png");
		mat2->SetTextures(
				"resources/textures/pepe_text.png",
				"resources/textures/grey.png",
				"resources/textures/grey.png",
				"resources/textures/grey.png");
		std::shared_ptr<Components::CMaterial> mat3 = floorEntity->AddComponent<Components::CMaterial>(m_pVulkanIntegration, shaderInfo);
		//mat3->SetTexture("resources/textures/Grass.jpg");
		mat3->SetTextures(
			"resources/textures/RustedIron/rustediron2_basecolor.png",
			"resources/textures/RustedIron/rustediron2_normal.png",
			"resources/textures/RustedIron/rustediron2_metallic.png",
			"resources/textures/grey.png");

		auto boxTrans = boxEntity->AddComponent<Components::CTransform>();
		//boxTrans->setScale(Math::Vector3f(0.05f));
		boxTrans->setPositionX(5.0f);
		auto floorTrans = floorEntity->AddComponent<Components::CTransform>();
		floorTrans->setPositionY(-0.3f);
		floorTrans->setPositionX(20.0f);
		floorTrans->setScaleX(20.0f);
		floorTrans->setScaleY(20.0f);
		floorTrans->setScaleZ(20.0f);
		auto pepeTrans  = pepeEntity->AddComponent<Components::CTransform>();
		pepeTrans->setPosition(Math::Vector3f(7.5f, 0.0f, 0.0f));
		pepeTrans->setScale(Math::Vector3f(3.0f));


		std::vector<Vertex> vertices =
		{
			{{0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f},	 {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f},	 {0.0f, 0.0f, 1.0f}}
		};

		//mesh1->CreateMeshFromFile("resources/models/sponza.obj");
		mesh1->CreateMeshFromFile("resources/models/bunny.obj");
		mesh2->CreateMeshFromFile("resources/models/pepeWithNormals.obj");
		mesh3->CreateMeshFromFile("resources/models/Sci-Fi-Floor-1-OBJ.obj");

		std::vector<std::shared_ptr<Dodo::Entity::CEntity>> entities = {};
		for (auto &ent : Entity::CEntityHandler::GetEntities())
		{
			entities.push_back(std::shared_ptr<Entity::CEntity>(ent));
		}
		std::vector<std::shared_ptr<Components::CMaterial>> materials = {mat1 , mat2, mat3 };
		std::vector<std::shared_ptr<Components::CMesh>> meshes = { mesh1, mesh2, mesh3 };


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

			//m_fTimer += m_deltaTime;
			//
			//if (m_fTimer >= 1.0f)
			//{
			//	float fps = 1.0 / m_deltaTime;
			//	char buf[10];
			//	sprintf_s(buf, "%.1f", fps);
			//	glfwSetWindowTitle(m_pWindow->GetWindow(), buf);
			//	m_fTimer = 0.0f;
			//}
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