#pragma once
#include "../Resource.h"
#include "../../Logger/LoggerProxy.h"
#include "Texture.h"

namespace SE
{
    class Shader;

	// @brief Represents a material that can be applied to 3D objects in the scene
	// A material consists of a shader and a set of textures that define its appearance.
    class Material : public Resource
    {
    public:
        Material(uint32_t id, std::string_view resourceName, Logger& logger) : Resource(id, std::string(resourceName)), 
            m_logger(logger, fmt::format("Material:{}", resourceName)) {}
		
        // @brief Initializes a material with a shader and a list of textures
		// @param shaderId The ID of the shader to use for this material
		// @param textures A span of pairs, each containing a TextureType and the corresponding file path for the texture
        bool init(uint32_t shaderId, std::span<const std::pair<TextureType, std::string_view>> textures);
    
        void destroy();

		// @brief Adds a texture to the material
		// @param type The type of the texture (Albedo, Normal, Height, AmbOcc, Roughness)
        // @param filePath The file path of the texture file
        bool addTexture(TextureType type, std::string_view filePath);

		// @brief Binds the material's shader and textures for rendering
        void bind();
        void unbind();

    private:
        void _bindTexture(unsigned int slot, Texture& tex);

        bool m_isValid{ false };
		std::vector<std::unique_ptr<Texture>> m_textures;
        uint32_t m_shaderId{ 0 };

        LoggerProxy m_logger;
    };
}

