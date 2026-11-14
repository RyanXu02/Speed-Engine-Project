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

		// @brief Returns the runtime ID of the entity
		uint32_t id() const { return m_id; }
		// @brief Returns the Scene Serialization ID of the entity
		// @brief This ID used for serialization and deserialization of the entity only
		uint64_t ssid() const { return m_registry->getEntitySSID(m_id); }

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
		// @brief Direction vectors from the entity's transform rotation (local space)
		glm::vec3 forward() { return _rotated(glm::vec3(0.f, 0.f, -1.f)); }
		glm::vec3 up() { return _rotated(glm::vec3(0.f, 1.f, 0.f)); }
		glm::vec3 right() { return _rotated(glm::vec3(1.f, 0.f, 0.f)); }

	private:
		glm::vec3 _rotated(const glm::vec3& axis)
		{
			//Transform* t = getComponent<Transform>();
			//return t ? glm::normalize(t->rotate * axis) : axis;
			return glm::normalize(getComponent<Transform>()->rotate * axis);
		}

		uint32_t m_id;
		EntityRegistry* m_registry;
		typedef std::function<void(Entity)> HookFunction;

		friend class EntityRegistry;

		static HookFunction _onMeshComponentAdded();

		static HookFunction _onTerrainComponentAdded();

		static HookFunction _onScriptComponentAdded();
		static HookFunction _onScriptComponentRemoved();

		static HookFunction _onCameraComponentAdded();
		static HookFunction _onCameraComponentRemoved();

		static HookFunction _onRigidBodyComponentAdded();

		static HookFunction _onSFXComponentAdded();
		static HookFunction _onSFXComponentRemoved();

		static HookFunction _onMusicComponentAdded();

		static HookFunction _onPhysicsShapeMeshComponentAdded();

		static HookFunction _onParticleEmitterAdded();
	};
} // namespace SE
