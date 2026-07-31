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
		ResourceChanged, SceneChanged,
		ActiveSceneModified
	};

	//BASE CLASS
	class Event
	{
	public:
		bool handled = false;

		virtual ~Event() = default;
		virtual EventType getEventType() const = 0;
		virtual std::string getName() const = 0;
		virtual std::string getDataAsString() const = 0;

	};

	class WindowClose : public Event
	{
		EventType getEventType() const override { return EventType::WindowClose; }
		std::string getName() const override { return "WindowClose"; }
		std::string getDataAsString() const override { return ""; }
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
		std::string getDataAsString() const override { 
			//return std::format("id {}, type {}, adding = {}",id, type, adding);
			return "";
		}
	};

	class SceneChanged : public Event
	{
	public:
		std::string name;

		SceneChanged(const std::string& p_name) : name(p_name) {}

		EventType getEventType() const override { return EventType::SceneChanged; }
		std::string getName() const override { return "SceneChanged"; }
		std::string getDataAsString() const override { 
			return std::format("changing scene to {}", name);
		}
	};

	class ActiveSceneModified : public Event
	{
	public:
		enum class ModifyType
		{
			Add,
			Remove,
			Rename
		};
		ModifyType modifytype;
		uint32_t id;
		std::string name;
		ActiveSceneModified(ModifyType modifytype, uint32_t id, const std::string& name) : modifytype(modifytype), id(id), name(name) {}
		EventType getEventType() const override { return EventType::ActiveSceneModified; }
		std::string getName() const override { return "ActiveSceneModified"; }
		std::string getDataAsString() const override {
			return std::format("modifytype {}, id {}, name {}", static_cast<int>(modifytype), id, name);
		}
	};
}

