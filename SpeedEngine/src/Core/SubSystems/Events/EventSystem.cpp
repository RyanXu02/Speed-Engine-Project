#include "pch.h"
#include "EventSystem.h"
#include "EventSubscription.h"

#include "../../Logger/Logger.h"

namespace SE
{
	EventSystem* EventSystem::s_instance = nullptr;
	
	void EventSystem::init()
	{
		SubSystem::init();
		s_instance = this;
	}

	void EventSystem::update(double deltaTime)
	{
		SubSystem::update(deltaTime);

		while (!m_eventQueue.empty())
		{
			auto event = std::move(m_eventQueue.front());
			m_eventQueue.pop();

			_fireEvent(*event);
		}
	}

	void EventSystem::shutdown()
	{
		SubSystem::shutdown();

		m_allEventCallbacks.clear();
		while (!m_eventQueue.empty())
		{
			m_eventQueue.pop();
		}
	}

	void EventSystem::publish(std::unique_ptr<Event> event)
	{
		m_eventQueue.push(std::move(event));
	}

	EventSubscription EventSystem::subscribe(EventType eventType, EventCallback callback)
	{
		uint32_t id = _assignId();
		m_allEventCallbacks[eventType].emplace_back( EventCallbackWrapper{id, callback} );
		return EventSubscription(id, eventType);
	}

	void EventSystem::unsubscribe(uint32_t id, EventType eventType)
	{
		auto it = m_allEventCallbacks.find(eventType);
		if (it != m_allEventCallbacks.end())
		{
			auto& callbacks = it->second;
			callbacks.erase(std::remove_if(callbacks.begin(), callbacks.end(),
				[id](const EventCallbackWrapper& eventCallbackWrapper) { return id == eventCallbackWrapper.id; }), callbacks.end());
		}
	}

	void EventSystem::_fireEvent(Event& event)
	{
		auto eventTypeIt = m_allEventCallbacks.find(event.getEventType());
		if (eventTypeIt != m_allEventCallbacks.end())
		{
			for (const auto& eventCallbackWrapper : eventTypeIt->second)
			{
				eventCallbackWrapper.callback(event);
				m_logger->debug("Fired event of type: " + event.getName() + " to callback with id: " + std::to_string(eventCallbackWrapper.id));
				m_logger->debug("Event info: {}", event.getDataAsString());
				if (event.handled) break;
			}
		}
	}
}