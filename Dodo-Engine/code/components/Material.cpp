#include "dodopch.h"
#include "Material.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

Dodo::Environment::DodoError Dodo::Components::CMaterial::LoadTexture()
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(m_texture.filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	if (!pixels)
	{
		CLog::Error("Failed to load texture image!");
		return DodoError::DODO_FAILED;
	}

	m_texture.texWidth    = texWidth;
	m_texture.texHeight   = texHeight;
	m_texture.texChannels = texChannels;
	m_texture.pixels	  = pixels;

	return DodoError::DODO_OK;
}
