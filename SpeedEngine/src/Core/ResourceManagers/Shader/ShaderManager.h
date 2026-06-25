#pragma once
#include "../ResourceManager.h"
#include "Shader.h"

namespace SE
{
    class ShaderManager : public ResourceManager
    {
    public:
        ShaderManager() { m_name = "ShaderManager"; }
        ~ShaderManager() override = default;

        void init() override;
        void shutdown() override;

        uint32_t addShader(const std::string& filePath);
        void unloadShader(uint32_t shaderId);

    private:
        // TODO: shared ptr of shaders? What to do with ids????
		std::unordered_map<std::string, uint32_t> m_shaderFilePathToId;
        std::unordered_map<uint32_t, std::unique_ptr<Shader>> m_shaders;
    };
}

