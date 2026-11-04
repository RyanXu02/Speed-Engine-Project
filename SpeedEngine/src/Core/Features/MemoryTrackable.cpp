#include "pch.h"

#include "MemoryTrackable.h"

#include "../Engine.h"
#include "MemoryTrackingSystem.h"

namespace SE
{
	MemoryTrackable::MemoryTrackable(const MemoryTag* tag)
	    : m_memoryTag(tag)
	{
		try
		{
			Engine::Instance().getSubSystem<MemoryTrackingSystem>()->setTag(this, m_memoryTag);
		}
		catch (const std::runtime_error&)
		{}
	}

	void* MemoryTrackable::operator new(std::size_t size)
	{
		auto* tracker = Engine::Instance().getSubSystem<MemoryTrackingSystem>();
		void* ptr = ::operator new(size);
		try
		{
			tracker->addAllocation(ptr, size);
		}
		catch (...)
		{
			::operator delete(ptr);
			throw;
		}
		return ptr;
	}

	void* MemoryTrackable::operator new(std::size_t size, std::align_val_t alignment)
	{
		auto* tracker = Engine::Instance().getSubSystem<MemoryTrackingSystem>();
		void* ptr = ::operator new(size, alignment);
		try
		{
			tracker->addAllocation(ptr, size);
		}
		catch (...)
		{
			::operator delete(ptr, alignment);
			throw;
		}
		return ptr;
	}

	void MemoryTrackable::operator delete(void* ptr) noexcept
	{
		if (!ptr)
			return;
		try
		{
			Engine::Instance().getSubSystem<MemoryTrackingSystem>()->removeAllocation(ptr);
		}
		catch (const std::runtime_error&)
		{
			// The tracker may already have been removed during shutdown
		}
		::operator delete(ptr);
	}

	void MemoryTrackable::operator delete(void* ptr, std::size_t) noexcept
	{
		operator delete(ptr);
	}

	void MemoryTrackable::operator delete(void* ptr, std::align_val_t alignment) noexcept
	{
		if (!ptr)
			return;
		try
		{
			Engine::Instance().getSubSystem<MemoryTrackingSystem>()->removeAllocation(ptr);
		}
		catch (const std::runtime_error&)
		{
			// The tracker may already have been removed during shutdown
		}
		::operator delete(ptr, alignment);
	}

	void MemoryTrackable::operator delete(void* ptr, std::size_t, std::align_val_t alignment) noexcept
	{
		operator delete(ptr, alignment);
	}

	void* MemoryTrackable::operator new[](std::size_t size)
	{
		return operator new(size);
	}

	void* MemoryTrackable::operator new[](std::size_t size, std::align_val_t alignment)
	{
		return operator new(size, alignment);
	}

	void MemoryTrackable::operator delete[](void* ptr) noexcept
	{
		operator delete(ptr);
	}

	void MemoryTrackable::operator delete[](void* ptr, std::size_t) noexcept
	{
		operator delete(ptr);
	}

	void MemoryTrackable::operator delete[](void* ptr, std::align_val_t alignment) noexcept
	{
		operator delete(ptr, alignment);
	}

	void MemoryTrackable::operator delete[](void* ptr, std::size_t, std::align_val_t alignment) noexcept
	{
		operator delete(ptr, alignment);
	}
} // namespace SE
