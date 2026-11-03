#pragma once
#include "Core/Logger/LoggerProxy.h"
#include "Core/ResourceManagers/Resource.h"

namespace SE
{
	class Shader;

	enum class TextureType
	{
		None = -1,
		Albedo,
		Normal,
		Height,
		AmbOcc,
		Roughness
	};

	struct TextureBinding
	{
		TextureType usage;
		uint32_t textureId; // not openGL texture id
	};

	// @brief Represents a material that can be applied to 3D objects in the scene
	// A material consists of a shader and a set of textures that define its
	// appearance.
	class Material : public Resource
	{
	public:
		Material(uint32_t id, std::string_view resourceName, Logger& logger)
		    : Resource(id, std::string(resourceName)),
		      m_logger(logger, fmt::format("Material:{}", resourceName))
		{}

		// @brief Initializes a material with a shader and a list of textures
		// @param shaderId The ID of the shader to use for this material
		// @param textures Material usage and ID of textures that are already loaded into ResourceManager
		// @returns true if the material was successfully initialized, false otherwise
		bool init(uint32_t shaderId, std::span<const TextureBinding> textures);

		// @brief Binds the material's shader and textures for rendering
		void bind();
		void unbind();

	private:
		// @brief Binds a texture to a slot and sets the corresponding uniform in the shader
		// @param slot The texture slot to bind the texture to
		// @param uniformName The name of the uniform in the shader
		// @param textureId The resourceID of the texture to bind
		void _bindTexture(unsigned int slot, std::string_view uniformName, uint32_t textureId);

		std::vector<TextureBinding> m_textures;
		uint32_t m_shaderId{ 0 };

		LoggerProxy m_logger;
	};
} // namespace SE
