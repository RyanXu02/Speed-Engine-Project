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
        ShaderManager(const std::string& name) : SubSystem(name) {}
        ~ShaderManager() override = default;

        void init() override;
        void shutdown() override;

    private:
		// id -> ShaderProgram
		std::unordered_map<uint32_t, ShaderProgram> m_shaders;
		
		std::pair<std::string, std::string> _verifyShaderPaths(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
        
        bool _addShader(uint32_t id, std::string_view vertexShaderPath, std::string_view fragmentShaderPath, std::string_view programName);
        const Shader* _getShader(uint32_t shaderId);
    };
    
}

