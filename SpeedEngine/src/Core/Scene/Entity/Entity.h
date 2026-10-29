#pragma once

#include "Component/ComponentStructs.h"
#include "EntityRegistry.h"

namespace SE
{
	class Scene;

	// @brief An entity is a just handle, registry owns all entities and component
	// @brief entity is owned by a scene, and can be created and destroyed by the scene

	class Entity
	{
	public:
		Entity(const Entity& other);
		Entity(uint32_t id, EntityRegistry* registry);

		operator uint32_t() { return m_id; }

		uint32_t id() const { return m_id; }
		const EntityRegistry::ComponentRegistry* getComponentRegistry() const { return m_registry->getComponentRegistry(m_id); }

		template<typename T, typename... Args>
		T* addComponent(Args&&... args)
		{
			return m_registry->addComponent<T>(m_id, std::forward<Args>(args)...);
		}

		template<typename T>
		T* getComponent()
		{
			return m_registry->getComponent<T>(m_id);
		}

		template<typename T>
		bool removeComponent()
		{
			return m_registry->removeComponent<T>(m_id);
		}

	private:
		uint32_t m_id;
		EntityRegistry* m_registry;
		typedef std::function<void(Entity)> HookFunction;

		friend class EntityRegistry;

		static HookFunction _onNameComponentAdded();
		static HookFunction _onNameComponentRemoved();
		static HookFunction _onMeshComponentAdded();
		static HookFunction _onMeshComponentRemoved();
		static HookFunction _onScriptComponentAdded();
		static HookFunction _onScriptComponentRemoved();
		static HookFunction _onCameraComponentAdded();
		static HookFunction _onSFXComponentAdded();
		static HookFunction _onMusicComponentAdded();
		static HookFunction _onPhysicsShapeMeshComponentAdded();
	};
} // namespace SE
