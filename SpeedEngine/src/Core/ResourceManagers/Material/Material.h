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
		Roughness,
		Emissive,
		Metallic
	};

	// number of texture types, remember to update when adding a new TextureType
	constexpr size_t NUM_TEXTURE_TYPES = static_cast<size_t>(TextureType::Metallic) + 1;

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
		// @param textures Material usage and ID of textures that are already loaded into ResourceManager,
		// can be empty, every missing slot binds the shared fallback texture
		// @returns true if the material was successfully initialized, false otherwise
		bool init(uint32_t shaderId, std::span<const TextureBinding> textures);

		// @brief Binds the material's shader and textures for rendering
		void bind();
		void unbind();

		// @brief Deletes the shared 1x1 fallback textures from GPU
		static void destroyDefaults();

	private:
		// @brief Binds a texture to a slot and sets the corresponding uniform in the shader
		// @param slot The texture slot to bind the texture to
		// @param uniformName The name of the uniform in the shader
		// @param glTextureId The OpenGL texture to bind
		void _bindTexture(unsigned int slot, std::string_view uniformName, unsigned int glTextureId);

		// @brief Gets the shared 1x1 fallback texture bound when a material has no texture of this type
		// @returns the OpenGL texture id
		static unsigned int _getDefaultTexture(TextureType type);

		// default fallback textures for objects that don't have the full pbr material set
		static inline std::array<unsigned int, NUM_TEXTURE_TYPES> s_defaultTextures{};

		std::vector<TextureBinding> m_textures;
		uint32_t m_shaderId{ 0 };

		LoggerProxy m_logger;
	};
} // namespace SE
