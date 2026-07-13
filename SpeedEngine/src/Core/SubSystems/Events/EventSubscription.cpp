#include "pch.h"
#include "EventSubscription.h"

namespace SE
{
	EventSubscription::~EventSubscription()
	{
		unsubscribe();
	}

	EventSubscription::EventSubscription(EventSubscription&& other) noexcept
		: m_eventType(other.m_eventType), m_id(other.m_id)
	{
		other.m_id = 0; // invalidate the moved-from object
	}
	EventSubscription& EventSubscription::operator=(EventSubscription&& other) noexcept
	{
		if (this != &other)
		{
			unsubscribe(); // unsubscribe from the current subscription if it exists
			m_eventType = other.m_eventType;
			m_id = other.m_id;
			other.m_id = 0; // invalidate the moved-from object
		}
		return *this;
	}

	void EventSubscription::unsubscribe() const
	{
		EventSystem::Instance().unsubscribe(m_id, m_eventType);

	}

}