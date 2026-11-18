#include "pch.h"
#include "AssetGPUCache.h"
#include "Core/ResourceManagers/ResourceManager.h"
#include "Core/Logger/LoggerProxy.h"
#include <glad/glad.h>

namespace SE
{
	GLCompStatus loadStartupShader(Shader& shader, ResourceManager& resources, const ShaderLoadDesc& desc)
	{
		const auto id = resources.requestShader(desc);
		if (!resources.waitForStartupResource(id))
			return { false, "Shader source unavailable" };
		const auto& source = *resources.getResource<ShaderSource>(id);
		return shader.init(source.vertex, source.fragment);
	}

	Shader* AssetGPUCache::shader(ResourceManager& resources, ResourceID id, LoggerProxy& logger)
	{
		if (const auto i = m_shaders.find(id); i != m_shaders.end())
			return i->second.get();
		const auto* source = resources.getResource<ShaderSource>(id);
		if (!source)
			return nullptr;
		auto program = std::make_unique<Shader>();
		const auto status = source->compute.empty() ? program->init(source->vertex, source->fragment)
			: program->initCompute(source->compute);
		if (!status.success)
		{
			logger.error("GPU shader {} failed: {}", id, status.log);
			program.reset();
		}
		return m_shaders.emplace(id, std::move(program)).first->second.get();
	}

	static unsigned int uploadTexture(int width, int height, int channels, const void* pixels, bool mipmaps, bool hdr = false)
	{
		unsigned int texture = 0;
		glGenTextures(1, &texture);
		if (!texture)
			return 0;
		GLint previousTexture = 0;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &previousTexture);
		const GLenum unpackNames[] = { GL_UNPACK_ALIGNMENT, GL_UNPACK_ROW_LENGTH, GL_UNPACK_SKIP_ROWS,
			GL_UNPACK_SKIP_PIXELS, GL_PIXEL_UNPACK_BUFFER_BINDING };
		GLint unpack[5]{};
		for (size_t i = 0; i < std::size(unpackNames); ++i)
			glGetIntegerv(unpackNames[i], &unpack[i]);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		for (size_t i = 1; i < 4; ++i)
			glPixelStorei(unpackNames[i], 0);
		const GLenum formats[] = { GL_RED, GL_RG, GL_RGB, GL_RGBA };
		const GLenum internalFormats[] = { GL_R8, GL_RG8, GL_RGB8, GL_RGBA8 };
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, hdr ? GL_RGB32F : internalFormats[channels - 1], width, height, 0,
			formats[channels - 1], hdr ? GL_FLOAT : GL_UNSIGNED_BYTE, pixels);
		if (mipmaps)
			glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mipmaps ? GL_LINEAR : GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glBindTexture(GL_TEXTURE_2D, previousTexture);
		for (size_t i = 0; i < 4; ++i)
			glPixelStorei(unpackNames[i], unpack[i]);
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, unpack[4]);
		return texture;
	}

	unsigned int AssetGPUCache::texture(ResourceManager& resources, ResourceID id)
	{
		if (const auto i = m_textures.find(id); i != m_textures.end())
			return i->second;
		const auto* data = resources.getResource<TextureData>(id);
		if (!data)
			return 0;
		const bool hdr = !data->hdrPixels.empty();
		const void* pixels = hdr ? static_cast<const void*>(data->hdrPixels.data()) : data->pixels.data();
		const auto texture = uploadTexture(data->width, data->height, data->channels, pixels, true, hdr);
		if (texture)
			m_textures.emplace(id, texture);
		return texture;
	}

	unsigned int AssetGPUCache::fallback(TextureType type)
	{
		auto& texture = m_fallbacks[static_cast<size_t>(type)];
		if (texture)
			return texture;
		std::array<unsigned char, 4> pixel{ 255, 255, 255, 255 };
		if (type == TextureType::Normal)
			pixel = { 128, 128, 255, 255 };
		else if (type == TextureType::Height)
			pixel = { 0, 0, 0, 255 };
		return texture = uploadTexture(1, 1, 4, pixel.data(), false);
	}

	void AssetGPUCache::bindMaterial(ResourceManager& resources, const Material& material, Shader& shader, size_t offset)
	{
		static constexpr std::array<std::string_view, NUM_TEXTURE_TYPES * 3> names{
			"albedoMap", "normalMap", "heightMap", "aoMap", "roughnessMap", "emissiveMap", "metallicMap",
			"albedoMap1", "normalMap1", "heightMap1", "aoMap1", "roughnessMap1", "emissiveMap1", "metallicMap1",
			"albedoMap2", "normalMap2", "heightMap2", "aoMap2", "roughnessMap2", "emissiveMap2", "metallicMap2"
		};
		std::array<unsigned int, NUM_TEXTURE_TYPES> slots{};
		for (const auto& binding : material.textures)
			slots[static_cast<size_t>(binding.usage)] = texture(resources, binding.textureId);
		for (size_t i = 0; i < slots.size(); ++i)
		{
			const auto slot = static_cast<unsigned int>(offset * NUM_TEXTURE_TYPES + i);
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, slots[i] ? slots[i] : fallback(static_cast<TextureType>(i)));
			shader.setInt(names[slot], slot);
		}
	}

	void AssetGPUCache::unbindMaterial(size_t offset)
	{
		for (size_t i = 0; i < NUM_TEXTURE_TYPES; ++i)
		{
			glActiveTexture(GL_TEXTURE0 + static_cast<unsigned int>(offset * NUM_TEXTURE_TYPES + i));
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void AssetGPUCache::clear()
	{
		m_shaders.clear();
		for (const auto& [id, texture] : m_textures)
			glDeleteTextures(1, &texture);
		m_textures.clear();
		glDeleteTextures(static_cast<GLsizei>(m_fallbacks.size()), m_fallbacks.data());
		m_fallbacks.fill(0);
	}
}