#include "pch.h"

#include "Texture.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace SE
{
	Texture::Texture(TextureType type)
	    : m_type(type)
	{
		switch (m_type)
		{
		case TextureType::Albedo: name = "albedoMap"; break;
		case TextureType::Normal: name = "normalMap"; break;
		case TextureType::Height: name = "heightMap"; break;
		case TextureType::AmbOcc: name = "aoMap"; break;
		case TextureType::Roughness: name = "roughnessMap"; break;
		case TextureType::Emissive: name = "emissiveMap"; break;
		case TextureType::Metallic: name = "metallicMap"; break;
		default: break;
		}
	}

	Texture::~Texture() {}

	std::string Texture::init(std::string_view filePath)
	{
		stbi_set_flip_vertically_on_load(true);
		int channels;
		unsigned char* image = stbi_load(filePath.data(), &width, &height, &channels, 0);
		if (!image)
		{
			return "Failed to load texture " + std::string(filePath);
		}
		else
		{
			if (channels == 1)
				format = GL_RED;
			else if (channels == 3)
				format = GL_RGB;
			else if (channels == 4)
				format = GL_RGBA;

			glGenTextures(1, &glTextureId);
			glBindTexture(GL_TEXTURE_2D, glTextureId);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, image);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); // tiles texture
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glBindTexture(GL_TEXTURE_2D, 0);
			stbi_image_free(image);

			return "";
		}
	}

	//for default texture fallback
	void Texture::initSolid(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		const unsigned char pixel[4] = { r, g, b, a };
		width = 1;
		height = 1;
		format = GL_RGBA;

		glGenTextures(1, &glTextureId);
		glBindTexture(GL_TEXTURE_2D, glTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixel);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//@brief deletes texture data
	void Texture::destroy()
	{
		if (glTextureId != 0)
		{
			glDeleteTextures(1, &glTextureId);
			glTextureId = 0;
		}
	}

	//return the default fallback texture based on the type enum
	Texture& Texture::getDefault(TextureType type)
	{
		auto& tex = s_defaults[static_cast<size_t>(type)];
		if (!tex)
		{
			tex = std::make_unique<Texture>(type);
			switch (type)
			{
				case TextureType::Albedo: tex->initSolid(255, 255, 255, 255); break;
				case TextureType::Normal: tex->initSolid(128, 128, 255, 255); break;
				case TextureType::Height: tex->initSolid(0, 0, 0, 255); break;
				case TextureType::AmbOcc: tex->initSolid(255, 255, 255, 255); break;
				case TextureType::Roughness: tex->initSolid(255, 255, 255, 255); break;
				case TextureType::Emissive: tex->initSolid(255, 255, 255, 255); break;
				case TextureType::Metallic: tex->initSolid(255, 255, 255, 255); break;
				default: tex->initSolid(255, 255, 255, 255); break;
			}
		}
		return *tex;
	}

	void Texture::destroyDefaults()
	{
		for (auto& tex : s_defaults)
		{
			if (tex) {
				tex->destroy();
			}
			tex.reset();
		}
	}
} // namespace SE
