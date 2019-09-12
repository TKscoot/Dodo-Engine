#include "dodopch.h"
#include "Material.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Dodo::Environment::DodoError Dodo::Components::CMaterial::LoadTexture(Texture &texture)
{
	int texWidth, texHeight, texChannels;
	

	stbi_uc* pixels = stbi_load(texture.filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);


	if (!pixels)
	{
		CLog::Error("Failed to load texture image!");
		return DodoError::DODO_FAILED;
	}

	texture.texWidth    = texWidth;
	texture.texHeight   = texHeight;
	texture.texChannels = texChannels;
	texture.pixels	    = pixels;


	return DodoError::DODO_OK;
}

void Dodo::Components::CMaterial::Finalize()
{
	vkDestroySampler(m_pIntegration->device(),   m_textures.roughness.textureData.textureSampler, nullptr);
	vkDestroyImageView(m_pIntegration->device(), m_textures.roughness.textureData.textureImageView, nullptr);

	vkDestroySampler(m_pIntegration->device(),   m_textures.metallic.textureData.textureSampler, nullptr);
	vkDestroyImageView(m_pIntegration->device(), m_textures.metallic.textureData.textureImageView, nullptr);

	vkDestroySampler(m_pIntegration->device(),   m_textures.normal.textureData.textureSampler, nullptr);
	vkDestroyImageView(m_pIntegration->device(), m_textures.normal.textureData.textureImageView, nullptr);

	vkDestroySampler(m_pIntegration->device(),   m_textures.albedo.textureData.textureSampler, nullptr);
	vkDestroyImageView(m_pIntegration->device(), m_textures.albedo.textureData.textureImageView, nullptr);


	vkDestroyImage(m_pIntegration->device(), m_textures.roughness.textureData.textureImage, nullptr);
	vkFreeMemory(m_pIntegration->device(),   m_textures.roughness.textureData.textureImageMemory, nullptr);

	vkDestroyImage(m_pIntegration->device(), m_textures.metallic.textureData.textureImage, nullptr);
	vkFreeMemory(m_pIntegration->device(),   m_textures.metallic.textureData.textureImageMemory, nullptr);

	vkDestroyImage(m_pIntegration->device(), m_textures.normal.textureData.textureImage, nullptr);
	vkFreeMemory(m_pIntegration->device(),   m_textures.normal.textureData.textureImageMemory, nullptr);

	vkDestroyImage(m_pIntegration->device(), m_textures.albedo.textureData.textureImage, nullptr);
	vkFreeMemory(m_pIntegration->device(),   m_textures.albedo.textureData.textureImageMemory, nullptr);
}
