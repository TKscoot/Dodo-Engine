#include "Engine.h"

namespace Dodo::Engine
{
	DodoError CEngine::Initialize()
	{
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

		Rendering::CMaterial::ShaderInfo shaderInfo;
		shaderInfo.vertexShaderFileName   = "shaders/vert.spv";
		shaderInfo.fragmentShaderFileName = "shaders/frag.spv";

		std::shared_ptr<Rendering::TestMaterial> mat = std::make_shared<Rendering::TestMaterial>(m_pVulkanIntegration, shaderInfo);

		mat->Update();

		std::vector<std::shared_ptr<Rendering::CMaterial>> materials = {};
		materials.push_back(mat);

		// renderer init
		m_pRenderer = std::make_shared<Rendering::CRenderer>(materials);
		m_pRenderer->Initialize(m_pVulkanIntegration, m_pWindow);



		//std::shared_ptr<Rendering::TestMaterial> mat = std::make_shared<Rendering::TestMaterial>();
		

		return result;
	}

	DodoError CEngine::Run()
	{
		m_bRunning = true;
		DodoError result;
		auto lastTime = std::chrono::high_resolution_clock::now();
		while (m_bRunning && !glfwWindowShouldClose(m_pWindow->GetWindow()))
		{
			auto newtime = std::chrono::high_resolution_clock::now();
			lastTime     = std::chrono::high_resolution_clock::now();
			glfwPollEvents();
			result = Update();
			m_pRenderer->DrawFrame();
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

		return DodoError::DODO_OK;
	}

	DodoError CEngine::Finalize()
	{
		return DodoError();
	}
}