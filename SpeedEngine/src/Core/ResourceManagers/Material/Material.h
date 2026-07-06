#pragma once
#include "../Resource.h"
#include "Texture.h"

namespace SE
{
    class Shader;

    class Material : public Resource
    {
    public:
        Material(uint32_t id, std::string resourceName) : Resource(id, resourceName) {}
		
        void init(Shader* shader, std::initializer_list<std::pair<TextureType, std::string_view>> textures);
    
        void destroy();

        bool addTexture(TextureType type, std::string_view filePath);

        void bind();
        void unbind();

    private:
        void _bindTexture(unsigned int slot, Texture& tex);

        uint32_t m_resourceId;
		std::vector<std::unique_ptr<Texture>> m_textures;
        Shader* m_shader{ nullptr };
        bool m_isValid{ false };
    };
}

