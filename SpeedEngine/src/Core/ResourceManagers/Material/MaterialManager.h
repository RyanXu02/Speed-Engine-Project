#pragma once
#include "../Manager.h"
#include "Material.h"

namespace SE
{
	class MaterialManager : public Manager
	{
		friend class ResourceManager;

	public:
		MaterialManager(const std::string& name = "MaterialManager") : Manager(ResourceType::Material, name) {}
		~MaterialManager() override = default;

		void init() override;
		void shutdown() override;

	private:
		// @brief adds a material to this manager
		// @param id The unique ID for the material
		// @param shaderId The ID of the shader to use for this material
		// @param textures A span of pairs, each containing a TextureType and the corresponding file path for the texture
		// @param materialName The name of the material shown in the resource browser
		// @returns the ID of the newly added material, or 0 if the material could not be added
		uint32_t addMaterial(uint32_t id, uint32_t shaderId,
			std::span<const std::pair<TextureType, std::string_view>> textures,
			std::string_view materialName);

		// @brief retrieves a material from the manager given an id
		// @param materialId the id of material to retrieve
		// @returns the requested material if it exists, nullptr if not
		Material* getMaterial(uint32_t materialId);

		// @brief removes a material from the manager given an id
		// @param materialId the id of material to remove
		// @returns true if successfully removed, false if not
		bool removeMaterial(uint32_t materialId);

		// map ids -> material objects
		std::unordered_map<uint32_t, std::unique_ptr<Material>> m_materials;
	};
}

