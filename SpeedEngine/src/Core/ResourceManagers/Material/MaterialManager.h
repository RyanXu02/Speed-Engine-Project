#pragma once
#include "../Manager.h"
#include "Material.h"

namespace SE
{
	class MaterialManager : public Manager
	{
	public:
		MaterialManager(const std::string& name = "MaterialManager") : Manager(ResourceType::Material, name) {}
		~MaterialManager() override = default;

		void init() override;
		void shutdown() override;

		uint32_t addMaterial(uint32_t id, uint32_t shaderId,
			std::span<const std::pair<TextureType, std::string_view>> textures,
			std::string_view materialName);

		Material* getMaterial(uint32_t materialId);

		//@brief removes a material from the manager given an id
		//@param materialId the id of material to remove
		//@returns true if successfully removed, false if not
		bool removeMaterial(uint32_t materialId);
	private:
		//id -> material object
		std::unordered_map<uint32_t, std::unique_ptr<Material>> m_materials;
	};
}

