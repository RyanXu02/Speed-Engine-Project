#pragma once

#define ALLOW_MULTIPLE_COMPONENTS true
#define DISALLOW_MULTIPLE_COMPONENTS false

namespace SE
{
	class Entity;

	class Component
	{

	public:
		Component() = delete; // use the one below thx
		Component(bool allowMultipleComponent) : m_allowMultipleComponent(allowMultipleComponent) {}

		Component& operator=(const Component&) = delete;

		virtual ~Component() = 0;

		virtual Component* clone() const = 0;

		virtual bool initComponent() = 0;
		virtual bool updateComponent() = 0;
		virtual bool renderComponent() = 0;
		virtual bool shutdownComponent() = 0;

		void setParentEntity(Entity* parentEntity) { m_parentEntity = parentEntity; }

		bool allowsMultipleComponents() const { return m_allowMultipleComponent; }

	protected:
		Entity* m_parentEntity{ nullptr };
	private:
		bool m_allowMultipleComponent{ DISALLOW_MULTIPLE_COMPONENTS };
	};
}

