#pragma once
#include "Component.h"
#include "Core/ResourceManagers/MeshResource/MeshResource.h"

namespace SE 
{
	// @brief Represents a mesh component that can be attached to an entity
	// @brief A mesh component contains a reference to a mesh resource, which contains the vertex data, sub-meshes, and indices for a 3D model
	class Mesh : public Component
	{
	public:
		Mesh() : Component(), m_meshResourceId(0) {}
		Mesh(const Mesh* other);
		~Mesh() override {};

		bool renderImGuiInterface();

		const std::vector<Vertex>& getPositionsToDraw() const;
		const std::vector<SubMesh>& getSubMeshesToDraw() const;
		const std::vector<uint32_t>& getIndicesToDraw() const;

		uint32_t getMeshResourceId() const { return m_meshResourceId; }
		void setMeshResourceId(uint32_t meshResourceId) { m_meshResourceId = meshResourceId; }

		bool hasValidResource() const;

	private:
		MeshResource* _getResource() const;

		uint32_t m_meshResourceId;

		// empty vectors to return when no resource is available
		static const std::vector<Vertex> s_emptyVertices;
		static const std::vector<SubMesh> s_emptySubMeshes;
		static const std::vector<uint32_t> s_emptyIndices;
	};
}

