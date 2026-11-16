#pragma once
#include "PhysicsShape.h"

namespace SE
{
	class PhysicsShapeTerrain : public PhysicsShape
	{
	public:
		PhysicsShapeTerrain() {};
		PhysicsShapeTerrain(std::string path)
		    : m_path(path) {};
		PhysicsShapeTerrain(uint32_t terrainId)
		    : m_terrainID(terrainId) {};
		~PhysicsShapeTerrain() = default;

		uint32_t getTerrainID() { return m_terrainID; }
		void setTerrainID(uint32_t resourceID);
		std::string getPath() { return m_path; }
		void setPath(std::string path);

		DEFINE_TYPE(PhysicsShapeTerrain, member("path", &PhysicsShapeTerrain::m_path));

	private:
		void createUnscaledShape() override;

		uint32_t m_terrainID{ 0 };
		std::string m_path = "None";
	};
}
