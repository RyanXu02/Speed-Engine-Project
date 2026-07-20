#pragma once

#include "../../ResourceManagers/Resource.h"
namespace SE
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
		ResourceChanged
	};

	//BASE CLASS
	class Event
	{
	public:
		bool handled = false;

		virtual ~Event() = default;
		virtual EventType getEventType() const = 0;
		virtual std::string getName() const = 0;

	};

	class WindowClose : public Event
	{
		EventType getEventType() const override { return EventType::WindowClose; }
		std::string getName() const override { return "WindowClose"; }
	};

	class ResourceChanged : public Event
	{
	public:
		uint32_t id;
		ResourceType type;
		bool adding;

		ResourceChanged(uint32_t p_id, ResourceType p_type, bool p_adding) : id(p_id), type(p_type), adding(p_adding) {}

		EventType getEventType() const override { return EventType::ResourceChanged; }
		std::string getName() const override { return "ResourceChanged"; }
	};
}

