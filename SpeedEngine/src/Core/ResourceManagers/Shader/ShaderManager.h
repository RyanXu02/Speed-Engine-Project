#pragma once
#include "../Manager.h"
#include "Shader.h"

namespace SE
{
    class ShaderManager : public Manager
    {
		friend class ResourceManager;

    public:
        ShaderManager(const std::string& name = "ShaderManager") : Manager(ResourceType::Shader, name) {}
        ~ShaderManager() override = default;

        void init() override;
        void shutdown() override;

    private:
        //@brief adds a shader to manager
        //@param id the id assigned to the shader
        //@param vertexShaderPath file path to the vertex shader
        //@param fragmentShaderPath file path to the fragment shader
        //@param programName name of the shader object (for readability in logs)
        //@returns true if shader successfully added, otherwise false
        uint32_t addShader(uint32_t id, 
            std::string_view vertexShaderPath,
            std::string_view fragmentShaderPath,
            std::string_view programName);

        //@brief retrieves a shader from manager given an id
        //@param shaderId the id of shader to retrieve
        //@returns pointer to shader obj, nullptr if not found
        Shader* getShader(uint32_t shaderId);

        //@brief removes a shader from manager given an id
        //@param shaderId the id of shader to remove
        //@returns true if successfully removed, false if not
        bool removeShader(uint32_t shaderId);

		// map ids -> ShaderPrograms
        std::unordered_map<uint32_t, std::unique_ptr<Shader>> m_shaders;
		
		// @brief helper fn that verifies that the vertex and fragment shader paths are valid
		std::pair<std::string, std::string> _verifyShaderPaths(std::string_view vertexShaderPath, std::string_view fragmentShaderPath);
        
    };
    
}

