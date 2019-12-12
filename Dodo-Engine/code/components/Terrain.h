#pragma once
#include "dodopch.h"
#include "environment/Error.h"
#include "common/DodoTypes.h"
#include "ECS.h"
#include "Mesh.h"
#include "common/SimplexNoise.h"

namespace Dodo
{
	namespace Components
	{
		using namespace Dodo::Environment;
		using namespace Dodo::Rendering;
		using namespace Dodo::Math;

		class CTerrain : public CMesh
		{
		public:
			CTerrain(uint32_t _patchSize, float _uvScale)
				: m_patchSize(_patchSize)
				, m_uvScale(_uvScale)
				, m_vertexCount(_patchSize * _patchSize)
			{
				m_vertices.clear();
				m_indices.clear();

				// initialize and create flat terrain
				ConstructTerrain();

				

			};

		private:
			const uint32_t m_patchSize;
			const float m_uvScale;
			const uint32_t m_vertexCount;

			void ConstructTerrain();
			int storeQuad(int pointer, int topLeft, int topRight, int bottomLeft, int bottomRight)
			{
				m_indices[pointer++] = topLeft;
				m_indices[pointer++] = bottomLeft;
				m_indices[pointer++] = bottomRight;
				m_indices[pointer++] = topLeft;
				m_indices[pointer++] = bottomRight;
				m_indices[pointer++] = topRight;
				return pointer;
			}

		};
	}
}
