#include "pch.h"
#include "ComponentToLuaBindings.h"

namespace SE
{

	void ComponentToLuaBindings::exportComponents(Logger*, sol::state& luaState) {

		luaState.new_usertype<Entity>("Entity",
			"id", [] (Entity& self) {
				return self.id();
			},
			"forward", &Entity::forward,
			"up", &Entity::up,
			"right", &Entity::right,
			"addComponent", [&] (Entity& self, const std::string& componentName, sol::object luaTable) {
				auto it = componentAdders.find(std::string(componentName));
				if (it != componentAdders.end()) {
				    it->second(self, luaTable);
				}
			},
			"getComponent", [&] (Entity& self, const std::string& componentName) {
				auto it = componentGetters.find(std::string(componentName));
				if (it != componentGetters.end()) {
				    return it->second(luaState, self);
				}
				return sol::object(luaState, sol::nil);
			},
			"removeComponent", [&] (Entity& self, const std::string& componentName) {
			    auto it = componentRemovers.find(std::string(componentName));
			    if (it != componentRemovers.end())
			    {
				    it->second(self);
			    }
			}
		);

		//camer acomponent has custom fields that the reflection does not support os manually bind it
		luaState.new_usertype<CameraComponent>("CameraComponent",
			"frustum", &CameraComponent::frustum,	//this need to be registered too, done in the scripting system
			"fov", &CameraComponent::fov,
			"nearPlane", &CameraComponent::nearPlane,
			"farPlane", &CameraComponent::farPlane,
			"isPrimary", &CameraComponent::isPrimary
		);

		componentGetters[CameraComponent::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object {
			CameraComponent* component = e.getComponent<CameraComponent>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};
		componentAdders[CameraComponent::getStaticType()] = [](Entity e, [[maybe_unused]] sol::table luaTable)
		{
			e.addComponent<CameraComponent>();
		};
		componentRemovers[CameraComponent::getStaticType()] = [](Entity e)
		{
			e.removeComponent<CameraComponent>();
		};

		_registerComponent<Transform>(luaState);

		luaState.new_usertype<RigidBody>("RigidBody", 
			"getOffset", &RigidBody::getOffset, 
			"setOffset", &RigidBody::setOffset,
			"getPosition", &RigidBody::getPosition,
			"setPosition", &RigidBody::setPosition,
			"getRotation", &RigidBody::getRotation,
			"setRotation", &RigidBody::setRotation,
			"getLinearVelocity", &RigidBody::getLinearVelocity, 
			"setLinearVelocity", &RigidBody::setLinearVelocity, 
			"getAngularVelocity", &RigidBody::getAngularVelocity,
		    "setAngularVelocity", &RigidBody::setAngularVelocity,
			"addForce", &RigidBody::addForce,
			"addTorque", &RigidBody::addTorque, 
			"addImpulse", &RigidBody::addImpulse, 
			"addAngularImpulse", &RigidBody::addAngularImpulse,
		    "activateBody", &RigidBody::activateBody,
		    "deactivateBody", &RigidBody::deactivateBody,
			"getMaxLinearVelocity", &RigidBody::getMaxLinearVelocity, 
			"setMaxLinearVelocity", &RigidBody::setMaxLinearVelocity, 
			"getMaxAngularVelocity", &RigidBody::getMaxAngularVelocity,
			"setMaxAngularVelocity", &RigidBody::setMaxAngularVelocity, 
			"getRestitution", &RigidBody::getRestitution, 
			"setRestitution", &RigidBody::setRestitution, 
			"getFriction", &RigidBody::getFriction, 
			"setFriction", &RigidBody::setFriction,
			"getGravityFactor", &RigidBody::getGravityFactor,
			"setGravityFactor", &RigidBody::setGravityFactor);

		componentGetters[RigidBody::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object
		{
			RigidBody* component = e.getComponent<RigidBody>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};
		componentAdders[RigidBody::getStaticType()] = [](Entity e, [[maybe_unused]] sol::table luaTable)
		{
			e.addComponent<RigidBody>();
		};
		componentRemovers[RigidBody::getStaticType()] = [](Entity e)
		{
			e.removeComponent<RigidBody>();
		};


		luaState.new_usertype<PhysicsShapeBox>("PhysicsShapeBox",
			"setHalfExtents", &PhysicsShapeBox::setHalfExtents);

		componentGetters[PhysicsShapeBox::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object
		{
			PhysicsShapeBox* component = e.getComponent<PhysicsShapeBox>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};
		componentAdders[PhysicsShapeBox::getStaticType()] = [](Entity e, [[maybe_unused]] sol::table luaTable)
		{
			e.addComponent<PhysicsShapeBox>();
		};
		componentRemovers[PhysicsShapeBox::getStaticType()] = [](Entity e)
		{
			e.removeComponent<PhysicsShapeBox>();
		};


		luaState.new_usertype<PhysicsShapeSphere>("PhysicsShapeSphere",
			"setRadius", &PhysicsShapeSphere::setRadius);

		componentGetters[PhysicsShapeSphere::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object
		{
			PhysicsShapeSphere* component = e.getComponent<PhysicsShapeSphere>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};
		componentAdders[PhysicsShapeSphere::getStaticType()] = [](Entity e, [[maybe_unused]] sol::table luaTable)
		{
			e.addComponent<PhysicsShapeSphere>();
		};
		componentRemovers[PhysicsShapeSphere::getStaticType()] = [](Entity e)
		{
			e.removeComponent<PhysicsShapeSphere>();
		};


		luaState.new_usertype<PhysicsShapeCapsule>("PhysicsShapeSphere",
			"setHalfHeight", &PhysicsShapeCapsule::setHalfHeight,
			"setRadius", &PhysicsShapeCapsule::setRadius);

		componentGetters[PhysicsShapeCapsule::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object
		{
			PhysicsShapeCapsule* component = e.getComponent<PhysicsShapeCapsule>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};
		componentAdders[PhysicsShapeCapsule::getStaticType()] = [](Entity e, [[maybe_unused]] sol::table luaTable)
		{
			e.addComponent<PhysicsShapeCapsule>();
		};
		componentRemovers[PhysicsShapeCapsule::getStaticType()] = [](Entity e)
		{
			e.removeComponent<PhysicsShapeCapsule>();
		};


		luaState.new_usertype<MeshComponent>("MeshComponent",
			"setPath", &MeshComponent::setPath);

		componentGetters[MeshComponent::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object
		{
			MeshComponent* component = e.getComponent<MeshComponent>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};
		componentAdders[MeshComponent::getStaticType()] = [](Entity e, [[maybe_unused]] sol::table luaTable)
		{
			e.addComponent<MeshComponent>();
		};
		componentRemovers[MeshComponent::getStaticType()] = [](Entity e)
		{
			e.removeComponent<MeshComponent>();
		};


		luaState.new_usertype<AudioSFXComponent>("AudioSFXComponent", 
			"PlayMySound", &AudioSFXComponent::PlayMySound, 
			"StopMySound", &AudioSFXComponent::StopMySound, 
			"updateMyPosition", &AudioSFXComponent::updateMyPosition);

		componentGetters[AudioSFXComponent::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object
		{
			AudioSFXComponent* component = e.getComponent<AudioSFXComponent>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};
		componentAdders[AudioSFXComponent::getStaticType()] = [](Entity e, sol::table)
		{
			e.addComponent<AudioSFXComponent>();
		};
		componentRemovers[AudioSFXComponent::getStaticType()] = [](Entity e)
		{
			e.removeComponent<AudioSFXComponent>();
		};


		luaState.new_usertype<AudioMusicComponent>("AudioMusicComponent", 
			"PlayMyTrack", &AudioMusicComponent::PlayMyTrack, 
			"StopMyTrack", &AudioMusicComponent::StopMyTrack);

		componentGetters[AudioMusicComponent::getStaticType()] = [](sol::state_view lua, Entity e) -> sol::object
		{
			AudioMusicComponent* component = e.getComponent<AudioMusicComponent>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};
		componentAdders[AudioMusicComponent::getStaticType()] = [](Entity e, sol::table)
		{
			e.addComponent<AudioMusicComponent>();
		};
		componentRemovers[AudioMusicComponent::getStaticType()] = [](Entity e)
		{
			e.removeComponent<AudioMusicComponent>();
		};

		_registerComponent<NameComponent>(luaState);
		_registerComponent<PrefabComponent>(luaState);
		_registerComponent<ScriptComponent>(luaState);
	}
}
