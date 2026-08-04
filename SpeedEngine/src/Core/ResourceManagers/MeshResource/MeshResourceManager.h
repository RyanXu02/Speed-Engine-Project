#pragma once
#include "../Manager.h"
#include "MeshResource.h"

namespace SE
{
	class MeshResourceManager : public Manager
	{
	public:
		MeshResourceManager(const std::string& name = "MeshResourceManager") : Manager(ResourceType::MeshResource, name) {}
		~MeshResourceManager() override = default;

		void init() override;
		void shutdown() override;

		uint32_t addMeshResource(uint32_t id, std::string_view objpath, std::string_view mtlpath); //TODO: IMPL,EMENT

		MeshResource* getMeshResource(uint32_t meshResourceId);

		bool removeMeshResource(uint32_t meshResourceId);
	private:
		std::unordered_map<uint32_t, std::unique_ptr<MeshResource>> m_meshresources;
	};

}

