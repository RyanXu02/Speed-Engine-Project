#pragma once
#include "Event.h"
#include "EventSystem.h"

namespace SE
{
	class EventSubscription
	{
	public:
		EventSubscription() : m_id(0), m_eventType(EventType::None) {}
		EventSubscription(uint32_t id, EventType eventType) : m_id(id), m_eventType(eventType) {}
		~EventSubscription();

		// no copy or assignment (because an EventSubscription object is tied to a specific subscription in the EventSystem)
		EventSubscription(const EventSubscription&) = delete;
		EventSubscription& operator=(const EventSubscription&) = delete;
		// use move semantics to transfer ownership of the subscription
		EventSubscription(EventSubscription&& other) noexcept;
		EventSubscription& operator=(EventSubscription&& other) noexcept;
	
		//@brief unsubscribes this EventSubscription from EventSystem
		void unsubscribe() const;

	private:
		EventType m_eventType;
		uint32_t m_id;
	};
}

