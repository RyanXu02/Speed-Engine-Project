#include "pch.h"
#include "Mesh.h"

namespace SE
{
	Mesh::Mesh(const Mesh* other) : Component(DISALLOW_MULTIPLE_COMPONENTS),
		m_positions(other->m_positions), m_submeshes(other->m_submeshes),
		m_meshResourceId(other->m_meshResourceId)
	{}

	bool Mesh::initComponent() {
		return true;
	}

	bool Mesh::updateComponent() {
		if (m_isDirty) {
			/*MeshResource* mr = static_cast<MeshResource>(ResourceManager::Instance().getResource(m_meshResourceId));
			m_posititoisn = mr.createPositions() //this is ok, copy vector from meshresource to mesh.positions
				.asfdjklasfdjha*/
			//function signature in meshResource: std::vector<MEsh::Vertex> createPositions()....
		}
		return true;
	}

	bool Mesh::renderComponent() {
		return true;
	}

	bool Mesh::shutdownComponent() {

		m_positions.clear();
		m_submeshes.clear();
		return true;
	}

}