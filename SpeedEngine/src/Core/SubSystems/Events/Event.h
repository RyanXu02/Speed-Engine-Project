#pragma once

#include "../../ResourceManagers/Resource.h"
#include "core/features/Serializable.h"
#include "Core/Features/MathToJSONBindings.h"
#include <glm/glm.hpp>
#include <Jolt/Physics/Body/BodyID.h>


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
		CollisionAddedEvent,
		CollisionPersistedEvent,
		CollisionRemovedEvent,
		ButtonActivated,
#if defined(COMPILE_WITH_EDITOR)
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

	class ButtonActivated : public Event
	{
	public:
		uint32_t entityId;
		std::string canvasKey;
		std::string elementKey;

		ButtonActivated(uint32_t entityId, std::string canvasKey, std::string elementKey)
			: entityId(entityId), canvasKey(std::move(canvasKey)), elementKey(std::move(elementKey))
		{}

		EventType getEventType() const override { return EventType::ButtonActivated; }
		std::string getName() const override { return "ButtonActivated"; }
		std::string getDataAsString() const override { return std::format("entity {}, canvas {}, element {}", entityId, canvasKey, elementKey); }
	};

	// @brief Event that is triggered when two rigidbodies first collide
	class CollisionAddedEvent : public Event
	{
	public:
		uint32_t bodyID1;
		uint32_t bodyID2;
		glm::vec3 normal;
		glm::vec3 id1Velocity;
		glm::vec3 id2Velocity;
		CollisionAddedEvent(uint32_t bodyID1, uint32_t bodyID2, glm::vec3 normal, glm::vec3 id1Velocity, glm::vec3 id2Velocity)
		    : bodyID1(bodyID1),
		      bodyID2(bodyID2),
		      normal(normal),
		      id1Velocity(id1Velocity),
		      id2Velocity(id2Velocity)
		{}
		EventType getEventType() const override { return EventType::CollisionAddedEvent; }
		std::string getName() const override { return "CollisionAddedEvent"; }
		std::string getDataAsString() const override { return std::format("Entity with BodyID {} collided with entity with BodyID {}", bodyID1, bodyID2); }
		DEFINE_TYPE(CollisionAddedEvent,
			member("bodyID1", &CollisionAddedEvent::bodyID1),
			member("bodyID2", &CollisionAddedEvent::bodyID2),
			member("normal", &CollisionAddedEvent::normal),
			member("id1Velocity", &CollisionAddedEvent::id1Velocity),
			member("id2Velocity", &CollisionAddedEvent::id2Velocity));
	};

	// @brief Event that is triggered when two rigidbodies continue colliding
	class CollisionPersistedEvent : public Event
	{
	public:
		uint32_t bodyID1;
		uint32_t bodyID2;
		glm::vec3 normal;
		CollisionPersistedEvent(uint32_t bodyID1, uint32_t bodyID2, glm::vec3 normal)
		    : bodyID1(bodyID1),
		      bodyID2(bodyID2),
		      normal(normal)
		{}
		EventType getEventType() const override { return EventType::CollisionPersistedEvent; }
		std::string getName() const override { return "CollisionPersistedEvent"; }
		std::string getDataAsString() const override { return std::format("Entity with BodyID {} continued colliding with entity with BodyID {}", bodyID1, bodyID2); }
		DEFINE_TYPE(CollisionPersistedEvent,
			member("bodyID1", &CollisionPersistedEvent::bodyID1),
			member("bodyID2", &CollisionPersistedEvent::bodyID2),
			member("normal", &CollisionPersistedEvent::normal));
	};

	// @brief Event that is triggered when two rigidbodies stop colliding
	class CollisionRemovedEvent : public Event
	{
	public:
		uint32_t bodyID1;
		uint32_t bodyID2;
		CollisionRemovedEvent(uint32_t bodyID1, uint32_t bodyID2)
		    : bodyID1(bodyID1),
		      bodyID2(bodyID2)
		{}
		EventType getEventType() const override { return EventType::CollisionRemovedEvent; }
		std::string getName() const override { return "CollisionRemovedEvent"; }
		std::string getDataAsString() const override { return std::format("Entity with BodyID {} stopped colliding with entity with BodyID {}", bodyID1, bodyID2); }
		DEFINE_TYPE(CollisionRemovedEvent,
			member("bodyID1", &CollisionRemovedEvent::bodyID1),
			member("bodyID2", &CollisionRemovedEvent::bodyID2));
	};

#if defined(COMPILE_WITH_EDITOR)
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
