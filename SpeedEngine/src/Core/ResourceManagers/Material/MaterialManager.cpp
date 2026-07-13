#include "pch.h"
#include "MaterialManager.h"


namespace SE
{
	void MaterialManager::init() {
		Manager::init();

		m_materials.reserve(128);
	}

	void MaterialManager::shutdown() {
		Manager::shutdown();

		for (auto& [id, material] : m_materials) {
			material.get()->destroy();
		}
		m_materials.clear();

		m_logger->info("MaterialManager cleared all materials");
	}

	uint32_t MaterialManager::addMaterial(uint32_t id, uint32_t shaderId,
		std::span<const std::pair<TextureType, std::string_view>> textures,
		std::string_view materialName) 
	{
		//look for material in cache
		auto it = std::ranges::find_if(m_materials, [materialName](const auto& kv) {
			return kv.second->getResourceName() == materialName;
			});
		if (it != m_materials.end()) {
			m_logger->verbose("Material with name already loaded: {} with ID {}", it->second->getResourceName(), it->first);
			return it->first;
		}

		auto newMat = std::make_unique<Material>(id, materialName, *m_logger);
		if (!newMat->init(shaderId, textures)) return 0;
		m_materials[id] = std::move(newMat);
		m_logger->info("Material '{}' loaded: ID {}", m_materials[id]->getResourceName(), id);
		
		return id;
	}

	Material* MaterialManager::getMaterial(uint32_t materialId) {
		auto it = m_materials.find(materialId);
		if (it == m_materials.end()) return nullptr;

		return it->second.get();
	}
}