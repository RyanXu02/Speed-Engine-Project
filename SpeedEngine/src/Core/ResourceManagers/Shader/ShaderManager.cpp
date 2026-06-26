#include "pch.h"
#include "ShaderManager.h"
#include "../../Logger/Logger.h"

namespace SE
{
	void ShaderManager::init()
	{
		ResourceManager::init();
	}

	void ShaderManager::shutdown()
	{
		ResourceManager::shutdown();
		
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

	uint32_t ShaderManager::_addShader(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
	{
		// check if vertex and fragment shader paths are valid
		auto shaderPaths = _verifyShaderPaths(vertexShaderPath, fragmentShaderPath);
		if (shaderPaths.first.empty() || shaderPaths.second.empty())
		{
			m_logger->critical("One or more invalid shader paths: {} and {}", vertexShaderPath, fragmentShaderPath);
			return 0;
		}
		// look for in cache
		auto it = std::ranges::find_if(m_shaders, [shaderPaths](const auto& kv) {
			return kv.second.sm_shaderPaths == shaderPaths;
			});
		if (it != m_shaders.end()) {
			m_logger->verbose("Shader already loaded: {} and {} with ID {}", shaderPaths.first, shaderPaths.second, it->first);
			return it->first;
		}

		// Load shader source code from file
		uint32_t resourceId = generateId();
		auto shader = std::make_unique<Shader>(resourceId);

		std::string vertexSource = ResourceManager::getString(shaderPaths.first);
		std::string fragmentSource = ResourceManager::getString(shaderPaths.second);

		GLCompStatus status = shader->init(vertexSource, fragmentSource);
		if (!status.success)
		{
			m_logger->critical("Shader compilation failed for {}: {}", resourceId, status.log);
			return 0;
		}

		m_shaders[resourceId] = { shaderPaths, std::move(shader) };
		m_logger->info("Shader loaded: ID {}", resourceId);

		return resourceId;
	}

	std::pair<std::string, std::string> ShaderManager::_verifyShaderPaths(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
	{
		std::pair<std::string, std::string> shaderPaths{"",""};

		if (!((vertexShaderPath.contains(".vert") && fragmentShaderPath.contains(".frag")) || (vertexShaderPath.contains(".frag") && fragmentShaderPath.contains(".vert"))))
		{
			return shaderPaths;
		}

		if (vertexShaderPath.contains(".vert"))
		{
			shaderPaths.first = vertexShaderPath.data();
			shaderPaths.second = fragmentShaderPath.data();
		}
		else
		{
			shaderPaths.first = fragmentShaderPath.data();
			shaderPaths.second = vertexShaderPath.data();
		}

		return shaderPaths;
	}

	const Shader* ShaderManager::_getShader(uint32_t shaderId)
	{
		auto it = m_shaders.find(shaderId);
		if (it == m_shaders.end()) return nullptr;
		
		return it->second.sm_shaderPtr.get();
	}
}