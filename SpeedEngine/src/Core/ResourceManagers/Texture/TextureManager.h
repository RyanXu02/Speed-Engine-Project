#pragma once
#include "../ResourceManager.h"

namespace SE
{
    class TextureManager : public ResourceManager
    {
    public:
        TextureManager() { m_name = "TextureManager"; }
		~TextureManager() override = default;
		
        void init() override;
        void shutdown() override;

    private:
		std::unordered_map<uint32_t, std::string> m_textures;
    };
}

