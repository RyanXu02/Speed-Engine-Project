#pragma once

#include "Component/Component.h"

namespace SE
{
	class Scene;

	class Entity
	{
	public:
		Entity(std::string name);

		Entity(const Entity&);

		void initializeEntity(Scene* parentScene);
		void updateEntity(double deltaTime) {};
		void shutdownEntity() {};

		uint32_t getInstanceId() const { return m_instanceId; }

		const std::string& getName() const { return m_name; }
		void setName(std::string_view name);

		void killEntity() { m_isAlive = false; }
		bool isAlive() const { return m_isAlive; }

		void addComponent(std::unique_ptr<Component> component);
	private:
		bool m_isAlive{ true };

		std::string m_name;

		uint32_t m_instanceId;
		static uint32_t s_nextId;
		
		Scene* m_parentScene{ nullptr };

		std::vector<std::unique_ptr<Component>> m_components;

		std::string m_tag {"Untagged"}; // not used
	};
}

