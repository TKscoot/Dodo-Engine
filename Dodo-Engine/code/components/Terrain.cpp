#include "dodopch.h"
#include "Terrain.h"

void Dodo::Components::CTerrain::ConstructTerrain()
{
	float persistence = 1.0f;
	const float wx = 25.0f;
	const float wy = 25.0f;


	// constructing terrain vertices of patchSize * patchSize
	m_vertices.resize(m_vertexCount);
	int idx = 0;
	for (auto y = 0; y < m_patchSize; y++)
	{
		for (auto x = 0; x < m_patchSize; x++)
		{

			// Noise calculations
			int octaves = 5;
			double total = 0;
			double frequency = 1;
			double amplitude = 1;
			double maxValue = 0;  // Used for normalizing result to 0.0 - 1.0
			for (int i = 0; i < octaves; i++)
			{
				total += SimplexNoise::noise(x * frequency, y * frequency) * amplitude;

				maxValue += amplitude;

				amplitude *= persistence;
				frequency *= 2;
			}

			m_vertices[idx].position.x = x * wx + wx / 2.0f - (float)m_patchSize * wx / 2.0f;;
			m_vertices[idx].position.y = (total / maxValue) * 40.0f;
			m_vertices[idx].position.z = y * wy + wy / 2.0f - (float)m_patchSize * wy / 2.0f;;
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

			Vector3f U = Vector3f(i2, i1, i0);
			Vector3f V = Vector3f(i2, i3, i1);

			// Add these indices to a list.
			m_indices.push_back(i2);
			m_indices.push_back(i1);
			m_indices.push_back(i0);
			m_indices.push_back(i2);
			m_indices.push_back(i3);
			m_indices.push_back(i1);
		}
	}

	// Calculate Normals

	for (int i = 0; i < m_indices.size(); i += 3)
	{
		// Get the face normal
		Vector3f vector1 = m_vertices[m_indices[(size_t)i + 1]].position - m_vertices[m_indices[i]].position;
		Vector3f vector2 = m_vertices[m_indices[(size_t)i + 2]].position - m_vertices[m_indices[i]].position;
		Vector3f faceNormal = glm::cross(vector1, vector2);
		//auto faceNormal = sf::VectorCross(vector1, vector2);
		//sf::Normalize(faceNormal);
		faceNormal = glm::normalize(faceNormal);

		// Add the face normal to the 3 vertices normal touching this face
		m_vertices[m_indices[i]].normal += faceNormal;
		m_vertices[m_indices[(size_t)i + 1]].normal += faceNormal;
		m_vertices[m_indices[(size_t)i + 2]].normal += faceNormal;
	}

	// Normalize vertices normal
	for (int i = 0; i < m_vertices.size(); i++)
	{
		//sf::Normalize(m_vertices[i].normal);
		m_vertices[i].normal = glm::normalize(m_vertices[i].normal);
	}

}
