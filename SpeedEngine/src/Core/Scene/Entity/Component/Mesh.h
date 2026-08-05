#pragma once
#include "Component.h"
#include "../../../ResourceManagers/MeshResource/MeshResource.h"

namespace SE 
{
	struct DrawData {
		std::vector<Vertex> positions;
		std::vector<uint32_t> indices;
	};

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

		DrawData getDrawData();

		uint32_t getMeshResourceId() const { return m_meshResourceId; }
		void setMeshResourceId(uint32_t meshResourceId) { m_meshResourceId = meshResourceId; m_isDirty = true; }
	private:
		Mesh* clone() const { return new Mesh(*this); }

		std::vector<Vertex> m_positions; //vertex data
		std::vector<SubMesh> m_submeshes; //draw calls as data, mainly for the materials
		std::vector<uint32_t> m_indices; //indices info, sent with vertex data to gpu
		uint32_t m_meshResourceId;
		bool m_isDirty = true;
	};
}

