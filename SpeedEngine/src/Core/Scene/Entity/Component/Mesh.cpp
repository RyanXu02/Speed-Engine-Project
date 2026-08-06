#include "pch.h"
#include "Mesh.h"

#include "../../../ResourceManagers/ResourceManager.h"

namespace SE
{
	const std::vector<Vertex> Mesh::s_emptyVertices;
	const std::vector<SubMesh> Mesh::s_emptySubMeshes;
	const std::vector<uint32_t> Mesh::s_emptyIndices;

	Mesh::Mesh(const Mesh* other) : Component(DISALLOW_MULTIPLE_COMPONENTS),
		m_meshResourceId(other->m_meshResourceId)
	{}

	bool Mesh::initComponent() {
		return true;
	}

	bool Mesh::updateComponent() {
		return true;
	}

	bool Mesh::renderComponent() {
		return true;
	}

	bool Mesh::shutdownComponent() {
		return true;
	}

	MeshResource* Mesh::_getResource() const {
		if (m_meshResourceId == 0) return nullptr;
		return static_cast<MeshResource*>(ResourceManager::Instance().getResource(m_meshResourceId));
	}

	bool Mesh::hasValidResource() const {
		return _getResource() != nullptr;
	}

	const std::vector<Vertex>& Mesh::getPositionsToDraw() const {
		MeshResource* resource = _getResource();
		return resource ? resource->getVertices() : s_emptyVertices;
	}

	const std::vector<SubMesh>& Mesh::getSubMeshesToDraw() const {
		MeshResource* resource = _getResource();
		return resource ? resource->getSubMeshes() : s_emptySubMeshes;
	}

	const std::vector<uint32_t>& Mesh::getIndicesToDraw() const {
		MeshResource* resource = _getResource();
		return resource ? resource->getIndices() : s_emptyIndices;
	}
}