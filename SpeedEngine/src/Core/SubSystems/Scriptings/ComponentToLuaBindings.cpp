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

		_registerComponent<NameComponent>(luaState);
	}
}
