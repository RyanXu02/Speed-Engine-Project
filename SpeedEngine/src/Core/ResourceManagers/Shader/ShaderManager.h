#pragma once
#include "../Manager.h"
#include "Shader.h"

namespace SE
{

    //encapsulates a shader program (vert/frag paths + shader obj)
	struct ShaderProgram
	{
        std::pair<std::string, std::string> sm_shaderPaths;
		std::unique_ptr<Shader> sm_shaderPtr;
	};

    class ShaderManager : public Manager
    {
    public:
        ShaderManager(const std::string& name = "ShaderManager") : Manager(ResourceType::Shader, name) {}
        ~ShaderManager() override = default;

        void init() override;
        void shutdown() override;

        //@brief adds a shader to manager
        //@param id the id assigned to the shader
        //@param vertexShaderPath file path to the vertex shader
        //@param fragmentShaderPath file path to the fragment shader
        //@param programName name of the shader object (for readability in logs)
        //@returns true if shader successfully added, otherwise false
        bool addShader(uint32_t id, 
            std::string_view vertexShaderPath,
            std::string_view fragmentShaderPath,
            std::string_view programName);

        //@brief retrieves a shader from manager given an id
        //@param shaderId the id of shader to retrieve
        //@returns pointer to shader obj, nullptr if not found
        const Shader* getShader(uint32_t shaderId);
    private:
		// id -> ShaderProgram
        std::unordered_map<uint32_t, ShaderProgram> m_shaders;
		
		std::pair<std::string, std::string> _verifyShaderPaths(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
        
    };
    
}

