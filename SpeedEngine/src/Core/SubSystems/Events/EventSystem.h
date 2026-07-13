#pragma once
#include "../SubSystem.h"
#include "Event.h"

namespace SE
{
	class EventSubscription;

    class EventSystem : public SubSystem
    {
        using EventCallback = std::function<void(Event&)>;

		struct EventCallbackWrapper
		{
			uint32_t id;
			EventCallback callback;
		};

    public:
		EventSystem() : SubSystem("EventSystem") {}

		//@brief gets static instance (meyers singleton)
		//@returns reference to instance
		static EventSystem& Instance()
		{
			if (s_instance)
			{
				return *s_instance;
			}

			static EventSystem instance;
			s_instance = &instance;
			return *s_instance;
		}

		void init() override;
		void update(double deltaTime) override;
		void shutdown() override;

		//@brief sends an event to event queue
		//@param event the event to publish
		void publish(std::unique_ptr<Event> event);

		//@brief creates a listener for an event
		//@param eventType the type of event that is being listened for
		//@param callback the callback function to be invoked when the event is heard
		//@return an EventSubscription obj. This is done for RAII (on destruction, it auto-unsubs itself from the event)
		EventSubscription subscribe(EventType eventType, EventCallback callback);

		//@brief removes the specified id from listening to the specified event type
		//@param id the event id to remove
		//@param eventType the event type to unsubscribe from
		void unsubscribe(uint32_t id, EventType eventType);

    private:
		static EventSystem* s_instance;

        // id for each listener
        std::atomic<uint32_t> m_ids{ 1 }; // 0 for invalid id
		uint32_t _assignId() { return m_ids.fetch_add(1, std::memory_order_relaxed); }

		std::unordered_map<EventType, std::vector<EventCallbackWrapper>> m_allEventCallbacks;
		std::queue<std::unique_ptr<Event>> m_eventQueue;

		void _fireEvent(Event& event);
    };
}

