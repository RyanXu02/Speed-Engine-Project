#include "pch.h"

#include "Material.h"

#include "../ResourceManager.h"
#include "../Shader/ShaderManager.h"
#include "Engine.h"

#include <glad/glad.h>

namespace SE
{
	bool Material::init(uint32_t shaderId, std::span<const std::pair<TextureType, std::string_view>> textures)
	{
		m_shaderId = shaderId;
		
		m_isValid = textures.empty();	// every slot binds the shared fallback texture

		for (const auto& [type, filePath] : textures)
		{
			if (addTexture(type, filePath))
				m_isValid = true;
		}
		if (m_isValid)
		{
			m_logger.info("Successfully initialized");
			return true;
		}
		else
		{
			m_logger.warn("Failed to initialize");
			return false;
		}
	}

	void Material::destroy()
	{
		for (auto& tex : m_textures)
		{
			tex.get()->destroy();
		}
	}

	bool Material::addTexture(TextureType type, std::string_view filePath)
	{
		auto newTexture = std::make_unique<Texture>(type);
		std::string msg = newTexture.get()->init(filePath);
		if (msg != "")
		{ // texture failed to load
			m_logger.warn(msg);
			return false;
		}
		m_textures.push_back(std::move(newTexture));
		return true;
	}

	// slot 0 = albedo, slot 1 = normal, slot 2 = height, slot 3 = ambient
	// occlusion, slot 4 = roughness, slot 5 = emissive, slot 6 = metallic
	// slots without a texture get the shared 1x1 fallback texture
	void Material::bind()
	{
		std::array<Texture*, NUM_TEXTURE_TYPES> slots{};
		for (auto& texture : m_textures)
		{
			if (texture->isLoaded()) {
				slots[static_cast<size_t>(texture->getType())] = texture.get();
			}
		}

		for (size_t slot = 0; slot < slots.size(); ++slot)
		{
			Texture* texture = slots[slot] ? slots[slot] : &Texture::getDefault(static_cast<TextureType>(slot));
			_bindTexture(static_cast<unsigned int>(slot), *texture);
		}
	}

	void Material::unbind()
	{
		for (unsigned int slot = 0; slot < static_cast<unsigned int>(NUM_TEXTURE_TYPES); ++slot)
		{
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void Material::_bindTexture(unsigned int slot, Texture& tex)
	{
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, tex.glTextureId);
		Shader* s = Engine::Instance().getSubSystem<ResourceManager>()->getResource<Shader>(m_shaderId);
		if (!s)
		{
			m_logger.critical("Slot {} failed to bind in shader (ID = {})", slot, m_shaderId);
			return;
		}
		s->setInt(tex.getVarName(), slot);
	}

} // namespace SE
