#pragma once

#include "Core/ResourceManagers/Material/Material.h"
#include "Core/ResourceManagers/Shader/Shader.h"
#include <array>

namespace SE
{
	class ResourceManager;
	struct ShaderLoadDesc;
	class LoggerProxy;

	// @brief Owns uploaded GPU assets (shaders, textures, material bindings) shared by rendering passes
	class AssetGPUCache
	{
	public:
		Shader* shader(ResourceManager& resources, ResourceID id, LoggerProxy& logger);
		unsigned int texture(ResourceManager& resources, ResourceID id);
		void bindMaterial(ResourceManager& resources, const Material& material, Shader& shader, size_t offset = 0);
		static void unbindMaterial(size_t offset = 0);
		void clear();

	private:
		unsigned int fallback(TextureType type);
		std::unordered_map<ResourceID, std::unique_ptr<Shader>> m_shaders;
		std::unordered_map<ResourceID, unsigned int> m_textures;
		std::array<unsigned int, NUM_TEXTURE_TYPES> m_fallbacks{};
	};

	GLCompStatus loadStartupShader(Shader& shader, ResourceManager& resources, const ShaderLoadDesc& desc);
}