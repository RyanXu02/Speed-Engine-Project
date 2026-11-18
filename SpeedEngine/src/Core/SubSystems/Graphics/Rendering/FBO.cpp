#include "pch.h"

#include "FBO.h"

#include <glad/glad.h>

namespace SE
{
	FBO::FBO(const Descriptor& descriptor) : m_descriptor(descriptor)
	{
		_create();
	}

	FBO::~FBO()
	{
		_destroy();
	}

	void FBO::bind() const
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
	}

	void FBO::resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0)
			throw std::invalid_argument("Framebuffer dimensions must be nonzero");

		if (width != getWidth() || height != getHeight())
		{
			m_descriptor.width = width;
			m_descriptor.height = height;
			_destroy();
			_create();
		}
	}

	void FBO::_create()
	{
		if (getWidth() == 0 || getHeight() == 0)
			throw std::invalid_argument("Framebuffer dimensions must be nonzero");

		GLint maxColors = 0;
		GLint maxDrawBuffers = 0;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColors);
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);
		if (m_descriptor.colors.size() > static_cast<size_t>(std::min(maxColors, maxDrawBuffers)))
			throw std::invalid_argument("Framebuffer color attachment count exceeds OpenGL limits");

		GLint previousDraw = 0;
		GLint previousRead = 0;
		GLint previousTexture = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousDraw);
		glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousRead);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);

		m_colorTextures.resize(m_descriptor.colors.size());
		std::vector<GLenum> drawBuffers(m_colorTextures.size());
		glGenFramebuffers(1, &m_fboID);
		if (!m_fboID)
			throw std::runtime_error("Failed to create OpenGL framebuffer");
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

		auto createAttachment = [&](uint32_t& texture, const Attachment& attachment, GLenum point)
		{
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, attachment.internalFormat, getWidth(), getHeight(), 0, attachment.format, attachment.type, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, attachment.minFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, attachment.magFilter);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, attachment.wrapS);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, attachment.wrapT);
			glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, attachment.borderColor.data());
			glFramebufferTexture2D(GL_FRAMEBUFFER, point, GL_TEXTURE_2D, texture, 0);
		};

		for (size_t i = 0; i < m_colorTextures.size(); ++i)
		{
			drawBuffers[i] = GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(i);
			createAttachment(m_colorTextures[i], m_descriptor.colors[i], drawBuffers[i]);
		}
		if (m_descriptor.depth)
			createAttachment(m_depthTexture, *m_descriptor.depth, GL_DEPTH_ATTACHMENT);

		if (drawBuffers.empty())
		{
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}
		else
		{
			glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
			glReadBuffer(GL_COLOR_ATTACHMENT0);
		}

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		glBindTexture(GL_TEXTURE_2D, previousTexture);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, previousDraw);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, previousRead);

		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			_destroy();
			throw std::runtime_error(std::format("Framebuffer {}x{} ({} colors, depth={}): incomplete, OpenGL status 0x{:X}",
				getWidth(), getHeight(), m_descriptor.colors.size(), m_descriptor.depth.has_value(), status));
		}
	}

	void FBO::_destroy()
	{
		glDeleteTextures(static_cast<GLsizei>(m_colorTextures.size()), m_colorTextures.data());
		m_colorTextures.clear();
		if (m_depthTexture)
		{
			glDeleteTextures(1, &m_depthTexture);
			m_depthTexture = 0;
		}
		if (m_fboID)
		{
			glDeleteFramebuffers(1, &m_fboID);
			m_fboID = 0;
		}
	}
} // namespace SE
