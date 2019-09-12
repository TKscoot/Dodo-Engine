#include "dodopch.h"
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <assimp/cimport.h>

#pragma comment (lib, "ext/assimp/lib/assimp-vc140-mt.lib")

using namespace Dodo::Math;

static Vector3f convert(const aiVector3D& in, const float factor = 1.0f)
{
	Vector3f vec;
	vec.x = in.x * factor;
	vec.y = in.y * factor;
	vec.z = in.z * factor;
	return vec;
}

static Vector2f convert2f(const aiVector3D& in, const float factor = 1.0f)
{
	Vector2f vec;
	vec.x = in.x * factor;
	vec.y = in.y * factor;
	return vec;
}



Dodo::Environment::DodoError Dodo::Components::CMesh::CreateMeshFromFile(std::string _filename)
{
	uint32_t flags
		= aiProcess_GenUVCoords |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_FlipUVs |
		//aiProcess_MakeLeftHanded |
		aiProcess_FixInfacingNormals;
		//| aiProcess_FlipWindingOrder;

	Assimp::Importer importer;

	const aiScene *pScene = importer.ReadFile(_filename, flags);

	if (!pScene)
	{
		CLog::Error("Assimp scene == nullptr!");
		return DodoError::DODO_FAILED;
	}

	if (!pScene->HasMeshes())
	{
		CLog::Error("Assimp scene does not have any meshes!");
		return DodoError::DODO_FAILED;
	}

	// Prescan all meshes to get the total size!
	unsigned int totalVertexCount = 0;
	unsigned int totalFaceCount = 0;
	unsigned int totalIndexCount = 0;

	for (unsigned int k = 0; k < pScene->mNumMeshes; ++k)
	{
		aiMesh *pMesh = pScene->mMeshes[k];

		totalVertexCount += pMesh->mNumVertices;
		totalFaceCount += pMesh->mNumFaces;
		totalIndexCount += pMesh->mNumFaces * 3;

		for (unsigned int j = 0; j < pMesh->mNumFaces; ++j)
		{
			aiFace *pFace = pMesh->mFaces + j;
		}
	}

	std::vector<Vertex> vertices;
	vertices.resize(totalVertexCount);

	std::vector<uint32_t> indices;
	indices.resize(totalIndexCount);

	uint64_t currentVertexOffset = 0;
	uint64_t currentIndexOffset = 0;

	for (unsigned int k = 0; k < pScene->mNumMeshes; ++k)
	{
		aiMesh *pMesh = pScene->mMeshes[k];

		for (unsigned int v = 0; v < pMesh->mNumVertices; ++v)
		{
			vertices[currentVertexOffset + v].position  = convert(pMesh->mVertices[v]); // glm::normalize(convert(pMesh->mVertices[v]));
			vertices[currentVertexOffset + v].normal	= convert(pMesh->mNormals[v]);
			vertices[currentVertexOffset + v].tangent	= convert(pMesh->mTangents[v]);
			vertices[currentVertexOffset + v].texcoords = convert2f(pMesh->mTextureCoords[0][v]);
		}

		for (unsigned int j = 0; j < pMesh->mNumFaces; ++j)
		{
			aiFace *pFace = pMesh->mFaces + j;
			for (unsigned int i = 0; i < pFace->mNumIndices; ++i)
			{
				indices[currentIndexOffset + ((3 * j) + i)] = (currentVertexOffset + *(pFace->mIndices + i));
			}
		}

		currentVertexOffset = pMesh->mNumVertices;
		currentIndexOffset = pMesh->mNumFaces * 3;
	}

	m_vertices = vertices;
	m_indices  = indices;

	return DodoError::DODO_OK;
}

void Dodo::Components::CMesh::Finalize()
{

}
