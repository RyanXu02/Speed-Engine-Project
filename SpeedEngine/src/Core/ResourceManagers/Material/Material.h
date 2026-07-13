#pragma once
#include "../Resource.h"
#include "../../Logger/LoggerProxy.h"
#include "Texture.h"

namespace SE
{
    class Shader;

    class Material : public Resource
    {
    public:
        Material(uint32_t id, std::string_view resourceName, Logger& logger) : Resource(id, std::string(resourceName)), 
            m_logger(logger, fmt::format("Material:{}", resourceName)) {}
		
        bool init(uint32_t shaderId, std::span<const std::pair<TextureType, std::string_view>> textures);
    
        void destroy();

        bool addTexture(TextureType type, std::string_view filePath);

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

