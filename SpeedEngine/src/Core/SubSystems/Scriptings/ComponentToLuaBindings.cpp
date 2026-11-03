#include "pch.h"
#include "ComponentToLuaBindings.h"

namespace SE
{

	void ComponentToLuaBindings::exportComponents(Logger* logger, sol::state& luaState) {

		luaState.new_usertype<Entity>("Entity",
			"id", [] (Entity& self) {
				return self.id();
			},
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
		componentAdders[CameraComponent::getStaticType()] = [](Entity e, sol::table luaTable)
		{
			e.addComponent<CameraComponent>();
		};
		componentRemovers[CameraComponent::getStaticType()] = [](Entity e)
		{
			e.removeComponent<CameraComponent>();
		};

		//had to manually register transform for custom functionality
		//the macro only allow registering member data, not function because of nlohman
		luaState.new_usertype<Transform>("Transform",
			"getPosition", &Transform::getTranslation,
			"setPosition", &Transform::setPosition,
			"getRotation", &Transform::getRotation,
			"setRotation", &Transform::setRotation,
			"getScale", &Transform::getScale,
			"setScale", &Transform::setScale
		);

		componentGetters["Transform"] = [](sol::state_view lua, Entity e) -> sol::object {
			Transform* component = e.getComponent<Transform>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};

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

		componentGetters["RigidBody"] = [](sol::state_view lua, Entity e) -> sol::object
		{
			RigidBody* component = e.getComponent<RigidBody>();
			if (component)
			{
				return sol::make_object(lua, component);
			}
			return sol::object(lua, sol::nil);
		};

		_registerComponent<NameComponent>(luaState);
	}
}
