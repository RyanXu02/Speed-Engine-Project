#include "pch.h"
#include "MeshResource.h"

#include "rapidobj/rapidobj.hpp"

namespace SE
{
	bool MeshResource::init(std::string_view objpath, std::string_view mtlpath) {
		using namespace rapidobj;

		Result parsedfile;
		if (mtlpath.empty()) 
			parsedfile = ParseFile(objpath, MaterialLibrary::Default(Load::Optional)); //no mtl provided
		else 
			parsedfile = ParseFile(objpath, MaterialLibrary::SearchPathW(mtlpath)); //mtl provided

		if (parsedfile.error) {
			m_logger.warn("Failed to load obj file at {}", objpath);
			return false;
		}

		bool success = Triangulate(parsedfile);
		if (!success) {
			m_logger.warn("Failed to triangulate obj {}", objpath);
			return false;
		}

		// reserve space for vertices and indices based on the parsed file
		size_t totalIndices = 0;
		for (const auto& shape : parsedfile.shapes) {
			totalIndices += shape.mesh.indices.size();
		}
		m_indices.reserve(totalIndices);
		m_vertices.reserve(totalIndices);

		// Process each shape and unpack indices into vertices
		for (const auto& shape : parsedfile.shapes) {
			SubMesh submesh;
			submesh.indexOffset = static_cast<uint32_t>(m_indices.size());
			submesh.indexCount = static_cast<uint32_t>(shape.mesh.indices.size());
			submesh.materialIndex = shape.mesh.material_ids.empty() ? -1 : shape.mesh.material_ids[0];

			for (const Index& idx : shape.mesh.indices) {
				Vertex vertex{};

				// Position
				if (idx.position_index >= 0) {
					size_t pos_idx = static_cast<size_t>(idx.position_index) * 3;
					if (pos_idx + 2 < parsedfile.attributes.positions.size()) {
						vertex.position = glm::vec3(
							parsedfile.attributes.positions[pos_idx],
							parsedfile.attributes.positions[pos_idx + 1],
							parsedfile.attributes.positions[pos_idx + 2]
						);
					}
				}

				// Normal (optional)
				if (idx.normal_index >= 0) {
					size_t norm_idx = static_cast<size_t>(idx.normal_index) * 3;
					if (norm_idx + 2 < parsedfile.attributes.normals.size()) {
						vertex.normal = glm::vec3(
							parsedfile.attributes.normals[norm_idx],
							parsedfile.attributes.normals[norm_idx + 1],
							parsedfile.attributes.normals[norm_idx + 2]
						);
					}
				}

				// Texture coordinates (optional)
				if (idx.texcoord_index >= 0) {
					size_t tex_idx = static_cast<size_t>(idx.texcoord_index) * 2;
					if (tex_idx + 1 < parsedfile.attributes.texcoords.size()) {
						vertex.uv = glm::vec2(
							parsedfile.attributes.texcoords[tex_idx],
							parsedfile.attributes.texcoords[tex_idx + 1]
						);
					}
				}

				m_vertices.push_back(vertex);
				m_indices.push_back(static_cast<uint32_t>(m_vertices.size() - 1));
			}

			m_submeshes.push_back(submesh);
		}

		m_logger.info("Loaded mesh: {} vertices, {} indices, {} submeshes",
			m_vertices.size(), m_indices.size(), m_submeshes.size());

		return true;
	}

	void MeshResource::destroy() {
		m_vertices.clear();
		m_submeshes.clear();
		m_indices.clear();
	}
}