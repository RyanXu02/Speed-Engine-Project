#pragma once
#include "Core/Features/Serializable.h"
#include "Core/ResourceManagers/Resource.h"

namespace SE
{
	class PhysicsShapeMesh
	{
	public:
		PhysicsShapeMesh() {};
		PhysicsShapeMesh(std::string path)
		    : m_path(path) {};
		PhysicsShapeMesh(uint32_t meshId)
		    : m_meshResourceID(meshId) {};
		~PhysicsShapeMesh() = default;

		ResourceID getMeshResourceID() const { return m_meshResourceID; }
		void setMeshResourceID(ResourceID id) { m_meshResourceID = id; }
		const std::string& getPath() const { return m_path; }
		void setPath(std::string path, ResourceID id) { m_path = std::move(path); m_meshResourceID = id; }

		DEFINE_TYPE(PhysicsShapeMesh, member("path", &PhysicsShapeMesh::m_path));

	private:
		ResourceID m_meshResourceID{ 0 };
		std::string m_path = "None";
	};
}
