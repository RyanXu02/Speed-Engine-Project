#pragma once

#include "../../ResourceManagers/Resource.h"
#include "core/features/Serializable.h"
#include "glm/glm.hpp"


// When adding editor-only events, add them at the bottom of this file inside the COMPILE_WITH_EDITOR block
namespace SE
{
	struct CameraFrustum;

	enum class EventType
	{
		None = 0,
		WindowClose,
		WindowResize,
		WindowFocus,
		WindowLostFocus,
		WindowMoved,
		KeyPressed,
		KeyReleased,
		KeyTyped,
		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseScrolled,
		SceneChanged, // Active scene has been changed
		ActiveSceneModified, // Active scene has been modified (entity added, removed, renamed)
		CameraSwitched,
#if defined(COMPILE_WITH_EDITOR)
		EntitySelected, // An entity has been selected in the SceneInfoWidget
		ResourceChanged, // A resource in the resource manager has been added or removed
#endif
	};

	// @brief Base class for all events that can be sent through the EventSystem
	class Event
	{
	public:
		bool handled = false;

		virtual ~Event() = default;
		virtual EventType getEventType() const = 0;
		virtual std::string getName() const = 0;
		virtual std::string getDataAsString() const = 0;
	};

	// @brief Event that is triggered when the window is closed
	class WindowClose : public Event
	{
		EventType getEventType() const override { return EventType::WindowClose; }
		std::string getName() const override { return "WindowClose"; }
		std::string getDataAsString() const override { return ""; }
	};

	class MouseMovedEvent : public Event
	{
	public:
		double m_x = 0;
		double m_y = 0;

		MouseMovedEvent(double x, double y)
		    : m_x(x),
		      m_y(y)
		{}

		EventType getEventType() const override { return EventType::MouseMoved; }
		std::string getName() const override { return "MouseMovedEvent"; }
		std::string getDataAsString() const override { return ""; }

		DEFINE_TYPE(
			MouseMovedEvent, 
			member("m_x", &MouseMovedEvent::m_x),
			member("m_y", &MouseMovedEvent::m_y)
		);
	};

	class SwitchCameraEvent : public Event
	{
	public:
		CameraFrustum* camera;

		SwitchCameraEvent(CameraFrustum* other)
		    : camera(other)
		{}

		EventType getEventType() const override { return EventType::CameraSwitched; }
		std::string getName() const override { return "SwitchCameraEvent"; }
		std::string getDataAsString() const override { return ""; }

		DEFINE_TYPE(
			SwitchCameraEvent
		);
	};

	
	class KeyPressedEvent : public Event
	{
	public:
		int keyCode = 0;
		bool isRepeat = false;

		KeyPressedEvent(int keyCode, bool isRepeat = false)
		    : keyCode(keyCode),
		      isRepeat(isRepeat)
		{}

		EventType getEventType() const override { return EventType::KeyPressed; }
		std::string getName() const override { return "KeyPressedEvent"; }
		std::string getDataAsString() const override { return ""; }

		DEFINE_TYPE(
			KeyPressedEvent, 
			member("keyCode", &KeyPressedEvent::keyCode), 
			member("isRepeat", &KeyPressedEvent::isRepeat)
		);
	};

	// @brief Event that is triggered when the active scene is changed
	class SceneChanged : public Event
	{
	public:
		std::string name;

		SceneChanged(const std::string& p_name)
		    : name(p_name)
		{}

		EventType getEventType() const override { return EventType::SceneChanged; }
		std::string getName() const override { return "SceneChanged"; }
		std::string getDataAsString() const override { return std::format("changing scene to {}", name); }
	};

	// @brief Event that is triggered when the active scene is modified
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
		ActiveSceneModified(ModifyType modifytype, uint32_t id, const std::string& name)
		    : modifytype(modifytype),
		      id(id),
		      name(name)
		{}
		EventType getEventType() const override { return EventType::ActiveSceneModified; }
		std::string getName() const override { return "ActiveSceneModified"; }
		std::string getDataAsString() const override { return std::format("modifytype {}, id {}, name {}", static_cast<int>(modifytype), id, name); }
	};
#if defined(COMPILE_WITH_EDITOR)
	// @brief Event that is triggered when an entity is selected in the
	// SceneInfoWidget
	class EntitySelected : public Event
	{
	public:
		uint32_t id;

		EntitySelected(uint32_t p_id)
		    : id(p_id)
		{}
		EventType getEventType() const override { return EventType::EntitySelected; }
		std::string getName() const override { return "EntitySelected"; }
		std::string getDataAsString() const override { return std::format("Entity with id {} has been selected", id); }
	};

	// @brief Event that is triggered when a resource in the resource manager is
	// changed
	class ResourceChanged : public Event
	{
	public:
		uint32_t id;
		ResourceType type;
		bool adding;

		ResourceChanged(uint32_t p_id, ResourceType p_type, bool p_adding)
		    : id(p_id),
		      type(p_type),
		      adding(p_adding)
		{}

		EventType getEventType() const override { return EventType::ResourceChanged; }
		std::string getName() const override { return "ResourceChanged"; }
		std::string getDataAsString() const override
		{
			// return std::format("id {}, type {}, adding = {}",id, type, adding);
			return "";
		}
	};
#endif
} // namespace SE
