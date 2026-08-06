#pragma once
#include "Component.h"
#include "../../../ResourceManagers/MeshResource/MeshResource.h"

namespace SE 
{
	class Mesh : public Component
	{
	public:
		Mesh() : Component(DISALLOW_MULTIPLE_COMPONENTS), m_meshResourceId(0) {}
		Mesh(const Mesh* other);
		~Mesh() override {};


		bool initComponent();
		bool updateComponent();
		bool renderComponent();
		bool shutdownComponent();

		const std::vector<Vertex>& getPositionsToDraw() const;
		const std::vector<SubMesh>& getSubMeshesToDraw() const;
		const std::vector<uint32_t>& getIndicesToDraw() const;

		uint32_t getMeshResourceId() const { return m_meshResourceId; }
		void setMeshResourceId(uint32_t meshResourceId) { m_meshResourceId = meshResourceId; }

		bool hasValidResource() const;

	private:
		Mesh* clone() const override { return new Mesh(this); }

		MeshResource* _getResource() const;

		uint32_t m_meshResourceId;

		// empty vectors to return when no resource is available
		static const std::vector<Vertex> s_emptyVertices;
		static const std::vector<SubMesh> s_emptySubMeshes;
		static const std::vector<uint32_t> s_emptyIndices;
	};
}

