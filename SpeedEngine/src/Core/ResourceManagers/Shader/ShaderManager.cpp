#include "pch.h"

#include "ShaderManager.h"

#include "../../Logger/Logger.h"
#include "../../ResourceManagers/ResourceManager.h"
#include "Engine.h"

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

		m_shaders.clear();
		m_shaderPaths.clear();

		m_logger->info("ShaderManager cleared all shaders.");
	}

	//@brief loads a shader, reusing an existing shader with the same source paths
	//@param id id of new shader
	//@param vertexShaderPath path to vertex shader file
	//@param fragmentShaderPath path to fragment shader file
	//@param programName name of shader program to create
	//@returns an existing or new shader ID, or 0 on failure
	uint32_t ShaderManager::addShader(uint32_t id, std::string_view vertexShaderPath, std::string_view fragmentShaderPath, std::string_view programName)
	{
		// check if vertex and fragment shader paths are valid
		auto shaderPaths = _verifyShaderPaths(vertexShaderPath, fragmentShaderPath);
		if (shaderPaths.first.empty() || shaderPaths.second.empty())
		{
			m_logger->critical("One or more invalid shader paths: {} and {}", vertexShaderPath, fragmentShaderPath);
			return 0;
		}
		if (auto it = m_shaderPaths.find(shaderPaths); it != m_shaderPaths.end())
		{
			assert(m_shaders.contains(it->second));
			return it->second;
		}

		// Load shader source code from file
		auto shader = std::make_unique<Shader>(id, std::string(programName));

		auto* resources = Engine::Instance().getSubSystem<ResourceManager>();
		const std::string* vertexSource = resources->getString(shaderPaths.first);
		const std::string* fragmentSource = resources->getString(shaderPaths.second);
		if (!vertexSource || !fragmentSource)
		{
			resources->forgetShaderSources(shaderPaths);
			return 0;
		}

		GLCompStatus status = shader->init(*vertexSource, *fragmentSource);
		if (!status.success)
		{
			m_logger->critical("Shader compilation failed for {}: {}", programName, status.log);
			resources->forgetShaderSources(shaderPaths);
			return 0;
		}

		m_shaders.try_emplace(id, std::move(shader));
		m_shaderPaths.emplace(shaderPaths, id);
		assert(m_shaderPaths.size() == m_shaders.size());

		m_logger->info("Shader '{}' loaded: ID {}", m_shaders[id]->getResourceName(), id);

		return id;
	}

	std::pair<std::string, std::string> ShaderManager::_verifyShaderPaths(std::string_view vertexShaderPath, std::string_view fragmentShaderPath)
	{
		std::pair<std::string, std::string> shaderPaths{ "", "" };

		//.vert and .frag extensions have to be contained in these variables in some
		// order
		if (!((vertexShaderPath.contains(".vert") && fragmentShaderPath.contains(".frag")) || (vertexShaderPath.contains(".frag") && fragmentShaderPath.contains(".vert"))))
		{
			return shaderPaths;
		}

		// vertexShaderPath contains .vert, meaning user passed in vertex path to this
		// variable
		if (vertexShaderPath.contains(".vert"))
		{
			shaderPaths.first = normalizeResourcePath(vertexShaderPath);
			shaderPaths.second = normalizeResourcePath(fragmentShaderPath);
		}
		else // vertexShaderPath contains .frag meaning user passed in fragment path
		    // to this variable, swap in shaderPaths
		{
			shaderPaths.first = normalizeResourcePath(fragmentShaderPath);
			shaderPaths.second = normalizeResourcePath(vertexShaderPath);
		}

		return shaderPaths;
	}

	Shader* ShaderManager::getShader(uint32_t shaderId)
	{
		auto it = m_shaders.find(shaderId);
		if (it == m_shaders.end())
			return nullptr;

		return it->second.get();
	}

	bool ShaderManager::removeShader(uint32_t shaderId)
	{
		auto it = m_shaders.find(shaderId);
		if (it == m_shaders.end())
			return false;

		m_shaders.erase(shaderId);
		std::erase_if(m_shaderPaths, [shaderId](const auto& entry)
		{
			if (entry.second != shaderId)
				return false;
			Engine::Instance().getSubSystem<ResourceManager>()->forgetShaderSources(entry.first);
			return true;
		});
		assert(m_shaderPaths.size() == m_shaders.size());
		return true;
	}
} // namespace SE
