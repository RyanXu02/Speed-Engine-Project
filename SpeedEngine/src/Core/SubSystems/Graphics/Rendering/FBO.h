#pragma once

#include <glad/glad.h>

namespace SE
{
	// @brief A texture attachment for an FBO
	// @brief Defaults to a RGBA8 texture
	// @param internalFormat The internal format of the texture
	// @param format The format of the texture
	// @param type The type of the texture
	// @param minFilter The minification filter of the texture
	// @param magFilter The magnification filter of the texture
	// @param wrapS The wrapping mode for the S coordinate
	// @param wrapT The wrapping mode for the T coordinate
	// @param borderColor The border color of the texture
	struct Attachment
	{
		GLint internalFormat = GL_RGBA8;
		GLenum format = GL_RGBA;
		GLenum type = GL_UNSIGNED_BYTE;
		GLint minFilter = GL_LINEAR;
		GLint magFilter = GL_LINEAR;
		GLint wrapS = GL_CLAMP_TO_EDGE;
		GLint wrapT = GL_CLAMP_TO_EDGE;
		std::array<float, 4> borderColor{ 1.0f, 1.0f, 1.0f, 1.0f };
	};

	// @brief Descriptor for creating an FBO
	// @param width The width of the FBO
	// @param height The height of the FBO
	// @param colors The color attachments of the FBO
	// @param depth The depth attachment of the FBO
	struct Descriptor
	{
		uint32_t width = 1;
		uint32_t height = 1;
		std::vector<Attachment> colors{ Attachment{} };
		std::optional<Attachment> depth = Attachment{ 
			.internalFormat = GL_DEPTH_COMPONENT24, 
			.format = GL_DEPTH_COMPONENT, 
			.type = GL_FLOAT };
	};

	// @brief An FBO for offscreen rendering
	class FBO
	{
	public:
		explicit FBO(const Descriptor& descriptor);
		~FBO();

		FBO(const FBO&) = delete;
		FBO& operator=(const FBO&) = delete;
		FBO(FBO&&) = delete;
		FBO& operator=(FBO&&) = delete;

		void bind() const;
		void resize(uint32_t width, uint32_t height);

		uint32_t getID() const { return m_fboID; }
		uint32_t getColorTexture(size_t attachment = 0) const { return m_colorTextures.at(attachment); }
		uint32_t getDepthTexture() const { return m_depthTexture; }
		uint32_t getWidth() const { return m_descriptor.width; }
		uint32_t getHeight() const { return m_descriptor.height; }

	private:
		void _create();
		void _destroy();

		uint32_t m_fboID = 0;
		std::vector<uint32_t> m_colorTextures;
		uint32_t m_depthTexture = 0;
		Descriptor m_descriptor;
	};
} // namespace SE
