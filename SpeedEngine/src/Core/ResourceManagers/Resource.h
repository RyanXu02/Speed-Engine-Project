#pragma once

#include "Core/Features/MemoryTrackable.h"

namespace SE
{
	std::string normalizeResourcePath(std::string_view path);

	// @brief base class for a generic resource managed by the ResourceManager
	class Resource : public MemoryTrackable
	{
		friend class ResourceManager;

	public:
		Resource(uint32_t id, std::string resourceName)
		    : MemoryTrackable(&::SE::tagStorage("Resource", resourceName)),
		      m_resourceId(id),
		      m_resourceName(resourceName)
		{}
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
		Font,
		Texture
	};
} // namespace SE
