#include "pch.h"
#include "PhysicsShapeMesh.h"

#include "Core/ResourceManagers/MeshResource/MeshResource.h"
#include "Core/ResourceManagers/ResourceManager.h"

#include <Jolt/Physics/Collision/Shape/MeshShape.h>

inline JPH::Vec3 toJPH(const glm::vec3& vec)
{
	return JPH::Vec3(vec.x, vec.y, vec.z);
}

namespace SE
{
	void PhysicsShapeMesh::setMeshResourceID(uint32_t resourceID)
	{
		if (resourceID == m_meshResourceID)
			return;

		m_meshResourceID = resourceID;
		updateShape();
	}

	void PhysicsShapeMesh::setPath(std::string path) 
	{
		if (path == m_path)
			return;

		m_path = path;
		auto meshResourceManager = Engine::Instance().getSubSystem<ResourceManager>();
		setMeshResourceID(meshResourceManager->addResource<ResourceType::MeshResource>(m_path, m_path));
	}

	void PhysicsShapeMesh::createShape()
	{
		if (m_meshResourceID == 0)
			return;

		MeshResource* meshResource = static_cast<MeshResource*>(Engine::Instance().getSubSystem<ResourceManager>()->getResource(m_meshResourceID));

		// Construct triangle list from mesh
		auto indices = meshResource->getIndices();
		auto vertices = meshResource->getVertices();
		JPH::TriangleList triangleList(indices.size() / 3);
		
		for (unsigned int i = 0; i < indices.size() / 3; ++i)
		{
			JPH::Vec3 v1(toJPH(vertices[indices[i * 3]].position));
			JPH::Vec3 v2(toJPH(vertices[indices[i * 3 + 1]].position));
			JPH::Vec3 v3(toJPH(vertices[indices[i * 3 + 2]].position));
			triangleList.push_back(JPH::Triangle(v1, v2, v3));
		}

		JPH::MeshShapeSettings shapeSettings(triangleList);
		JPH::Shape::ShapeResult result; // can be used for error checking
		m_shape = new JPH::MeshShape(shapeSettings, result);
	}
}
