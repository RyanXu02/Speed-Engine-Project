#pragma once
#include "../Manager.h"
#include "Texture.h"

namespace SE
{
    class TextureManager : public Manager
    {
    public:
		TextureManager(const std::string& name = "TextureManager") : Manager(ResourceType::Texture, name) {}
		~TextureManager() override = default;
		
        void init() override;
        void shutdown() override;

        //@brief loads a texture
        //@param id
        uint32_t loadTexture(uint32_t id,
            std::optional<std::string_view> albedoPath,
            std::optional<std::string_view> heightPath,
            std::optional<std::string_view> normalPath
            //std::optional<std::string_view> roughnessPath,
            //std::optional<std::string_view> metallicPath,
            //std::optional<std::string_view> aoPath
        ) {
            return 0;
        };
        void unloadTexture(uint32_t textureId) {};

    private:
		std::unordered_map<uint32_t, std::unique_ptr<Texture>> m_textures;
    };
}

