#include "pch.h"

#include "FrameSlot.h"

namespace SE
{
	FrameSlot::FrameSlot(std::size_t initialCapacity)
		: m_storage(std::make_unique<std::byte[]>(initialCapacity)),
		  m_arena(m_storage.get(), initialCapacity),
		  m_frame(std::in_place, &m_arena)
	{}

	void FrameSlot::beginFrame()
	{
		m_frame.reset();
		m_arena.release();
		m_frame.emplace(&m_arena);
	}
} // namespace SE
