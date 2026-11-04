#pragma once

#include "Core/Features/Serializable.h"

namespace SE
{
	class Entity;

	class EntityRegistry : public Serializable
	{
	public:
		//Note: ideally want to use sparse set and dense array for performance but until that becomes an issue, do not touch
		// each entity id is associated with a map of components
		typedef std::unordered_map<std::type_index, std::any> ComponentRegistry;
		typedef std::unordered_map<uint32_t, ComponentRegistry> Registry;

		explicit EntityRegistry(const LoggerProxy& logger);
		~EntityRegistry();

		uint32_t addEntity();
		const ComponentRegistry* getComponentRegistry(uint32_t entityID) const;
		bool hasEntity(uint32_t entityID);
		bool removeEntity(uint32_t entityID);
		void clear();
		bool empty();

		virtual void serialize(nlohmann::json& j) const override;
		virtual void deserialize(const nlohmann::json& j) override;

		template<typename T, typename... Args>
		T* addComponent(uint32_t entityID, Args&&... args)
		{
			auto it = m_registry.find(entityID);
			if (it == m_registry.end())
			{
				return nullptr;
			}
			ComponentRegistry& components = it->second;
			auto [componentIt, status] = components.try_emplace(typeid(T), T(std::forward<Args>(args)...));

			return std::any_cast<T>(&componentIt->second);
		}

		template<typename T>
		T* getComponent(uint32_t entityID)
		{
			auto it = m_registry.find(entityID);
			if (it == m_registry.end())
			{
				return nullptr;
			}

			ComponentRegistry& components = it->second;

			auto componentIt = components.find(typeid(T));
			if (componentIt == components.end())
			{
				return nullptr;
			}
			return std::any_cast<T>(&componentIt->second);
		}

		template<typename T>
		bool removeComponent(uint32_t entityID)
		{
			auto it = m_registry.find(entityID);
			if (it == m_registry.end())
			{
				return false;
			}

			ComponentRegistry& components = it->second;
			auto componentIt = components.find(typeid(T));

			if (componentIt == components.end())
			{
				return false;
			}

			components.erase(componentIt);
			return true;
		}


		const Registry::const_iterator begin() const;
		const Registry::const_iterator end() const;


	private:
		const LoggerProxy& m_logger;
		Registry m_registry;
		std::atomic<uint32_t> m_entityID{ 1 }; // reserve 0 for invalid entity

		typedef std::function<void(uint32_t, const nlohmann::json&)> OnComponentAdded;
		typedef std::function<void(uint32_t, const nlohmann::json&)> OnComponentRemoved;

		uint32_t _assignID();
	};
}
