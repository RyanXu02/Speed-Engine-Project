#include "pch.h"
#include "PhysicsShapeMesh.h"

#include "Engine.h"

#include "Core/ResourceManagers/MeshResource/MeshResource.h"
#include "Core/ResourceManagers/ResourceManager.h"

#include <Jolt/Physics/Collision/Shape/MeshShape.h>

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
		auto resourceManager = Engine::Instance().getSubSystem<ResourceManager>();
		setMeshResourceID(resourceManager->loadMesh({ .path = m_path, .name = m_path }));
	}

	void PhysicsShapeMesh::createUnscaledShape()
	{
		if (m_meshResourceID == 0)
			return;

		MeshResource* meshResource = static_cast<MeshResource*>(Engine::Instance().getSubSystem<ResourceManager>()->getResource(m_meshResourceID));

		// Construct triangle list from mesh
		auto indices = meshResource->getIndices();
		auto vertices = meshResource->getVertices();
		JPH::TriangleList triangleList;
		triangleList.reserve(indices.size() / 3);
		
		for (unsigned int i = 0; i < indices.size() / 3; ++i)
		{
			JPH::Vec3 v1(toJPH(vertices[indices[i * 3]].position));
			JPH::Vec3 v2(toJPH(vertices[indices[i * 3 + 1]].position));
			JPH::Vec3 v3(toJPH(vertices[indices[i * 3 + 2]].position));
			triangleList.push_back(JPH::Triangle(v1, v2, v3));
		}

		JPH::MeshShapeSettings shapeSettings(triangleList);
		JPH::Shape::ShapeResult result; // can be used for error checking
		m_unscaledShape = new JPH::MeshShape(shapeSettings, result);
	}
}
