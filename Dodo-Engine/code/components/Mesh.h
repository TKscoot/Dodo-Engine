#pragma once
#include "dodopch.h"
#include "environment/Error.h"
#include "common/DodoTypes.h"
#include "ECS.h"

namespace Dodo
{
	namespace Components
	{
		using namespace Dodo::Environment;
		using namespace Dodo::Rendering;

		class CMesh : public CComponent
		{
		public:
			struct DataBuffer
			{
				// Vertex buffer
				VkBuffer vertexBuffer;
				VkDeviceMemory vertexBufferMemory;

				// Index buffer
				VkBuffer indexBuffer;
				VkDeviceMemory indexBufferMemory;
			};

			struct DepthData
			{
				VkImage		   depthImage;
				VkDeviceMemory depthImageMemory;
				VkImageView	   depthImageView;
			};

			const std::vector<Vertex> const vertices() { return m_vertices; }

			std::vector<Vertex> m_vertices = {
				{{-0.5f, -0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
				{{0.5f,  -0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
				{{0.5f,   0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
				{{-0.5f,  0.5f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {1.0f, 1.0f}, {1.0f, 1.0f, 1.0f}}
			};

			std::vector<uint32_t> m_indices = {
				0, 1, 2, 2, 3, 0
			};

			DodoError CreateMeshFromFile(std::string _filename);

			void Finalize();

			DataBuffer m_dataBuffers;
			DepthData  m_depthData;
		private:

		};
	}
}