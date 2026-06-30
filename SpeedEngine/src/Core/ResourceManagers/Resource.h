#pragma once

namespace SE
{
	class Resource
	{
	friend class ResourceManager;
	
	public:
		Resource(std::string resourceName) : m_resourceName(resourceName) {}
		~Resource() = default;

		const std::string& getResourceName() const { return m_resourceName; }
	private:
		std::string m_resourceName;
	};
	
	enum class ResourceType
	{
		Texture,
		Shader,
	};
}

