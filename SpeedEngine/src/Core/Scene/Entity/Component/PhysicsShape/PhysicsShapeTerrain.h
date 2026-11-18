#pragma once
#include "Core/Features/Serializable.h"
#include "Core/ResourceManagers/Resource.h"

namespace SE
{
	class PhysicsShapeTerrain
	{
	public:
		PhysicsShapeTerrain() {};
		PhysicsShapeTerrain(std::string path)
		    : m_path(path) {};
		PhysicsShapeTerrain(uint32_t terrainId)
		    : m_terrainID(terrainId) {};
		~PhysicsShapeTerrain() = default;

		ResourceID getTerrainID() const { return m_terrainID; }
		void setTerrainID(ResourceID id) { m_terrainID = id; }
		const std::string& getPath() const { return m_path; }
		void setPath(std::string path, ResourceID id) { m_path = std::move(path); m_terrainID = id; }

		DEFINE_TYPE(PhysicsShapeTerrain, member("path", &PhysicsShapeTerrain::m_path));

	private:
		ResourceID m_terrainID{ 0 };
		std::string m_path = "None";
	};
}
