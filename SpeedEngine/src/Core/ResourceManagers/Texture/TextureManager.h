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

        uint32_t loadTexture(const std::string& filePath);
		void unloadTexture(uint32_t textureId);

    private:
		std::unordered_map<uint32_t, std::unique_ptr<Texture>> m_textures;
    };
}

