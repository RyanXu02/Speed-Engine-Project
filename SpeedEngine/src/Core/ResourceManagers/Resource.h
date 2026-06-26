#pragma once

namespace SE
{
	class Resource
	{
	friend class ResourceManager;
	
	public:
		Resource(std::string resourceName) : m_resourceName(resourceName) {}
		~Resource() = default;

	private:
		std::string m_resourceName;
	};
}

