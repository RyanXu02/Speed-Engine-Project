#include "pch.h"
#include "ShaderManager.h"
#include "../../Logger/Logger.h"
#include "../../ResourceManagers/ResourceManager.h"

namespace SE
{
	void ShaderManager::init()
	{
		Manager::init();

		m_shaders.reserve(64);
	}

	void ShaderManager::shutdown()
	{
		Manager::shutdown();
		
		for (auto& [k, shaderProgram] : m_shaders)
		{
			if (shaderProgram.sm_shaderPtr)
			{
				shaderProgram.sm_shaderPtr->destroy();
			}
		}
		m_shaders.clear();

		m_logger->info("ShaderManager cleared all shaders.");
	}

	//@brief adds a shader to be managed by the ShaderManager. if a shader of programName already exists, returns id of existing shader
	//@param id id of new shader
	//@param vertexShaderPath path to vertex shader file
	//@param fragmentShaderPath path to fragment shader file
	//@param programName name of shader program to create
	//@returns id of new shader, or id of existing shader if new shader would be a duplicate name
	uint32_t ShaderManager::addShader(uint32_t id, 
		std::string_view vertexShaderPath, 
		std::string_view fragmentShaderPath, 
		std::string_view programName)
	{
		// check if vertex and fragment shader paths are valid
		auto shaderPaths = _verifyShaderPaths(vertexShaderPath, fragmentShaderPath);
		if (shaderPaths.first.empty() || shaderPaths.second.empty())
		{
			m_logger->critical("One or more invalid shader paths: {} and {}", vertexShaderPath, fragmentShaderPath);
			return 0;
		}
		// look for in cache
		auto it = std::ranges::find_if(m_shaders, [programName](const auto& kv) {
			return kv.second.sm_shaderPtr->getResourceName() == programName;
			});
		if (it != m_shaders.end()) {
			m_logger->verbose("Shader with name already loaded: {} with ID {}", it->second.sm_shaderPtr->getResourceName(), it->first);
			return it->first;
		}
		/*auto it = std::ranges::find_if(m_shaders, [shaderPaths](const auto& kv) {
			return kv.second.sm_shaderPaths == shaderPaths;
			});
		if (it != m_shaders.end()) {
			m_logger->verbose("Shader already loaded: {} and {} with ID {}", shaderPaths.first, shaderPaths.second, it->first);
			return it->first;
		}*/

		// Load shader source code from file
		auto shader = std::make_unique<Shader>(id, programName.data());

		std::string vertexSource = ResourceManager::Instance().getString(shaderPaths.first);
		std::string fragmentSource = ResourceManager::Instance().getString(shaderPaths.second);

		GLCompStatus status = shader->init(vertexSource, fragmentSource);
		if (!status.success)
		{
			m_logger->critical("Shader compilation failed for {}: {}", programName, status.log);
			return 0;
		}

		m_shaders[id] = { shaderPaths, std::move(shader) };
		
		m_logger->info("Shader '{}' loaded: ID {}", m_shaders[id].sm_shaderPtr->getResourceName(), id);

		return id;
	}

	std::pair<std::string, std::string> ShaderManager::_verifyShaderPaths(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
	{
		std::pair<std::string, std::string> shaderPaths{"",""};

		//.vert and .frag extensions have to be contained in these variables in some order
		if (!((vertexShaderPath.contains(".vert") && fragmentShaderPath.contains(".frag")) || (vertexShaderPath.contains(".frag") && fragmentShaderPath.contains(".vert"))))
		{
			return shaderPaths;
		}

		//vertexShaderPath contains .vert, meaning user passed in vertex path to this variable
		if (vertexShaderPath.contains(".vert"))
		{
			shaderPaths.first = std::string(vertexShaderPath);
			shaderPaths.second = std::string(fragmentShaderPath);
		}
		else //vertexShaderPath contains .frag meaning user passed in fragment path to this variable, swap in shaderPaths
		{
			shaderPaths.first = std::string(fragmentShaderPath);
			shaderPaths.second = std::string(vertexShaderPath);
		}

		return shaderPaths;
	}

	const Shader* ShaderManager::getShader(uint32_t shaderId)
	{
		auto it = m_shaders.find(shaderId);
		if (it == m_shaders.end()) return nullptr;
		
		return it->second.sm_shaderPtr.get();
	}
}