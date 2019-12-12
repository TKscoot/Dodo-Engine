#include "dodopch.h"
#include "Terrain.h"

void Dodo::Components::CTerrain::ConstructTerrain()
{
	// constructing terrain vertices of patchSize * patchSize
	//m_vertices.resize(m_vertexCount);
	//const float wx = 2.0f;
	//const float wy = 2.0f;
	CreateMeshFromFile("resources/models/terrain.obj");


	int idx = 0;
	for (auto y = 0; y < m_patchSize; y++)
	{
		for (auto x = 0; x < m_patchSize; x++)
		{
			//m_vertices[idx].position.x = x;
			//m_vertices[idx].position.y = SimplexNoise::noise(x * 0.1f, y * 0.1f) * 10.0f;
			//m_vertices[idx].position.z = y;
			idx++;
		}
	}

	//for (auto x = 0; x < m_patchSize; x++)
	//{
	//	for (auto y = 0; y < m_patchSize; y++)
	//	{
	//		uint32_t index = (x + y * m_patchSize);
	//		m_vertices[index].position[0] = x * wx + wx / 2.0f - (float)m_patchSize * wx / 2.0f;
	//		m_vertices[index].position[1] = 1.0f
	//		m_vertices[index].position[2] = y * wy + wy / 2.0f - (float)m_patchSize * wy / 2.0f;
	//		m_vertices[index].texcoords = glm::vec2((float)x / m_patchSize, (float)y / m_patchSize) * m_uvScale;
	//	}
	//}


	//for (auto i = 0; i < m_patchSize; i++)
	//{
	//	for (auto j = 0; j < m_patchSize; j++)
	//	{
	//		uint32_t index = (i + j * m_patchSize);
	//
	//		const auto factorRow = float(i) / float(m_patchSize - 1) * 100.0f;
	//		const auto factorColumn = float(j) / float(m_patchSize - 1) * 100.0f;
	//		//const auto& vertexHeight = _heightData[i][j];
	//
	//
	//		m_vertices[index].position = glm::vec3(factorColumn, noise * 10.0f, factorRow);
	//	}
	//	//_vbo.addData(_vertices[i].data(), m_patchSize * sizeof(glm::vec3));
	//}


	// TODO: apply heightmap here!

	//for (int i = 0; i < m_patchSize * m_patchSize; i++)
	//{
	//	if (i % 50 == 0)
	//	{
	//		m_vertices[i].position.y = 3.0f;
	//	}
	//}

	// End TODO

	//m_indices.clear();
	//
	//for (uint64_t k = 0; k < m_patchSize; ++k)
	//{
	//	for (uint64_t j = 0; j < m_patchSize; ++j)
	//	{
	//		// CW
	//		Vector3f triangle0 = Vector3f((j*m_patchSize) + k, (j*m_patchSize) + (k + 1), (j + 1)*m_patchSize + k);
	//		Vector3f triangle1 = Vector3f((j + 1)*m_patchSize + (k), (j*m_patchSize) + (k + 1), (j + 1)*m_patchSize + (k + 1));
	//
	//		// Add these indices to a list.
	//		m_indices.push_back(triangle0.x);
	//		m_indices.push_back(triangle0.y);
	//		m_indices.push_back(triangle0.z);
	//		m_indices.push_back(triangle1.x);
	//		m_indices.push_back(triangle1.y);
	//		m_indices.push_back(triangle1.z);
	//
	//	}
	//}
}
