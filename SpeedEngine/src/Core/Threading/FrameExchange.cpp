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
			m_exchange->_freeSlot(m_index);
	}

	RenderFrame& FrameExchange::WriteHandle::frame()
	{
		return m_exchange->m_slots[m_index].frame();
	}

	void FrameExchange::WriteHandle::publish()
	{
		m_exchange->_publish(m_index);
		m_exchange = nullptr;
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
			m_exchange->_freeSlot(m_index);
	}

	const RenderFrame& FrameExchange::ReadHandle::frame() const
	{
		return m_exchange->m_slots[m_index].frame();
	}

	FrameExchange::WriteHandle FrameExchange::tryBeginWrite()
	{
		std::unique_lock lock(m_mutex);
		const auto free = std::find(m_states.begin(), m_states.end(), FrameSlotState::Free);
		if (free == m_states.end())
			return {};
		const auto index = static_cast<std::size_t>(free - m_states.begin());
		m_states[index] = FrameSlotState::Writing;
		lock.unlock();

		// Only this writer owns the slot, arena reset must not hold the exchange mutex.
		WriteHandle write(*this, index);
		m_slots[index].beginFrame();
		return write;
	}

	void FrameExchange::_publish(std::size_t index)
	{
		std::lock_guard lock(m_mutex);
		m_ready[(m_readyHead + m_readyCount) % 2] = index;
		++m_readyCount;
		m_states[index] = FrameSlotState::Ready;
	}

	FrameExchange::ReadHandle FrameExchange::tryAcquireReady()
	{
		std::lock_guard lock(m_mutex);

		if (m_readyCount == 0)
			return {};

		const auto index = m_ready[m_readyHead];
		m_readyHead = (m_readyHead + 1) % 2;
		--m_readyCount;

		m_states[index] = FrameSlotState::Reading;
		return ReadHandle(*this, index);
	}

	void FrameExchange::_freeSlot(std::size_t index)
	{
		std::lock_guard lock(m_mutex);
		m_states[index] = FrameSlotState::Free;
	}
} // namespace SE
