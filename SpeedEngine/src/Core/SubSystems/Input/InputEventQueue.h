#pragma once

#include "InputEvent.h"

namespace SE
{
	// Doesn't have anything to do with the event system, just a thread-safe queue for input events
	class InputEventQueue
	{
	public:
		void push(InputEvent event)
		{
			std::lock_guard lock(m_mutex);
			m_pending.push_back(event);
		}

		void drain(std::vector<InputEvent>& events)
		{
			events.clear();
			std::lock_guard lock(m_mutex);
			m_pending.swap(events);
		}

	private:
		std::mutex m_mutex;
		std::vector<InputEvent> m_pending;
	};
}
