#pragma once

namespace SE
{
	class Entity;

	// @brief Base class for all components that can be attached to an entity
	// @brief A component can be allowed to be attached multiple times to an entity, or only once. This is specified in the constructor of the component.
	class Component
	{

	public:
		virtual ~Component() = 0;

		// @brief Used to render information and controls about this component to ImGui properties window
		virtual bool renderImGuiInterface() = 0;

	protected:
		Component() = default;
		Entity* m_parentEntity{ nullptr };
	private:
		Component& operator=(const Component&) = delete;

	};
}

