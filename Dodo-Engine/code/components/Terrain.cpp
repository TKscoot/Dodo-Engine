#include "dodopch.h"
#include "Terrain.h"

void Dodo::Components::CTerrain::ConstructTerrain()
{
	// constructing terrain vertices of patchSize * patchSize
	m_vertices.resize(m_vertexCount);
	const float wx = 2.0f;
	const float wy = 2.0f;

	for (auto x = 0; x < m_patchSize; x++)
	{
		for (auto y = 0; y < m_patchSize; y++)
		{
			uint32_t index = (x + y * m_patchSize);
			m_vertices[index].position[0] = x * wx + wx / 2.0f - (float)m_patchSize * wx / 2.0f;
			m_vertices[index].position[1] = 0.0f;
			m_vertices[index].position[2] = y * wy + wy / 2.0f - (float)m_patchSize * wy / 2.0f;
			m_vertices[index].texcoords = glm::vec2((float)x / m_patchSize, (float)y / m_patchSize) * m_uvScale;
		}
	}

	// TODO: apply heightmap here!


	// End TODO



	// Calculating indices
	const uint32_t w = (m_patchSize - 1);
	const uint32_t indexCount = w * w * 4;
	
	m_indices.resize(4000000);
	
	//for (auto x = 0; x < w; x++)
	//{
	//	for (auto y = 0; y < w; y++)
	//	{
	//		uint32_t index = (x + y * w) * 4;
	//		m_indices[index] = (x + y * m_patchSize);
	//		m_indices[index + 1] = m_indices[index] + m_patchSize;
	//		m_indices[index + 2] = m_indices[index + 1] + 1;
	//		m_indices[index + 3] = m_indices[index] + 1;	
	//	}
	//}


	for (int i = 0; i < m_vertexCount; i++)
	{
		// First trangle of side - CCW from bottom left
		//m_indices[i  ] = i;			   // vertex 0
		//m_indices[i + 1] = i + 1;        // vertex 1
		//m_indices[i + 2] = i + m_patchSize;        // vertex 2
						  
		// Second triangle of side - CCW from bottom left
		//m_indices[i + 3] = i + 1;        // vertex 0
		//m_indices[i + 4] = i + 2;        // vertex 1
		//m_indices[i + 5] = i + 3;        // vertex 2
		//i += 2;
	}
	// First trangle of side - CCW from bottom left
	//m_indices[0  ]	 = 0;			   // vertex 0
	//m_indices[1] = m_patchSize;        // vertex 1
	//m_indices[2] =  1;        // vertex 2
	
	// Second triangle of side - CCW from bottom left
	//m_indices[3] = m_patchSize;        // vertex 0
	//m_indices[4] = 1;        // vertex 1
	//m_indices[5] = m_patchSize + 1;        // vertex 2


	// KLAPPT ABER IST FALSCH!!!

	int idx = 0;
	short pitch = (short)(m_patchSize - 1);
	short i1 = 0;
	short i2 = 1;
	short i3 = (short)(1 + pitch);
	short i4 = pitch;

	short row = 0;

	for (int z = 0; z < m_patchSize; z++)
	{
		for (int x = 0; x < m_patchSize - 1; x++)
		{
			m_indices[idx++] = i1;
			m_indices[idx++] = i2;
			m_indices[idx++] = i3;

			m_indices[idx++] = i3;
			m_indices[idx++] = i4;
			m_indices[idx++] = i1;

			i1++;
			i2++;
			i3++;
			i4++;

		}

		row += pitch;
		i1 = row;
		i2 = (short)(row + 1);
		i3 = (short)(i2 + pitch);
		i4 = (short)(row + pitch);
	}
}
