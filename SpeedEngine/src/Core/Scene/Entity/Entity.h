#pragma once

#include "Component/Component.h"
#include <typeindex>

namespace SE
{
	class Scene;

	// @brief An entity is a container for components that define its behavior and appearance in the scene
	// @brief An entity is owned by a scene, and can be created and destroyed by the scene
	class Entity
	{
	public:
		Entity(std::string name);
		Entity(const Entity&);

		void initializeEntity(Scene* parentScene);

		uint32_t getInstanceId() const { return m_instanceId; }

		const std::string& getName() const { return m_name; }
		void setName(std::string_view name);

		// @brief Marks the entity as dead, so it can be removed from the scene in the next update cycle
		void killEntity() { m_isAlive = false; }
		bool isAlive() const { return m_isAlive; }

		template<typename T, typename... Args>
		T& addComponepnt (Args&& ...args) {

		}

		void addComponent(std::unique_ptr<Component> component);

		template <typename T>
		T* getComponent() const
		{
			for (const auto& component : m_components)
			{
				if (typeid(*component.get()) == typeid(T))
				{
					return static_cast<T*>(component.get());
				}
			}
			return nullptr;
		}

		const std::vector<std::unique_ptr<Component>>& getComponents() const { return m_components; }

	private:
		bool m_isAlive{ true };

		std::string m_name;

		uint32_t m_instanceId;
		static uint32_t s_nextId;
		
		Scene* m_parentScene{ nullptr };

		std::vector<std::unique_ptr<Component>> m_components;

		uint32_t m_id;
		std::unordered_map<std::type_index, std::any> m_componentsNew;
	};
}

