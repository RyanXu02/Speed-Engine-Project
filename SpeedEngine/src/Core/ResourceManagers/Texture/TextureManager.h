#pragma once
#include "../ResourceManager.h"
#include "Texture.h"

namespace SE
{
    class TextureManager : public ResourceManager
    {
    public:
        TextureManager() { m_name = "TextureManager"; }
		~TextureManager() override = default;
		
        void init() override;
        void shutdown() override;

        uint32_t loadTexture(const std::string& filePath);
		void unloadTexture(uint32_t textureId);

    private:
		std::unordered_map<uint32_t, std::unique_ptr<Texture>> m_textures;
    };
}

