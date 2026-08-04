#pragma once

namespace SE
{
	class Resource
	{
	friend class ResourceManager;
	
	public:
		Resource(uint32_t id, std::string resourceName) : m_resourceId(id), m_resourceName(resourceName) {}
		~Resource() = default;

		const std::string& getResourceName() const { return m_resourceName; }
		const uint32_t getId() const { return m_resourceId; }
	private:
		uint32_t m_resourceId;
		std::string m_resourceName;
		
	};
	
	enum class ResourceType
	{
		Material,
		Shader,
		MeshResource,
	};
}

