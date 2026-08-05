#include "pch.h"
#include "MeshResource.h"

#include "rapidobj/rapidobj.hpp"

namespace SE
{
	bool MeshResource::init(std::string_view objpath, std::string_view mtlpath) {
		using namespace rapidobj;

		Result parsedfile;
		if (mtlpath.empty()) parsedfile = ParseFile(objpath, MaterialLibrary::Default(Load::Optional)); //no mtl provided
		else parsedfile = ParseFile(objpath, MaterialLibrary::SearchPathW(mtlpath)); //mtl provided

		if (parsedfile.error) {
			m_logger.warn("Failed to load obj file at {}", objpath);
			return false;
		}

		bool success = Triangulate(parsedfile);
		if (!success) {
			m_logger.warn("Failed to triangulate obj {}", objpath);
			return false;
		}

		//stride 3 for 1 vertex position
		for (size_t i = 0; i < parsedfile.attributes.positions.size(); i+=3 ) {
			float px, py, pz;
			px = parsedfile.attributes.positions[i];
			py = parsedfile.attributes.positions[i+1];
			pz = parsedfile.attributes.positions[i+2];
			/*nx = parsedfile.attributes.normals[i];
			ny = parsedfile.attributes.normals[i+1];
			nz = parsedfile.attributes.normals[i+2];*/

			m_vertices.push_back(Vertex({ glm::vec3(px,py,pz) }));
		}
		//stride 2 for texcoords into vertex.uv
		//......
		//need to do normals also
		
		//submeshes are material related so will implement later

		//m_indices population
		for (auto& shape : parsedfile.shapes) {
			//m_logger.info("shape :{}", shape.name);
			for (Index idx : shape.mesh.indices) {
				m_indices.push_back(idx.position_index);
			}
		}

		return true;
	}

	void MeshResource::destroy() {
		m_vertices.clear();
		m_submeshes.clear();
		m_indices.clear();
	}
}