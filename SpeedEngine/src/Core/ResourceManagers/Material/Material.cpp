#include "pch.h"

#include "Material.h"

#include "../ResourceManager.h"
#include "../Shader/ShaderManager.h"
#include "Engine.h"

#include <glad/glad.h>

namespace SE
{

	bool Material::init(uint32_t shaderId, std::span<const TextureBinding> textures)
	{
		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		for (const auto& binding : textures)
		{
			if (binding.usage == TextureType::None || !resources->getResource<Texture>(binding.textureId))
			{
				m_logger.warn("Invalid texture binding: {}", binding.textureId);
				return false;
			}
		}
		if (textures.empty())
		{
			m_logger.warn("Failed to initialize");
			return false;
		}

		m_shaderId = shaderId;
		m_textures.assign(textures.begin(), textures.end());
		m_logger.info("Successfully initialized");
		return true;
	}

	// slot 0 = albedo, slot 1 = normal, slot 2 = height, slot 3 = ambient
	// occlusion, slot 4 = roughness
	void Material::bind()
	{
		for (const auto& binding : m_textures)
		{
			switch (binding.usage)
			{
			case TextureType::Albedo: _bindTexture(0, "albedo", binding.textureId); break;
			case TextureType::Normal: _bindTexture(1, "normalmap", binding.textureId); break;
			case TextureType::Height: _bindTexture(2, "heightmap", binding.textureId); break;
			case TextureType::AmbOcc: _bindTexture(3, "ambientOcc", binding.textureId); break;
			case TextureType::Roughness: _bindTexture(4, "roughness", binding.textureId); break;
			default: assert(false); break;
			}
		}
	}

	void Material::unbind()
	{
		for (unsigned int slot = 0; slot < 5; ++slot)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void Material::_bindTexture(unsigned int slot, std::string_view uniformName, uint32_t textureId)
	{
		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		auto* texture = resources->getResource<Texture>(textureId);
		assert(texture);
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, texture->getTextureId());
		Shader* s = resources->getResource<Shader>(m_shaderId);
		if (!s)
		{
			m_logger.critical("Slot {} failed to bind in shader (ID = {})", slot, m_shaderId);
			return;
		}
		s->setInt(uniformName, slot);
	}

} // namespace SE
