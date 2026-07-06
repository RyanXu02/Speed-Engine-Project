#include "pch.h"
#include "Texture.h"
#include "glad/gl.h"

namespace SE 
{
	Texture::Texture(TextureType type) : m_type(type) {
		switch (m_type) {
		case TextureType::Albedo:
			name = "albedo";
			break;
		case TextureType::Normal:
			name = "normalmap";
			break;
		case TextureType::Height:
			name = "heightmap";
			break;
		case TextureType::AmbOcc:
			name = "ambientOcc";
			break;
		case TextureType::Roughness:
			name = "roughness";
			break;
		default:
			break;
		}
	}

	Texture::~Texture() {

	}

	void Texture::init() {

	}

	//@brief deletes texture data
	void Texture::destroy() {
		glDeleteTextures(1, &glTextureId);
	}
}