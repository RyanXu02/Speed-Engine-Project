#include "pch.h"
#include "Material.h"
#include "../Shader/ShaderManager.h"
#include "../ResourceManager.h"

#include "glad/gl.h"

namespace SE
{


	bool Material::init(uint32_t shaderId, std::initializer_list<std::pair<TextureType, std::string_view>> textures) {
		m_shaderId = shaderId;

		for (const auto& [type, filePath] : textures) {
			if (addTexture(type, filePath))
				m_isValid = true;
		}
		if (m_isValid) {
			m_logger.info("Successfully initialized");
			return true;
		}
		else {
			m_logger.warn("Failed to initialize");
			return false;
		}
	}

	void Material::destroy() {
		for (auto& tex : m_textures) {
			tex.get()->destroy();
		}
	}

	bool Material::addTexture(TextureType type, std::string_view filePath) {
		auto newTexture = std::make_unique<Texture>(type);
		std::string msg = newTexture.get()->init(filePath);
		if (msg != "") { //texture failed to load
			m_logger.warn(msg);
			return false;
		}
		m_textures.push_back(std::move(newTexture));
		return true;
	}

	//slot 0 = albedo, slot 1 = normal, slot 2 = height, slot 3 = ambient occlusion, slot 4 = roughness
	void Material::bind() {
		for (auto& t : m_textures) {
			if (!t->isLoaded()) continue;
			switch (t->getType()) {
			case TextureType::Albedo:
				_bindTexture(0, *t);
				break;
			case TextureType::Normal:
				_bindTexture(1, *t);
				break;
			case TextureType::Height:
				_bindTexture(2, *t);
				break;
			case TextureType::AmbOcc:
				_bindTexture(3, *t);
				break;
			case TextureType::Roughness:
				_bindTexture(4, *t);
				break;
			}
		}
	}

	void Material::unbind() {
		for (unsigned int slot = 0; slot < 5; ++slot) {
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void Material::_bindTexture(unsigned int slot, Texture& tex) {
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, tex.glTextureId);
		Shader* s = ResourceManager::Instance().getResource<Shader>(m_shaderId);
		if (!s) {
			m_logger.critical("Slot {} failed to bind in shader (ID = {}", slot, m_shaderId);
			return;
		}
		s->setInt(tex.getVarName(), slot);
	}
	

}