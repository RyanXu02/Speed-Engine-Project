#pragma once

#include "Component/ComponentStructs.h"
#include <typeindex>

namespace SE
{
	class Scene;

	// @brief An entity is a container for components that define its behavior and appearance in the scene
	// @brief An entity is owned by a scene, and can be created and destroyed by the scene
	class Entity
	{
	public:
		Entity(uint32_t id);

		uint32_t id() const { return m_id; }

		template<typename T, typename... Args>
		T* addComponent (Args&& ...args) {
			auto [it, status] = m_componentsNew.insert_or_assign(typeid(T), T(std::forward<Args>(args)...));
			if (!status) {
				return nullptr;
			}
			return std::any_cast<T>(&it->second);
		}

		template<typename T>
		T* getComponent() {
			auto it = m_componentsNew.find(typeid(T));
			if (it == m_componentsNew.end()) {
				return nullptr;
			}
			return std::any_cast<T>(&it->second);
		}

	private:
		uint32_t m_id;

		//Note: ideally want to use sparse set and dense array but until that becomes an issue, do not touch
		std::unordered_map<std::type_index, std::any> m_componentsNew;
	};
}

