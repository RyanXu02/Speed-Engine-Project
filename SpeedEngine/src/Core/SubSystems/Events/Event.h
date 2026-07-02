#pragma once

namespace SE
{
	enum class EventType
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

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
}

