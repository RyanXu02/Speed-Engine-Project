#pragma once
#include "PhysicsShape.h"

namespace SE
{
	class PhysicsShapeMesh : public PhysicsShape
	{
	public:
		PhysicsShapeMesh() {};
		PhysicsShapeMesh(std::string path)
		    : m_path(path) {};
		PhysicsShapeMesh(uint32_t meshId)
		    : m_meshResourceID(meshId) {};
		~PhysicsShapeMesh() = default;

		uint32_t getMeshResourceID() { return m_meshResourceID; }
		void setMeshResourceID(uint32_t resourceID);
		std::string getPath() { return m_path; }
		void setPath(std::string path);

		DEFINE_TYPE(PhysicsShapeMesh, member("path", &PhysicsShapeMesh::m_path));

	private:
		void createShape() override;

		uint32_t m_meshResourceID{ 0 };
		std::string m_path = "None";
	};
}
