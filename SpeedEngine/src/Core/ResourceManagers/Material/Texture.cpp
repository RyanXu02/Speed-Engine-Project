#include "pch.h"
#include "Texture.h"

#include "glad/gl.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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

	std::string Texture::init(std::string_view filePath) {
		stbi_set_flip_vertically_on_load(true);
		int channels;
		unsigned char* image = stbi_load(filePath.data(), &width, &height, &channels, 0);
		if (!image) {
			return "Failed to load texture " + std::string(filePath);
		}
		else {
			if (channels == 1) format = GL_RED;
			else if (channels == 3) format = GL_RGB;
			else if (channels == 4) format = GL_RGBA;

			glGenTextures(1, &glTextureId);
			glBindTexture(GL_TEXTURE_2D, glTextureId);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //tiles texture
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(image);

			return "";
		}
	}

	//@brief deletes texture data
	void Texture::destroy() {
		glDeleteTextures(1, &glTextureId);
	}
}