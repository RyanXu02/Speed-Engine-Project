#pragma once
#include "../../SubSystems/SubSystem.h"
#include "Shader.h"

namespace SE
{

	struct ShaderProgram
	{
        std::pair<std::string, std::string> sm_shaderPaths;
		std::unique_ptr<Shader> sm_shaderPtr;
	};

    class ShaderManager : public SubSystem
    {
    public:
        ShaderManager() { m_name = "ShaderManager"; }
        ~ShaderManager() override = default;

        void init() override;
        void shutdown() override;

    protected:


    private:
		// id -> ShaderProgram
		std::unordered_map<uint32_t, ShaderProgram> m_shaders;
		
		std::pair<std::string, std::string> _verifyShaderPaths(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
        
        uint32_t _addShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
        const Shader* _getShader(uint32_t shaderId);
    };
    
}

