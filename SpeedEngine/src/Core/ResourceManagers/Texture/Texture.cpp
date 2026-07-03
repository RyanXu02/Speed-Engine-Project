#include "pch.h"
#include "Texture.h"
#include "glad/gl.h"

namespace SE 
{
	Texture::Texture(uint32_t id, std::string name) : m_resourceId(id), Resource(name) {}

	Texture::~Texture() {

	}

	void Texture::init() {

	}

	void Texture::bind(unsigned int slot) const {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, m_textureId);
	}

	void Texture::unbind() const {
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	//@brief deletes texture data
	void Texture::destroy() {
		glDeleteTextures(1, &m_textureId);
	}
}