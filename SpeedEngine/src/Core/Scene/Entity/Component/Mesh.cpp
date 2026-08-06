#include "pch.h"
#include "Mesh.h"

#include "../../../ResourceManagers/ResourceManager.h"

namespace SE
{
	Mesh::Mesh(const Mesh* other) : Component(DISALLOW_MULTIPLE_COMPONENTS),
		m_positions(other->m_positions),
		m_submeshes(other->m_submeshes),
		m_indices(other->m_indices),
		m_meshResourceId(other->m_meshResourceId),
		m_isDirty(other->m_isDirty)
	{}

	bool Mesh::initComponent() {
		return true;
	}

	bool Mesh::updateComponent() {
		if (m_isDirty && m_meshResourceId != 0) {
			MeshResource* mr = static_cast<MeshResource*>(ResourceManager::Instance().getResource(m_meshResourceId));
			if (mr)
			{
				m_positions = mr->getVertices();
				m_submeshes = mr->getSubMeshes();
				m_indices = mr->getIndices();
				m_isDirty = false;
			}
			else
			{
				return false;
			}
		}
		return true;
	}

	bool Mesh::renderComponent() {
		return true;
	}

	bool Mesh::shutdownComponent() {

		m_positions.clear();
		m_submeshes.clear();
		m_indices.clear();
		return true;
	}
}