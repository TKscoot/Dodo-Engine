#include "dodopch.h"
#include "Terrain.h"

void Dodo::Components::CTerrain::ConstructTerrain()
{
	// constructing terrain vertices of patchSize * patchSize
	m_vertices.resize(m_vertexCount);
	int idx = 0;
	for (auto y = 0; y < m_patchSize; y++)
	{
		for (auto x = 0; x < m_patchSize; x++)
		{
			m_vertices[idx].position.x = x;
			m_vertices[idx].position.y = SimplexNoise::noise(x * 0.1f, y * 0.1f) * 10.0f;
			m_vertices[idx].position.z = y;
			m_vertices[idx].texcoords = glm::vec2((float)x / m_patchSize, (float)y / m_patchSize) * m_uvScale;

			idx++;
		}
	}

	// Calculating indices
	m_indices.clear();
	for (uint64_t x = 0; x < (m_patchSize - 1); ++x)
	{
		for (uint64_t y = 0; y < (m_patchSize - 1); ++y)
		{
			uint64_t i0 = ((y + 0) * m_patchSize) + (x + 0);
			uint64_t i1 = ((y + 0) * m_patchSize) + (x + 1);
			uint64_t i2 = ((y + 1) * m_patchSize) + (x + 0);
			uint64_t i3 = ((y + 1) * m_patchSize) + (x + 1);

			// Add these indices to a list.
			m_indices.push_back(i2);
			m_indices.push_back(i1);
			m_indices.push_back(i0);
			m_indices.push_back(i2);
			m_indices.push_back(i3);
			m_indices.push_back(i1);
		}
	}
}
