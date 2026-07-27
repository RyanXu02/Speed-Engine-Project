#pragma once

#include "Component/Component.h"

namespace SE
{
	class Scene;

	class Entity
	{
	public:

		Entity(std::string name) : m_instanceId(s_nextId++), m_name(name) {}
	private:
		std::string m_name;

		uint32_t m_instanceId;
		static uint32_t s_nextId;
		
		Scene* m_ParentScene{ nullptr };

		std::vector<std::unique_ptr<Component>> m_components;

		std::string m_tag {"Untagged"}; // not used
	};
}

