#include "pch.h"
#include "Material.h"
#include "../Shader/Shader.h"

#include "glad/gl.h"

namespace SE
{


	void Material::init(Shader* shader, std::initializer_list<std::pair<TextureType, std::string_view>> textures) {
		m_shader = shader;
		for (const auto& [type, filePath] : textures) {
			if (addTexture(type, filePath))
				m_isValid = true;
		}
	}

	void Material::destroy() {
		//TODO IMPLEMENT
	}

	bool Material::addTexture(TextureType type, std::string_view filePath) {
		auto newTexture = std::make_unique<Texture>(type);
		//todo: create texture!
		m_textures.push_back(std::move(newTexture));
		return true;
	}

	//slot 0 = albedo, slot 1 = normal, slot 2 = height, slot 3 = ambient occluision, slot 4 = roughness
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
		m_shader->setInt(tex.getVarName(), slot);
	}
	

}