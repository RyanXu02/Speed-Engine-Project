#include "pch.h"

#include "Texture.h"

#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace SE
{
	Texture::~Texture()
	{
		if (m_textureId != 0)
		{
			glDeleteTextures(1, &m_textureId);
		}
	}

	std::string Texture::init(std::string_view filePath)
	{
		assert(m_textureId == 0);

		// load image
		stbi_set_flip_vertically_on_load(true);
		int width = 0, height = 0, channels = 0;
		const std::string path(filePath);
		unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, 0);
		if (!image)
			return "Failed to load texture " + std::string(filePath);

		// create OpenGL texture
		glGenTextures(1, &m_textureId);
		if (!m_textureId)
		{
			stbi_image_free(image);
			return "Failed to create GPU texture " + path;
		}

		// save OpenGL state and set unpack parameters for texture upload
		GLint previousTexture = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
		const GLenum unpackNames[] = { GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_ROWS,
			GL_UNPACK_SKIP_PIXELS, GL_PIXEL_UNPACK_BUFFER_BINDING };
		GLint unpack[5]{};
		for (size_t i = 0; i < std::size(unpackNames); ++i)
			glGetIntegerv(unpackNames[i], &unpack[i]);

		// reset unpack parameters to default values
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		
		// upload texture data to GPU
		assert(channels >= 1 && channels <= 4);
		const GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
		const GLenum internalFormats[] = { GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 };
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormats[channels - 1], width, height, 0,
			formats[channels - 1], GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		// restore previous OpenGL state
		glBindTexture(GL_TEXTURE_2D, previousTexture);
		for (size_t i = 0; i < 4; ++i)
			glPixelStorei(unpackNames[i], unpack[i]);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack[4]);

		stbi_image_free(image);
		return "";
	}
} // namespace SE
