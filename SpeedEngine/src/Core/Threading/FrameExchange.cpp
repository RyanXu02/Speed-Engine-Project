#include "pch.h"

#include "FrameExchange.h"

namespace SE
{
	FrameExchange::WriteHandle::WriteHandle(FrameExchange& exchange, std::size_t index)
		: m_exchange(&exchange), m_index(index)
	{}

	FrameExchange::WriteHandle::WriteHandle(WriteHandle&& other) noexcept
		: m_exchange(std::exchange(other.m_exchange, nullptr)), m_index(other.m_index)
	{}

	FrameExchange::WriteHandle::~WriteHandle()
	{
		if (m_exchange)
			m_exchange->_freeSlot(m_index, FrameSlotState::Writing);
	}

	RenderFrame& FrameExchange::WriteHandle::frame()
	{
		assert(m_exchange);
		return m_exchange->m_slots[m_index].frame();
	}

	bool FrameExchange::WriteHandle::publish()
	{
		assert(m_exchange);
		const bool published = m_exchange->_publish(m_index);
		m_exchange = nullptr;
		return published;
	}

	FrameExchange::ReadHandle::ReadHandle(FrameExchange& exchange, std::size_t index)
		: m_exchange(&exchange), m_index(index)
	{}

	FrameExchange::ReadHandle::ReadHandle(ReadHandle&& other) noexcept
		: m_exchange(std::exchange(other.m_exchange, nullptr)), m_index(other.m_index)
	{}

	FrameExchange::ReadHandle::~ReadHandle()
	{
		if (m_exchange)
			m_exchange->_freeSlot(m_index, FrameSlotState::Reading);
	}

	const RenderFrame& FrameExchange::ReadHandle::frame() const
	{
		assert(m_exchange);
		const auto& slot = m_exchange->m_slots[m_index];
		return slot.frame();
	}

	FrameExchange::WriteHandle FrameExchange::beginWrite()
	{
		std::unique_lock lock(m_mutex);

		// Wait until a free slot is available or the exchange is closed
		m_freeSlotAvailable.wait(lock, [this]
		{
			return m_closed || std::find(m_states.begin(), m_states.end(), FrameSlotState::Free) != m_states.end();
		});
		if (m_closed)
			return {};

		const auto index = static_cast<std::size_t>(std::find(m_states.begin(), m_states.end(), FrameSlotState::Free) - m_states.begin());
		m_states[index] = FrameSlotState::Writing;
		lock.unlock();

		// Only this writer owns the slot, arena reset must not hold the exchange mutex.
		WriteHandle write(*this, index);
		m_slots[index].beginFrame();
		return write;
	}

	bool FrameExchange::_publish(std::size_t index)
	{
		{
			std::lock_guard lock(m_mutex);

			assert(m_states[index] == FrameSlotState::Writing);
			// Exchange isn't closed, publish the frame and make it available for reading
			if (!m_closed)
			{
				m_ready[(m_readyHead + m_readyCount) % 2] = index;
				++m_readyCount;
				m_states[index] = FrameSlotState::Ready;
				return true;
			}

			m_states[index] = FrameSlotState::Free;
		}
		// notify the writer that a free slot is available
		m_freeSlotAvailable.notify_one();
		return false;
	}

	FrameExchange::ReadHandle FrameExchange::tryAcquireReady()
	{
		std::lock_guard lock(m_mutex);

		if (m_readyCount == 0)
			return {};

		const auto index = m_ready[m_readyHead];
		m_readyHead = (m_readyHead + 1) % 2;
		--m_readyCount;

		assert(m_states[index] == FrameSlotState::Ready);
		m_states[index] = FrameSlotState::Reading;
		return ReadHandle(*this, index);
	}

	void FrameExchange::_freeSlot(std::size_t index, FrameSlotState expectedState)
	{
		{
			std::lock_guard lock(m_mutex);
			assert(m_states[index] == expectedState);
			m_states[index] = FrameSlotState::Free;
		}
		// notify the writer that a free slot is available
		m_freeSlotAvailable.notify_one();
	}

	void FrameExchange::close()
	{
		{
			std::lock_guard lock(m_mutex);
			m_closed = true;
		}
		// notify all waiting writers that the exchange is closed
		m_freeSlotAvailable.notify_all();
	}
} // namespace SE
