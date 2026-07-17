#include "pch.h"
#include "FBO.h"

#include "glad/gl.h"

namespace SE
{
	FBO::FBO(uint32_t width, uint32_t height, bool includeDepth)
		: m_width(width), m_height(height), m_includeDepth(includeDepth)
	{
		_create();
	}

	FBO::~FBO()
	{
		_destroy();
	}

	FBO::FBO(FBO&& other) noexcept
		: m_fboID(other.m_fboID), m_colorTexture(other.m_colorTexture), m_depthTexture(other.m_depthTexture),
		m_width(other.m_width), m_height(other.m_height), m_includeDepth(other.m_includeDepth)
	{
		other.m_fboID = 0;
		other.m_colorTexture = 0;
		other.m_depthTexture = 0;
		other.m_width = 0;
		other.m_height = 0;
		other.m_includeDepth = true;
	}
	FBO& FBO::operator=(FBO&& other) noexcept
	{
		if (this != &other)
		{
			_destroy();
			m_fboID = other.m_fboID;
			m_colorTexture = other.m_colorTexture;
			m_depthTexture = other.m_depthTexture;
			m_width = other.m_width;
			m_height = other.m_height;
			m_includeDepth = other.m_includeDepth;
			other.m_fboID = 0;
			other.m_colorTexture = 0;
			other.m_depthTexture = 0;
			other.m_width = 0;
			other.m_height = 0;
			other.m_includeDepth = true;
		}
		return *this;
	}

	void FBO::bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
		glViewport(0, 0, m_width, m_height);
	}
	void FBO::unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FBO::resize(uint32_t width, uint32_t height)
	{
		if (width != m_width || height != m_height)
		{
			m_width = width;
			m_height = height;
			_destroy();
			_create();
		}
	}

	void FBO::_create()
	{
		// create frame buffer
		glGenFramebuffers(1, &m_fboID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

		// create color texture
		glGenTextures(1, &m_colorTexture);
		glBindTexture(GL_TEXTURE_2D, m_colorTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
		
		// depth if flag
		if (m_includeDepth)
		{
			glGenTextures(1, &m_depthTexture);
			glBindTexture(GL_TEXTURE_2D, m_depthTexture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
		}

		// Check framebuffer completeness
		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			// Framebuffer is not complete - could log error here
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void FBO::_destroy()
	{
		if (m_colorTexture)
		{
			glDeleteTextures(1, &m_colorTexture);
			m_colorTexture = 0;
		}
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
}
