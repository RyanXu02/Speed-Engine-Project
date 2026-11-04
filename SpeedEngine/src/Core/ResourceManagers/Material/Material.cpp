#include "pch.h"

#include "Material.h"

#include "../ResourceManager.h"
#include "../Shader/ShaderManager.h"
#include "Engine.h"

#include <glad/glad.h>

namespace SE
{
	// sampler names in the shaders, indexed by TextureType
	static constexpr std::array<std::string_view, NUM_TEXTURE_TYPES> SAMPLER_NAMES = {
		"albedoMap",
		"normalMap",
		"heightMap",
		"aoMap",
		"roughnessMap",
		"emissiveMap",
		"metallicMap",
	};

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

		// no textures is fine, every slot binds the shared fallback texture
		m_shaderId = shaderId;
		m_textures.assign(textures.begin(), textures.end());
		m_logger.info("Successfully initialized");
		return true;
	}

	// slot 0 = albedo, slot 1 = normal, slot 2 = height, slot 3 = ambient
	// occlusion, slot 4 = roughness, slot 5 = emissive, slot 6 = metallic
	// slots without a texture get the shared 1x1 fallback texture
	void Material::bind()
	{
		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();

		std::array<unsigned int, NUM_TEXTURE_TYPES> slots{};
		for (const auto& binding : m_textures)
		{
			if (auto* texture = resources->getResource<Texture>(binding.textureId)) {
				slots[static_cast<size_t>(binding.usage)] = texture->getTextureId();
			}
		}

		for (size_t slot = 0; slot < slots.size(); ++slot)
		{
			const unsigned int glTextureId = slots[slot] ? slots[slot] : _getDefaultTexture(static_cast<TextureType>(slot));
			_bindTexture(static_cast<unsigned int>(slot), SAMPLER_NAMES[slot], glTextureId);
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

	void Material::destroyDefaults()
	{
		for (auto& texture : s_defaultTextures)
		{
			if (texture) {
				glDeleteTextures(1, &texture);
			}
			texture = 0;
		}
	}

	void Material::_bindTexture(unsigned int slot, std::string_view uniformName, unsigned int glTextureId)
	{
		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		glActiveTexture(GL_TEXTURE0 + slot);
		glBindTexture(GL_TEXTURE_2D, glTextureId);
		Shader* s = resources->getResource<Shader>(m_shaderId);
		if (!s)
		{
			m_logger.critical("Slot {} failed to bind in shader (ID = {})", slot, m_shaderId);
			return;
		}
		s->setInt(uniformName, slot);
	}

	// return the default fallback texture based on the type enum
	unsigned int Material::_getDefaultTexture(TextureType type)
	{
		unsigned int& texture = s_defaultTextures[static_cast<size_t>(type)];
		if (texture)
			return texture;

		// flat normal for the normal map, black for height, white for the rest so the factors pass through
		std::array<unsigned char, 4> pixel{ 255, 255, 255, 255 };
		if (type == TextureType::Normal)
			pixel = { 128, 128, 255, 255 };
		else if (type == TextureType::Height)
			pixel = { 0, 0, 0, 255 };

		GLint previousTexture = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel.data());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glBindTexture(GL_TEXTURE_2D, previousTexture);
		return texture;
	}

} // namespace SE
